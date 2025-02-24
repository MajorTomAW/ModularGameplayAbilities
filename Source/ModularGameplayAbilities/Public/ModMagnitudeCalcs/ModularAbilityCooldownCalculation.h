// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "ModularAbilityCooldownCalculation.generated.h"

/**
 * Gameplay Modifier Magnitude Calculation class for calculating the cooldown of a modular ability.
 * Must be implemented by gameplay effects that are meant to be used with abilities using the explicit cooldown duration.
 */
UCLASS(meta = (DisplayName = "Explicit Cooldown Implementer"))
class MODULARGAMEPLAYABILITIES_API UModularAbilityCooldownCalculation : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UModularAbilityCooldownCalculation();

	//~ Begin UGameplayModMagnitudeCalculation Interface
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
	//~ End UGameplayModMagnitudeCalculation Interface
};
