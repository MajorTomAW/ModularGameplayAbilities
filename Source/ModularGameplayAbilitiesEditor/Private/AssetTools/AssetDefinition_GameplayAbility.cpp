// Copyright © 2024 Playton. All Rights Reserved.


#include "AssetDefinition_GameplayAbility.h"

#include "Abilities/GameplayAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_GameplayAbility)

#define LOCTEXT_NAMESPACE "GameplayAbilities"

//////////////////////////////////////////////////////////////////////////
/// UAssetDefinition_GameplayAbility

FText UAssetDefinition_GameplayAbility::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName", "Gameplay Ability");
}

FLinearColor UAssetDefinition_GameplayAbility::GetAssetColor() const
{
	return FLinearColor(FColor( 63, 126, 255 ));
}

TSoftClassPtr<> UAssetDefinition_GameplayAbility::GetAssetClass() const
{
	return UGameplayAbility::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayAbility::GetAssetCategories() const
{
	static const auto Categories =
	{
		FAssetCategoryPath(LOCTEXT("CategoryLabel", "Ability System"))
	};

	return Categories;
}

//////////////////////////////////////////////////////////////////////////
/// UAssetDefinition_GameplayEffect

FText UAssetDefinition_GameplayEffect::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName", "Gameplay Effect");
}

FLinearColor UAssetDefinition_GameplayEffect::GetAssetColor() const
{
	return FLinearColor(FColor( 63, 126, 255 ));
}

TSoftClassPtr<> UAssetDefinition_GameplayEffect::GetAssetClass() const
{
	return UGameplayEffect::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameplayEffect::GetAssetCategories() const
{
	static const auto Categories =
	{
		FAssetCategoryPath(LOCTEXT("CategoryLabel", "Ability System"))
	};

	return Categories;
}

#undef LOCTEXT_NAMESPACE
