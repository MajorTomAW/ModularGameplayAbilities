// Copyright © 2025 MajorT. All Rights Reserved.


#include "Abilities/Costs/ModularAbilityCost_ExplicitAttribute.h"

#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "Abilities/ModularGameplayAbility.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_AbilityFail_Cost, "Ability.ActivateFail.Cost");

FModularAbilityCost_ExplicitAttribute::FModularAbilityCost_ExplicitAttribute()
{
	CostValue.SetValue(1.f);
	FailureTag = TAG_AbilityFail_Cost;
}

bool FModularAbilityCost_ExplicitAttribute::CheckCost(
	const UModularGameplayAbility* Ability,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Attribute.IsValid())
	{
		const UAbilitySystemComponent* AbilitySystem = ActorInfo->AbilitySystemComponent.Get();
		const UAttributeSet* Set = AbilitySystem->GetAttributeSet(Attribute.GetAttributeSetClass());

		const int32 AbilityLevel = Ability->GetAbilityLevel();
		const float CurrentValue = Attribute.GetNumericValueChecked(Set);
		const float EvaluatedCost = CostValue.GetValueAtLevel(AbilityLevel) * -1.f;

		if ((CurrentValue + EvaluatedCost) < 0.0f)
		{
			return false;
		}
	}

	return true;
}

void FModularAbilityCost_ExplicitAttribute::ApplyCost(
	const UModularGameplayAbility* Ability,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (!Attribute.IsValid())
	{
		return;
	}

	const UAbilitySystemComponent* AbilitySystem = ActorInfo->AbilitySystemComponent.Get();
	const UAttributeSet* Set = AbilitySystem->GetAttributeSet(Attribute.GetAttributeSetClass());

	const int32 AbilityLevel = Ability->GetAbilityLevel();
	const float CurrentBase = Attribute.GetNumericValueChecked(Set);
	const float Magnitude = CostValue.GetValueAtLevel(AbilityLevel) * -1.f;

	// Aggregate the new base value
	const float NewBase = FAggregator::StaticExecModOnBaseValue(CurrentBase, EGameplayModOp::Additive, Magnitude);
	
	// Set the base value to the current value
	FActiveGameplayEffectsContainer& MutableContainer =
		const_cast<FActiveGameplayEffectsContainer&>(AbilitySystem->GetActiveGameplayEffects());

	MutableContainer.SetAttributeBaseValue(Attribute, NewBase);
}
