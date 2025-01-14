// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "ClassViewerFilter.h"

class FModularGameplayAbilitiesClassFilter : public IClassViewerFilter
{
public:
	FModularGameplayAbilitiesClassFilter()
		: DisallowedClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown)
	{
	}

	explicit FModularGameplayAbilitiesClassFilter(const UClass* AllowedClass)
		: DisallowedClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown)
	{
		AllowedChildrenOfClasses.Add(AllowedClass);
		ParentClass = AllowedClass;
	}
	
	const UClass* ParentClass = nullptr;
	TSet<const UClass*> AllowedChildrenOfClasses;
	EClassFlags DisallowedClassFlags;

public:
	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InClass &&
			!InClass->HasAnyClassFlags(DisallowedClassFlags) &&
			( InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed ||
				InClass->IsChildOf(ParentClass));
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const class IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<class FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) &&
			( InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed ||
				InUnloadedClassData->IsChildOf(ParentClass));
	}
};
