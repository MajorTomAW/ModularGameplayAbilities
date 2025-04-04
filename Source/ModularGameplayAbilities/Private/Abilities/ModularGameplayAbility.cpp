// Copyright © 2024 MajorT. All Rights Reserved.


#include "Abilities/ModularGameplayAbility.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "ModularAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "Misc/DataValidation.h"

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

	bStartWithCooldown = false;
	bPersistCooldownOnDeath = true;
	ExplicitCooldownDuration.Value = 0.f;

	bApplyingCostsEnabled = true;

	bStopsAIBehaviorLogic = false;
	bStopsAIMovement = false;
	bStopsAIRVOAvoidance = false;
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

bool UModularGameplayAbility::CanChangeActivationGroup(EGameplayAbilityActivationGroup::Type DesiredGroup)
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

void UModularGameplayAbility::TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const
{
PRAGMA_DISABLE_DEPRECATION_WARNINGS
	const bool bIsPredicting = (Spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting);
PRAGMA_ENABLE_DEPRECATION_WARNINGS

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
	TagContainer.AppendTags(GetAssetTags());
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
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UModularGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UModularGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UModularGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!bApplyingCostsEnabled)
	{
		return;
	}
	
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
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
	UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect();
	if (!IsValid(CooldownEffect))
	{
		return;
	}
	FGameplayEffectSpecHandle CooldownSpecHandle = MakeOutgoingGameplayEffectSpec(CooldownEffect->GetClass(), AbilityLevel);

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
	if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
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
		ModularAbilitySystem->GetAdditionalActivationTagRequirements(GetAssetTags(), AllRequiredTags, AllBlockedTags);
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
	if (UGameplayEffect* CDGE = GetCooldownGameplayEffect())
	{
		return &CDGE->GetGrantedTags();
	}

	if (ExplicitCooldownDuration.Value > 0.0f)
	{
		return &ExplicitCooldownTags;
	}

	return nullptr;
}

void UModularGameplayAbility::OnPawnAvatarSet()
{
	K2_OnPawnAvatarSet();
}

void UModularGameplayAbility::NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
{
}

#if WITH_EDITOR
EDataValidationResult UModularGameplayAbility::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::NonInstanced)
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(NSLOCTEXT("ModularGameplayAbilities", "NonInstancedAbilityError", "NonInstanced abilities are deprecated. Use InstancedPerActor instead."));
	}

	return Result;
}
#endif