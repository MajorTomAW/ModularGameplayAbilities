// Copyright © 2024 MajorT. All Rights Reserved.


#include "AbilitySetFactory.h"

#include "ModularAbilitySet.h"
#include "ModularAbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilitySetFactory)

#define LOCTEXT_NAMESPACE "AbilitySetFactory"

//////////////////////////////////////////////////////////////////////////
/// UAbilitySetFactory

UAbilitySetFactory::UAbilitySetFactory()
{
	SupportedClass = UModularAbilitySet::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

FText UAbilitySetFactory::GetDisplayName() const
{
	return LOCTEXT("AbilitySetFactoryDisplayName", "Ability Set");
}

bool UAbilitySetFactory::ConfigureProperties()
{
	return true;
}

UObject* UAbilitySetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(InClass->IsChildOf(UModularAbilitySet::StaticClass()));
	return NewObject<UModularAbilitySet>(InParent, InClass, InName, Flags | RF_Transactional, Context);
}

//////////////////////////////////////////////////////////////////////////
/// UTagRelationshipFactory

UTagRelationshipFactory::UTagRelationshipFactory()
{
	SupportedClass = UModularAbilityTagRelationshipMapping::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

FText UTagRelationshipFactory::GetDisplayName() const
{
	return LOCTEXT("TagRelationshipFactoryDisplayName", "Tag Relationship");
}

bool UTagRelationshipFactory::ConfigureProperties()
{
	return true;
}

UObject* UTagRelationshipFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(InClass->IsChildOf(UModularAbilityTagRelationshipMapping::StaticClass()));
	return NewObject<UModularAbilityTagRelationshipMapping>(InParent, InClass, InName, Flags | RF_Transactional, Context);
}

#undef LOCTEXT_NAMESPACE
