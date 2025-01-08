// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"

#include "AbilitySetFactory.generated.h"

//////////////////////////////////////////////////////////////////////////
/// UAbilitySetFactory

UCLASS(HideCategories = Object)
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