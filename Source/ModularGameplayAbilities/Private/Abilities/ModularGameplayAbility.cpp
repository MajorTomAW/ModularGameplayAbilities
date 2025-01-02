// Copyright © 2024 Playton. All Rights Reserved.


#include "Abilities/ModularGameplayAbility.h"

#include "ModularAbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"

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
	Super::SetCanBeCanceled(bCanBeCanceled);
}

void UModularGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
}

void UModularGameplayAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilitySpec& Spec)
{
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
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

void UModularGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}
