// Author: Tom Werner (MajorT), 2026 February

#pragma once

#include "AttributeInitializationKey.generated.h"

/** Key used as attribute set initter source. */
USTRUCT(BlueprintType)
struct FAttributeInitializationKey
{
	GENERATED_BODY()

public:
	FName Get() const
	{
		return *FString::Printf(TEXT("%s.%s"), *Category.ToString(), *SubCategory.ToString());
	}

	/** Category to use for a lookup in the attribute initialization spreadsheets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Key, meta=(GetOptions="ModularGameplayAbilities.ModularAbilitySystemGlobals.GetCategoryNames"))
	FName Category = NAME_None;

	/** Subcategory to use for a lookup in the attribute initialization spreadsheets. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Key)
	FName SubCategory = NAME_None;
};

