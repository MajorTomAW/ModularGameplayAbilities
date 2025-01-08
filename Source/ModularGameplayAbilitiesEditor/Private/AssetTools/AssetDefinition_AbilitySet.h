// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "AssetDefinitionDefault.h"

#include "AssetDefinition_AbilitySet.generated.h"

//////////////////////////////////////////////////////////////////////////
/// UAssetDefinition_AbilitySet

UCLASS()
class UAssetDefinition_AbilitySet : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	//~ Begin UAssetDefinition Interface
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	//~ End UAssetDefinition Interface
};

//////////////////////////////////////////////////////////////////////////
/// UAssetDefinition_TagRelationshipMapping

UCLASS()
class UAssetDefinition_TagRelationshipMapping : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	//~ Begin UAssetDefinition Interface
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	//~ End UAssetDefinition Interface
};
