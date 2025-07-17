// Author: Tom Werner (MajorT), 2025


#include "AssetDefinition_AbilitySet.h"

#include "ModularAbilitySet.h"
#include "ModularAbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AssetDefinition_AbilitySet)

#define LOCTEXT_NAMESPACE "ModularGameplayAbilitiesEditorPlugin"

//////////////////////////////////////////////////////////////////////////
/// UAssetDefinition_AbilitySet

FText UAssetDefinition_AbilitySet::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName_AbilitySet", "Ability Set");
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
		FAssetCategoryPath(LOCTEXT("CategoryLabel_AbilitySet", "Ability System"), LOCTEXT("SubCategoryLabel_AbilitySet", "Data"))
	};

	return Categories;
}

//////////////////////////////////////////////////////////////////////////
/// UAssetDefinition_TagRelationshipMapping

FText UAssetDefinition_TagRelationshipMapping::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName_TagRelationship", "Tag Relationship");
}

FLinearColor UAssetDefinition_TagRelationshipMapping::GetAssetColor() const
{
	return FLinearColor(FColor::FromHex("#0059cb"));
}

TSoftClassPtr<> UAssetDefinition_TagRelationshipMapping::GetAssetClass() const
{
	return UModularAbilityTagRelationshipMapping::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_TagRelationshipMapping::GetAssetCategories() const
{
	static const auto Categories =
	{
		FAssetCategoryPath(LOCTEXT("CategoryLabel_TagRelationship", "Ability System"), LOCTEXT("SubCategoryLabel_TagRelationship", "Data"))
	};

	return Categories;
}

#undef LOCTEXT_NAMESPACE
