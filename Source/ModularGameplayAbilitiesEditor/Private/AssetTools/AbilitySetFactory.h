// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Factories/Factory.h"

#include "AbilitySetFactory.generated.h"

//////////////////////////////////////////////////////////////////////////
/// UAbilitySetFactory

class UModularAbilitySet;

UCLASS(HideCategories = Object, Config=Editor)
class UAbilitySetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAbilitySetFactory();

protected:
	//~ Begin UFactory Interface
	virtual FText GetDisplayName() const override;
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface

private:
	UPROPERTY()
	TSubclassOf<UModularAbilitySet> SelectedAssetClass;

	UPROPERTY(Config)
	TArray<FSoftClassPath> CommonAssetClasses;
};

//////////////////////////////////////////////////////////////////////////
/// UTagRelationshipFactory

UCLASS(HideCategories = Object)
class UTagRelationshipFactory : public UFactory
{
	GENERATED_BODY()

public:
	UTagRelationshipFactory();

protected:
	//~ Begin UFactory Interface
	virtual FText GetDisplayName() const override;
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface
};
