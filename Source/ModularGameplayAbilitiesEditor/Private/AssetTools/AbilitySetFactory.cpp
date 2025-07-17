// Author: Tom Werner (MajorT), 2025


#include "AbilitySetFactory.h"

#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "ModularAbilitySet.h"
#include "ModularAbilityTagRelationshipMapping.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilitySetFactory)

#define LOCTEXT_NAMESPACE "AbilitySetFactory"


/*------------------------------------------------------------------------------
	Shared - used by multiple factories
------------------------------------------------------------------------------*/

class FAssetClassParentFilter : public IClassViewerFilter
{
public:
	FAssetClassParentFilter()
		: DisallowedClassFlags(CLASS_None), bDisallowBlueprintBase(false)
	{}

	/** All children of these classes will be included unless filtered out by another setting. */
	TSet< const UClass* > AllowedChildrenOfClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags;

	/** Disallow blueprint base classes. */
	bool bDisallowBlueprintBase;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		bool bAllowed= !InClass->HasAnyClassFlags(DisallowedClassFlags)
			&& InClass->CanCreateAssetOfClass()
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;

		if (bAllowed && bDisallowBlueprintBase)
		{
			if (FKismetEditorUtilities::CanCreateBlueprintOfClass(InClass))
			{
				return false;
			}
		}

		return bAllowed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		if (bDisallowBlueprintBase)
		{
			return false;
		}

		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};


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
	// nullptr the AssetClass so we can check for selection
	SelectedAssetClass = nullptr;

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;
	TSharedPtr<FAssetClassParentFilter> Filter = MakeShareable(new FAssetClassParentFilter);
	Options.ClassFilters.Add(Filter.ToSharedRef());

	for (auto& CommonClass : CommonAssetClasses)
	{
		if (UClass* Resolved = CommonClass.ResolveClass())
		{
			Options.ExtraPickerCommonClasses.Add(Resolved);
		}
	}


	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_HideDropDown;
	Filter->AllowedChildrenOfClasses.Add(UModularAbilitySet::StaticClass());

	const FText TitleText = LOCTEXT("CreateDataAssetOptions", "Pick Class For Data Asset Instance");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UModularAbilitySet::StaticClass());

	if (bPressedOk)
	{
		SelectedAssetClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UAbilitySetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	if (SelectedAssetClass != nullptr)
	{
		return NewObject<UDataAsset>(InParent, SelectedAssetClass, InName, Flags | RF_Transactional);
	}
	
	// if we have no data asset class, use the passed-in class instead
	check(InClass->IsChildOf(UDataAsset::StaticClass()));
	return NewObject<UDataAsset>(InParent, InClass, InName, Flags);
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
