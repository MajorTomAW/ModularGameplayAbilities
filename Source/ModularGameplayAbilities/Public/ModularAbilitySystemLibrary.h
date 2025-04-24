// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ModularAbilitySystemLibrary.generated.h"

struct FGameplayAbilityTargetDataHandle;
struct FGameplayAbilityTargetData;

/** Blueprint function library for some useful functions related to the ability system. */
UCLASS(meta=(ScriptName="ModularAbilitySystemLibrary"))
class MODULARGAMEPLAYABILITIES_API UModularAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Creates a forward targeting vector based on parameters. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability|Targeting")
	static void GetForwardFacingVectorFromActor(
		AActor* SourceActor,
		float ForwardDistance,
		float HorizontalAdjustment,
		float VerticalAdjustment,
		FVector& ForwardVector);
};
