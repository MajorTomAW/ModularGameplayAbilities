// Copyright © 2024 Playton. All Rights Reserved.


#include "ModularAbilitySystemComponent.h"

#include "AbilitySystemLog.h"
#include "Abilities/ModularGameplayAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularAbilitySystemComponent)

UModularAbilitySystemComponent::UModularAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FMemory::Memset(ActivationGroupCounts, 0 , sizeof(ActivationGroupCounts));
}

bool UModularAbilitySystemComponent::IsActivationGroupBlocked(EGameplayAbilityActivationGroup::Type Group) const
{
	bool bBlocked = false;

	switch (Group) {
	case EGameplayAbilityActivationGroup::Independent:
		{ // Independent abilities are never blocked
			bBlocked = false;
			break;
		}
	case EGameplayAbilityActivationGroup::Exclusive_Replaceable:
	case EGameplayAbilityActivationGroup::Exclusive_Blocking:
		{ // Exclusive abilities can activate if nothing is blocked
			bBlocked = (ActivationGroupCounts[(uint8)EGameplayAbilityActivationGroup::Exclusive_Blocking] > 0);
			break;
		}
	default:
		{ // Invalid group
			ensureAlwaysMsgf(false, TEXT("Invalid activation group: %d"), (uint8)Group);
			break;
		}
	}

	return bBlocked;
}

void UModularAbilitySystemComponent::AddAbilityToActivationGroup(
	EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);
	
	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group) {
	case EGameplayAbilityActivationGroup::Independent:
		{ // Independent abilities can't cancel other abilities
			break;
		}
	case EGameplayAbilityActivationGroup::Exclusive_Replaceable:
	case EGameplayAbilityActivationGroup::Exclusive_Blocking:
		{
			CancelActivationGroupAbilities(EGameplayAbilityActivationGroup::Exclusive_Replaceable, Ability, bReplicateCancelAbility);
			break;
		}
	default:
		{ // Invalid group
			ensureAlwaysMsgf(false, TEXT("Invalid activation group: %d"), (uint8)Group);
			break;
		}
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)EGameplayAbilityActivationGroup::Exclusive_Blocking] +
		ActivationGroupCounts[(uint8)EGameplayAbilityActivationGroup::Exclusive_Replaceable];

	ensureMsgf(ExclusiveCount <= 1, TEXT("%hs: Multiple exclusive abilities are running at the same time!"), __func__);
}

void UModularAbilitySystemComponent::RemoveAbilityFromActivationGroup(
	EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

void UModularAbilitySystemComponent::CancelActivationGroupAbilities(
	EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* AbilityToIgnore, bool bReplicateCancelAbilities)
{
	auto ShouldCancelFunc = [this, Group, AbilityToIgnore](const UModularGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
	{
		return ((Ability->GetActivationGroup() == Group) && (Ability != AbilityToIgnore));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbilities);
}

void UModularAbilitySystemComponent::CancelAbilitiesByFunc(
	TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();

	for (const auto& Spec : ActivatableAbilities.Items)
	{
		if (!Spec.IsActive())
		{
			continue;
		}

		UModularGameplayAbility* AbilityCDO = Cast<UModularGameplayAbility>(Spec.Ability);
		if (!AbilityCDO)
		{
			continue;
		}
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			// Cancel all instances of the ability
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			for (UGameplayAbility* Instance : Instances)
			{
				UModularGameplayAbility* ModularAbility = CastChecked<UModularGameplayAbility>(Instance);

				if (ShouldCancelFunc(ModularAbility, Spec.Handle))
				{
					if (ModularAbility->CanBeCanceled())
					{
						ModularAbility->CancelAbility(Spec.Handle, AbilityActorInfo.Get(), ModularAbility->GetCurrentActivationInfo(), bReplicateCancelAbility);
					}
					else
					{
						ABILITY_LOG(Error, TEXT("%hs: Can't cancel ability [%s] because CanBeCanceled() is false."), __func__, *ModularAbility->GetName());
					}
				}
			}
		}
		else
		{
			// Cancel CDO
			if (ShouldCancelFunc(AbilityCDO, Spec.Handle))
			{
				// CDO can always be canceled
				check(AbilityCDO->CanBeCanceled());
				AbilityCDO->CancelAbility(Spec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
			}
		}
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}
}





