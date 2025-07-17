// Author: Tom Werner (MajorT), 2025


#include "Effects/ModularGameplayEffect_Cooldown.h"

#include "ModMagnitudeCalcs/ModularAbilityCooldownCalculation.h"

UModularGameplayEffect_Cooldown::UModularGameplayEffect_Cooldown()
{
	// Use our custom calculation, so it works with abilities that have an explicit cooldown duration
	FCustomCalculationBasedFloat CustomCalc;
	CustomCalc.CalculationClassMagnitude = UModularAbilityCooldownCalculation::StaticClass();

	// Initialize the effect with the cooldown calculation
	DurationMagnitude = FGameplayEffectModifierMagnitude(CustomCalc);
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
}
