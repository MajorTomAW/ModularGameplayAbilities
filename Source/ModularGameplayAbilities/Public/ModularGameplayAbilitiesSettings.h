// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "UObject/Object.h"

#include "ModularGameplayAbilitiesSettings.generated.h"

UCLASS(Config=Engine, DefaultConfig, MinimalAPI)
class UModularGameplayAbilitiesSettings : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	static MODULARGAMEPLAYABILITIES_API bool IsUsingExperimentalInput() { return GetDefault<ThisClass>()->bEnableExperimentalAbilityInput; }

	UFUNCTION()
	static MODULARGAMEPLAYABILITIES_API bool IsNotUsingExperimentalInput() { return !GetDefault<ThisClass>()->bEnableExperimentalAbilityInput; }

protected:
	UPROPERTY(Config, EditAnywhere, Category = Experimental, meta=(ConfigRestartRequired=true))
	bool bEnableExperimentalAbilityInput = false;
};
