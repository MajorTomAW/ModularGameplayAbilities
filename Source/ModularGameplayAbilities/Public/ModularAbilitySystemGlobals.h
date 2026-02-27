// Author: Tom Werner (MajorT), 2026 February

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "ModularAbilitySystemGlobals.generated.h"


/** This only class purpose is to have an accessor to the GlobalAttributeDefaultsTables */
UCLASS(MinimalAPI)
class UModularAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	/** Returns the global attribute defaults tables. */
	const TArray<TObjectPtr<UCurveTable>>& GetGlobalAttributeDefaultsTables() const { return GlobalAttributeDefaultsTables; }

#if WITH_EDITOR
	UFUNCTION()
	static TArray<FName> GetCategoryNames();
#endif
};
