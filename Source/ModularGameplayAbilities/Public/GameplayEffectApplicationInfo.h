// Author: Tom Werner (MajorT), 2025

#pragma once

#include "ScalableFloat.h"

#include "GameplayEffectApplicationInfo.generated.h"

class UGameplayEffect;

/** Data about a gameplay effect to be applied, including its level. */
USTRUCT(BlueprintType)
struct FGameplayEffectApplicationInfo
{
	GENERATED_BODY()

	/** The gameplay effect class to be applied. This is a soft reference to allow for lazy loading. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=GameplayEffect)
	TSoftClassPtr<UGameplayEffect> GameplayEffect;

	/** The default level of the gameplay effect. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=GameplayEffect, meta = (ClampMin = 1, UIMin = 1))
	FScalableFloat Level;
};
