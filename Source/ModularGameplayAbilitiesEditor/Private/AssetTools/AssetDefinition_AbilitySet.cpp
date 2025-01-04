// Copyright © 2024 MajorT. All Rights Reserved.


#include "AssetDefinition_AbilitySet.h"

#include "ModularAbilitySet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_AbilitySet)

#define LOCTEXT_NAMESPACE "ModularGameplayAbilitiesEditor"

FText UAssetDefinition_AbilitySet::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName", "Ability Set");
}

FLinearColor UAssetDefinition_AbilitySet::GetAssetColor() const
{
	return FLinearColor(FColor(201, 29, 85));
}

TSoftClassPtr<> UAssetDefinition_AbilitySet::GetAssetClass() const
{
	return UModularAbilitySet::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_AbilitySet::GetAssetCategories() const
{
	static const auto Categories =
	{
		FAssetCategoryPath(LOCTEXT("CategoryLabel", "Ability System"), LOCTEXT("SubCategoryLabel", "Data"))
	};

	return Categories;
}

#undef LOCTEXT_NAMESPACE