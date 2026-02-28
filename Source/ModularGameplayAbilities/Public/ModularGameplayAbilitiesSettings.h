// Author: Tom Werner (MajorT), 2025

#pragma once

#include "UObject/Object.h"

#include "ModularGameplayAbilitiesSettings.generated.h"

UCLASS(Config=Engine, DefaultConfig, MinimalAPI)
class UModularGameplayAbilitiesSettings : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION()
	static MODULARGAMEPLAYABILITIES_API bool IsUsingAlterAbilityInput() { return GetDefault<ThisClass>()->bEnableAlterAbilityInput; }

	UFUNCTION()
	static MODULARGAMEPLAYABILITIES_API bool IsNotUsingAlterAbilityInput() { return !GetDefault<ThisClass>()->bEnableAlterAbilityInput; }

protected:
	UPROPERTY(Config, EditAnywhere, Category = Experimental, meta=(ConfigRestartRequired=true))
	bool bEnableAlterAbilityInput = false;
};
