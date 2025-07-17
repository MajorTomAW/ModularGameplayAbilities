// Author: Tom Werner (MajorT), 2025


#include "ModMagnitudeCalcs/ModularAbilityCooldownCalculation.h"

#include "Abilities/GameplayAbility.h"
#include "Abilities/ModularGameplayAbility.h"

UModularAbilityCooldownCalculation::UModularAbilityCooldownCalculation()
{
}

float UModularAbilityCooldownCalculation::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayEffectContextHandle& Context = Spec.GetContext();
	const UGameplayAbility* Ability = Context.GetAbilityInstance_NotReplicated();
	if (!IsValid(Ability))
	{
		return 0.0f;
	}

	const UModularGameplayAbility* ModularAbility = Cast<UModularGameplayAbility>(Ability);
	checkf(ModularAbility, TEXT("ModularAbilityCooldownCalculation can only be used with ModularGameplayAbilities!"));

	// See if this ability has an explicit cooldown duration
	if (!ModularAbility->HasExplicitCooldownDuration())
	{
		return 0.0f;
	}

	// Calculate cooldown duration based on the explicit cooldown duration of the ability
	// And its level
	const int32 AbilityLevel = Ability->GetAbilityLevel();
	const float CooldownDuration = ModularAbility->GetExplicitCooldownDuration().GetValueAtLevel(AbilityLevel);
	return CooldownDuration;
}
