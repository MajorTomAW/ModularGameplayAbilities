// Copyright © 2024 MajorT. All Rights Reserved.


#include "Abilities/ModularGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "ModularAbilitySystemComponent.h"
#include "Abilities/Costs/ModularAbilityCost.h"
#include "GameFramework/PlayerState.h"
#include "Misc/DataValidation.h"
#include "Runtime/AIModule/Classes/AIController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularGameplayAbility)

#define ENGINE_VERSION_OLDER_5_4 ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4

#if !ENGINE_VERSION_OLDER_5_4
#include "AbilitySystemLog.h"


#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(FunctionName, ReturnValue)																				\
{																																						\
	if (!ensure(IsInstantiated()))																														\
	{																																					\
		ABILITY_LOG(Error, TEXT("%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy."), *GetPathName());	\
		return ReturnValue;																																\
	}																																					\
}
#endif

UModularGameplayAbility::UModularGameplayAbility(const FObjectInitializer& ObjectInitializer)
	: UGameplayAbility(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationPolicy = EGameplayAbilityActivationPolicy::Active;
	ActivationGroup = EGameplayAbilityActivationGroup::Independent;
	bActivateIfTagsAlreadyPresent =  false;
	bForceReceiveInput = false;
	LastInputCallbackTime = 0.0f;

	bStartWithCooldown = false;
	bPersistCooldownOnDeath = true;
	ExplicitCooldownDuration.Value = 0.f;

	bApplyingCostsEnabled = true;

	bStopsAIBehaviorLogic = false;
	bStopsAIMovement = false;
	bStopsAIRVOAvoidance = false;
	ActivationNoiseRange = 1000.f;
	ActivationNoiseLoudness = 1.0f;

	auto ImplementedInBlueprint = [] (const UFunction* Func) -> bool
	{
		return Func &&
			ensure(Func->GetOuter()) &&
			Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};

	// Check for blueprint implementation of these events to save performance
	{ // Input pressed
		const UFunction* Func = GetClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(ThisClass, K2_OnAbilityInputPressed));
		bHasBlueprintInputPressed = ImplementedInBlueprint(Func);
	}
	{ // Input released
		const UFunction* Func = GetClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(ThisClass, K2_OnAbilityInputReleased));
		bHasBlueprintInputReleased = ImplementedInBlueprint(Func);
	}
}

AController* UModularGameplayAbility::GetControllerFromActorInfo() const
{
#if !ENGINE_VERSION_OLDER_5_4
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(GetControllerFromActorInfo, nullptr);
#else
	ensureMsgf(IsInstantiated(),
		TEXT("%hs called on the CDO. NonInstanced abilities are deprecated, thus we always expect this to be called on an instanced object."), __func__);
#endif

	if (!ensure(CurrentActorInfo))
	{
		return nullptr;
	}

	if (AController* C = CurrentActorInfo->PlayerController.Get())
	{
		return C;
	}

	// Look for a controller or pawn in the owner chain.
	AActor* TestActor = CurrentActorInfo->OwnerActor.Get();
	while (TestActor)
	{
		if (AController* C = Cast<AController>(TestActor))
		{
			return C;
		}

		if (const APawn* P = Cast<APawn>(TestActor))
		{
			return P->GetController();
		}

		if (const APlayerState* PS = Cast<APlayerState>(TestActor))
		{
			return PS->GetOwningController();
		}

		TestActor = TestActor->GetOwner();
	}

	return nullptr;
}

APawn* UModularGameplayAbility::GetAvatarAsPawn() const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		return Cast<APawn>(Avatar);
	}

	return nullptr;
}

bool UModularGameplayAbility::CanChangeActivationGroup(EGameplayAbilityActivationGroup::Type DesiredGroup) const
{
	if (!IsInstantiated() || !IsActive())
	{
		return false;
	}

	if (ActivationGroup == DesiredGroup)
	{
		return true;
	}

	UModularAbilitySystemComponent* ModularASC = Cast<UModularAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	check(ModularASC);

	if ((ActivationGroup != EGameplayAbilityActivationGroup::Exclusive_Blocking) && ModularASC->IsActivationGroupBlocked(DesiredGroup))
	{
		// Can't change groups if the desired group is blocked (unless it is the one doing the blocking).
		return false;
	}

	if ((DesiredGroup == EGameplayAbilityActivationGroup::Exclusive_Replaceable) && !CanBeCanceled())
	{
		// If the ability can't be canceled, it can't be replaced.
		return false;
	}

	return true;
}

bool UModularGameplayAbility::ChangeActivationGroup(EGameplayAbilityActivationGroup::Type DesiredGroup)
{
#if !ENGINE_VERSION_OLDER_5_4
	ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN(ChangeActivationGroup, false);
#endif

	if (!CanChangeActivationGroup(DesiredGroup))
	{
		return false;
	}

	if (ActivationGroup != DesiredGroup)
	{
		UModularAbilitySystemComponent* ModularASC = Cast<UModularAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
		check(ModularASC);

		ModularASC->RemoveAbilityFromActivationGroup(ActivationGroup, this);
		ModularASC->AddAbilityToActivationGroup(DesiredGroup, this);

		ActivationGroup = DesiredGroup;
	}

	return true;
}

TArray<AActor*> UModularGameplayAbility::GetTrackedActors() const
{
	TArray<AActor*> TrackedActors;

	if (!CurrentActorInfo || !IsInstantiated())
	{
		return TrackedActors;
	}

	UModularAbilitySystemComponent* AbilitySystem =
		Cast<UModularAbilitySystemComponent>(CurrentActorInfo->AbilitySystemComponent.Get());
	if (!AbilitySystem)
	{
		return TrackedActors;
	}

	const FGameplayAbilitySpecHandle SpecHandle = GetCurrentAbilitySpecHandle();
	if (!SpecHandle.IsValid())
	{
		return TrackedActors;
	}

	if (const TArray<FAbilityTrackedActorEntry>* TrackedEntries =
		AbilitySystem->AbilitySpecTrackedActors.Find(SpecHandle))
	{
		for (const auto& Entry : *TrackedEntries)
		{
			if (Entry.TrackedActor.IsValid())
			{
				TrackedActors.Add(Entry.TrackedActor.Get());
			}
		}
	}

	return TrackedActors;
}

void UModularGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
#if ENGINE_VERSION_OLDER_5_4
	const bool bIsPredicting = (GetCurrentActivationInfo().ActivationMode == EGameplayAbilityActivationMode::Predicting);
#else
	const bool bIsPredicting = (Spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);
#endif

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	const bool bIsPassiveAbility = (ActivationPolicy == EGameplayAbilityActivationPolicy::Passive);

	// Only passive abilities can be auto-activated on spawn
	if (!bIsPassiveAbility)
	{
		return;
	}

	auto HasRequiredTags = [&, ASC]()->bool
	{
		if (ActivationRequiredTags.IsEmpty())
		{
			return true;
		}

		if (ASC->GetOwnedGameplayTags().HasAllExact(ActivationRequiredTags) &&
			bActivateIfTagsAlreadyPresent)
		{
			return true;
		}

		return false;
	};

	if (ActorInfo &&
		!Spec.IsActive() &&
		!bIsPredicting &&
		HasRequiredTags())
	{
		const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

		if (ASC &&
			AvatarActor &&
			!AvatarActor->GetTearOff() &&
			(AvatarActor->GetLifeSpan() <= 0.f))
		{
			const bool bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) ||
				(NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);

			const bool bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated) ||
				(NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly);

			const bool bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			const bool bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

void UModularGameplayAbility::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
	TagContainer.AppendTags(GetAssetTags());
#else
	TagContainer.AppendTags(AbilityTags);
#endif
}

bool UModularGameplayAbility::ShouldReceiveInputEvents() const
{
	if (ActivationPolicy == EGameplayAbilityActivationPolicy::Active)
	{
		return true;
	}

	return bForceReceiveInput;
}

AAIController* UModularGameplayAbility::GetOwningAIController() const
{
	AAIController* AIController = Cast<AAIController>(GetControllerFromActorInfo());

	if (AIController == nullptr)
	{
		if (const APawn* Pawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
		{
			AIController = Pawn->GetController<AAIController>();
		}
	}

	return AIController;
}

void UModularGameplayAbility::SetAIFocalPoint(AActor* FocusTarget, FVector FocalPoint, uint8 Priority)
{
	if (AAIController* AIController = GetOwningAIController())
	{
		// Decide whether to set the focus or focal points based on which data is valid
		if (IsValid(FocusTarget))
		{
			AIController->SetFocus(FocusTarget, Priority);
		}
		else if (!FocalPoint.IsNearlyZero())
		{
			AIController->SetFocalPoint(FocalPoint, Priority);
		}
	}
}

void UModularGameplayAbility::ClearAIFocalPoint(uint8 Priority)
{
	if (AAIController* AIController = GetOwningAIController())
	{
		// Just clear the focus
		AIController->ClearFocus(Priority);
	}
}

AActor* UModularGameplayAbility::GetAIAbilityTarget() const
{
	// The targeting actor is equivalent to the focus actor
	if (const AAIController* AIController = GetOwningAIController())
	{
		return AIController->GetFocusActor();
	}

	return nullptr;
}

bool UModularGameplayAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	// This makes sure we aren't calling this function on the CDO
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	// Check for Super
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check for activation group
	UModularAbilitySystemComponent* ModularASC = Cast<UModularAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get());
	if (ModularASC->IsActivationGroupBlocked(ActivationGroup))
	{
		if (OptionalRelevantTags)
		{
			// Add Tag to the container
		}

		return false;
	}

	return true;
}

void UModularGameplayAbility::SetCanBeCanceled(bool bCanBeCanceled)
{
	if (!bCanBeCanceled && (ActivationGroup == EGameplayAbilityActivationGroup::Exclusive_Replaceable))
	{
		ABILITY_LOG(Error, TEXT("%hs: Can't set CanBeCanceled to false on an Exclusive_Replaceable ability (%s)."), __func__, *GetName());
		return;
	}
	
	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UModularGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	K2_OnAbilityAdded();

	TryActivateAbilityOnSpawn(ActorInfo, Spec);

	if (bStartWithCooldown)
	{
		ApplyCooldown(Spec.Handle, ActorInfo, GetCurrentActivationInfo());
	}
}

void UModularGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	K2_OnAbilityRemoved();
	
	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UModularGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* AbilitySystem = ActorInfo->AbilitySystemComponent.Get();
	if (ensure(AbilitySystem) &&
		ShouldReceiveInputEvents() &&
		(bHasBlueprintInputPressed || bHasBlueprintInputReleased))
	{
		// Cache the starting time of this ability's activation
		LastInputCallbackTime = GetWorld()->GetTimeSeconds();
		
		// For locally controlled players, immediately check for the input pressed flag
		constexpr bool bShouldCheckImmediately = false;
		if (IsLocallyControlled() && bShouldCheckImmediately)
		{
			const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
			if (Spec && Spec->InputPressed)
			{
				OnInputChangedCallback(true);
				return;
			}
		}

		// Assign to the input pressed delegate
		if (bHasBlueprintInputPressed)
		{
			AbilitySystem->AbilityReplicatedEventDelegate(
				EAbilityGenericReplicatedEvent::InputPressed,
				Handle,
				ActivationInfo.GetActivationPredictionKey())
			.AddUObject(this, &ThisClass::OnInputChangedCallback, true);	
		}

		// Assign to the input released delegate
		if (bHasBlueprintInputReleased)
		{
			AbilitySystem->AbilityReplicatedEventDelegate(
				EAbilityGenericReplicatedEvent::InputReleased,
				Handle,
				ActivationInfo.GetActivationPredictionKey())
			.AddUObject(this, &ThisClass::OnInputChangedCallback, false);
		}

		if (IsForRemoteClient())
		{
			if (!AbilitySystem->CallReplicatedEventDelegateIfSet(
				EAbilityGenericReplicatedEvent::InputPressed,
				Handle,
				ActivationInfo.GetActivationPredictionKey()))
			{
				//@TODO: Await remote player data?
			}
		}
	}
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
void UModularGameplayAbility::OnInputChangedCallback(bool bIsPressed)
{
	// This shouldn't be false at this point, but just in case
	if (!ShouldReceiveInputEvents())
	{
		return;
	}

	// Determine the duration between the last input callback and this one
	const float ElapsedTime = GetWorld()->GetTimeSeconds() - LastInputCallbackTime;
	
	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystem || !IsValid(this))
	{
		return;
	}

	const EAbilityGenericReplicatedEvent::Type EventType =
		bIsPressed
		? EAbilityGenericReplicatedEvent::InputPressed
		: EAbilityGenericReplicatedEvent::InputReleased;

	FScopedPredictionWindow PredictionWindow(AbilitySystem, IsPredictingClient());
	if (IsPredictingClient())
	{
		// Notify the server about this callback
		AbilitySystem->ServerSetReplicatedEvent(
			EventType,
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey(),
			AbilitySystem->ScopedPredictionKey);
	}
	else
	{
		// Otherwise consume the event
		AbilitySystem->ConsumeGenericReplicatedEvent(
			EventType,
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey());
	}

	// We're done now. Inform the blueprint about the input event
	if (bIsPressed && bHasBlueprintInputPressed)
	{
		K2_OnAbilityInputPressed(ElapsedTime);
	}
	else if (bHasBlueprintInputReleased)
	{
		K2_OnAbilityInputReleased(ElapsedTime);
	}
}

void UModularGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// When this ability was ended, make sure we have no pending delegates still assigned to the ASC
	if (UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo())
	{
		// Remove pressed delegate
		AbilitySystem->AbilityReplicatedEventDelegate(
			EAbilityGenericReplicatedEvent::InputPressed,
			Handle,
			ActivationInfo.GetActivationPredictionKey())
		.RemoveAll(this);

		// Remove released delegate
		AbilitySystem->AbilityReplicatedEventDelegate(
			EAbilityGenericReplicatedEvent::InputReleased,
			Handle,
			ActivationInfo.GetActivationPredictionKey())
		.RemoveAll(this);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UModularGameplayAbility::CheckCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
	{
		return false;
	}

	// Check for additional costs
	for (const auto& InstancedCost : AbilityCosts)
	{
		if (const FModularAbilityCost* Cost = InstancedCost.GetPtr<FModularAbilityCost>())
		{
			if (!Cost->CheckCost(this, Handle, ActorInfo, OptionalRelevantTags))
			{
				return false;
			}
		}
	}

	return true;
}

void UModularGameplayAbility::ApplyCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!bApplyingCostsEnabled)
	{
		return;
	}
	
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	check(ActorInfo);

	// Used to determine if the ability actually hit a target
	// (as some costs are only spent on successful attempts)
	auto DetermineIfAbilityHitTarget = [&] () ->bool
	{
		if (!ActorInfo->IsNetAuthority())
		{
			return false;
		}

		if (UModularAbilitySystemComponent* ModularAbilitySystem = Cast<UModularAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
		{
			FGameplayAbilityTargetDataHandle TargetData;
			ModularAbilitySystem->GetAbilityTargetData(Handle, ActivationInfo, TargetData);

			for (int32 TargetDataIdx = 0; TargetDataIdx < TargetData.Data.Num(); ++TargetDataIdx)
			{
				if (UAbilitySystemBlueprintLibrary::TargetDataHasHitResult(TargetData, TargetDataIdx))
				{
					return true;
				}
			}
		}

		return false;
	};

	// Apply any additional ability costs
	bool bAbilityHitTarget = false;
	bool bHasDeterminedIfAbilityHitTarget = false;

	for (int i = 0; i < AbilityCosts.Num(); ++i)
	{
		if (!ensure(AbilityCosts.IsValidIndex(i)))
		{
			continue;
		}

		const FInstancedStruct& Instanced = AbilityCosts[i];
		if (const FModularAbilityCost* Cost = Instanced.GetPtr<FModularAbilityCost>())
		{
			if (Cost->ShouldOnlyApplyCostOnHit())
			{
				if (!bHasDeterminedIfAbilityHitTarget)
				{
					bAbilityHitTarget = DetermineIfAbilityHitTarget();
					bHasDeterminedIfAbilityHitTarget = true;
				}

				if (!bAbilityHitTarget)
				{
					continue;
				}
			}

			FModularAbilityCost* MutableCost = const_cast<FModularAbilityCost*>(Cost);
			MutableCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
}

bool UModularGameplayAbility::CheckCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);
}

void UModularGameplayAbility::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	const int32 AbilityLevel = GetAbilityLevel(Handle, ActorInfo);

	// Check if we have a cooldown effect
	const UGameplayEffect* CooldownEffectCDO = GetCooldownGameplayEffect();
	if (!IsValid(CooldownEffectCDO))
	{
		return;
	}
	const FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CooldownEffectCDO->GetClass(), AbilityLevel);

	// Check if we have valid cooldown tags
	if (HasExplicitCooldownDuration())
	{
		if (ExplicitCooldownTags.IsValid())
		{
			CooldownSpecHandle.Data->AppendDynamicAssetTags(ExplicitCooldownAssetTags);
			CooldownSpecHandle.Data->DynamicGrantedTags.AppendTags(ExplicitCooldownTags);	
		}
		else
		{
			ABILITY_LOG(Error, TEXT("ExplicitCooldownTags are not valid for ability %s. Cooldown will not be applied."), *GetName());
			return;
		}
	}

	// Apply cooldown
	FActiveGameplayEffectHandle CooldownHandle =
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CooldownSpecHandle);

	// Let others know we applied a cooldown
	OnApplyCooldownDelegate.Broadcast(this, ExplicitCooldownDuration.GetValueAtLevel(AbilityLevel), ExplicitCooldownTags);
}

bool UModularGameplayAbility::DoesAbilitySatisfyTagRequirements(
	const UAbilitySystemComponent& AbilitySystemComponent,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	// Specialized version to handle death exclusion and AbilityTags expansion via ASC
	bool bBlocked = false;
	bool bMissing = false;

	const UAbilitySystemGlobals& AbilitySystemGlobals = UAbilitySystemGlobals::Get();
	const FGameplayTag& BlockedTag = AbilitySystemGlobals.ActivateFailTagsBlockedTag;
	const FGameplayTag& MissingTag = AbilitySystemGlobals.ActivateFailTagsMissingTag;

	// CHeck if any of this ability's tags are already blocked
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
#else
	if (AbilitySystemComponent.AreAbilityTagsBlocked(AbilityTags))
#endif
	{
		bBlocked = true;
	}

	const UModularAbilitySystemComponent* ModularAbilitySystem = Cast<UModularAbilitySystemComponent>(&AbilitySystemComponent);
	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;
	
	// Expand our ability tags by adding additional required/blocked tags
	if (ModularAbilitySystem)
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
		ModularAbilitySystem->GetAdditionalActivationTagRequirements(GetAssetTags(), AllRequiredTags, AllBlockedTags);
#else
		ModularAbilitySystem->GetAdditionalActivationTagRequirements(AbilityTags, AllRequiredTags, AllBlockedTags);
#endif
	}

	// Check to see the required/blocked tags for this ability
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemTags;
		AbilitySystemTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemTags);

		if (AbilitySystemTags.HasAny(AllBlockedTags))
		{
			if (OptionalRelevantTags) // Add additional tag checks
			{
				
			}

			bBlocked = true;
		}

		if (!AbilitySystemTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	// Check source required/blocked tags
	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	// Check target required/blocked tags
	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	// Fill out the relevant tags
	if (bBlocked)
	{
		if (OptionalRelevantTags && BlockedTag.IsValid())
		{
			OptionalRelevantTags->AddTag(BlockedTag);
		}

		return false;
	}

	if (bMissing)
	{
		if (OptionalRelevantTags && MissingTag.IsValid())
		{
			OptionalRelevantTags->AddTag(MissingTag);
		}

		return false;
	}

	return true;
}

const FGameplayTagContainer* UModularGameplayAbility::GetCooldownTags() const
{
	// Gosh, this really isn't thread safe, is it?
	static FGameplayTagContainer LocalCooldownTags;
	if (UGameplayEffect* CDGE = GetCooldownGameplayEffect())
	{
		LocalCooldownTags.AppendTags(CDGE->GetGrantedTags());
	}

	if (ExplicitCooldownDuration.GetValueAtLevel(GetAbilityLevel()) > 0.0f)
	{
		LocalCooldownTags.AppendTags(ExplicitCooldownTags);
	}

	return &LocalCooldownTags;
}

void UModularGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}

void UModularGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
	bool bSimpleFailureFound = false;
	for (const FGameplayTag& Reason : FailedReason)
	{
		
	}
}

void UModularGameplayAbility::K2_OverrideFailedReason_Implementation(
	const FGameplayTagContainer& FailedReason,
	FGameplayTagContainer& OverridenFailedReason) const
{
	OverridenFailedReason = FailedReason;
}

#if WITH_EDITOR
EDataValidationResult UModularGameplayAbility::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	if (GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("ModularGameplayAbilities", "NonInstancedAbilityError", "NonInstanced abilities are deprecated. Use InstancedPerActor instead."));
	}
PRAGMA_ENABLE_DEPRECATION_WARNINGS	

	return Result;
}
#endif