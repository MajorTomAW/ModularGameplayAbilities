// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ModularGameplayEffect_Cooldown.generated.h"

/**
 * A generic gameplay effect that comes with an inbuilt cooldown mod mag calculation component for its duration.
 */
UCLASS(Abstract)
class MODULARGAMEPLAYABILITIES_API UModularGameplayEffect_Cooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UModularGameplayEffect_Cooldown();
};
