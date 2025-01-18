// Copyright © 2024 MajorT. All Rights Reserved.


#include "GameplayAbilityFactory.h"

#include "ClassViewerModule.h"
#include "KismetCompilerModule.h"
#include "ModularGameplayAbilitiesClassFilter.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/BlueprintMapLibrary.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/SClassPickerDialog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbilityFactory)

#define LOCTEXT_NAMESPACE "GameplayAbilityFactory"

//////////////////////////////////////////////////////////////////////////
/// UGameplayAbilityFactory

UGameplayAbilityFactory::UGameplayAbilityFactory()
{
	SupportedClass = UGameplayAbility::StaticClass();
	ParentClass = UGameplayAbility::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;

	CommonAbilityClasses.Add(UGameplayAbility::StaticClass());
}

FText UGameplayAbilityFactory::GetDisplayName() const
{
	return LOCTEXT("GameplayAbilityFactoryDisplayName", "Gameplay Ability");
}

bool UGameplayAbilityFactory::ConfigureProperties()
{
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(ParentClass, DerivedClasses, false);
	if (DerivedClasses.Num() == 0 || bSkipClassPicker)
	{
		// If there is only one derived class, skip the class picker
		return true;
	}
	
	// Null the parent class to ensure one is selected
	ParentClass = nullptr;

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.ExtraPickerCommonClasses.Empty();
	Options.DisplayMode = EClassViewerDisplayMode::ListView;
	Options.bAllowViewOptions = true;
	Options.bExpandRootNodes = true;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowObjectRootClass = true;

	TArray<UClass*> CommonClasses = CommonAbilityClasses;
	Options.ExtraPickerCommonClasses = CommonClasses;

	TSharedPtr<FModularGameplayAbilitiesClassFilter> Filter = MakeShareable(new FModularGameplayAbilitiesClassFilter(UGameplayAbility::StaticClass()));
	Options.ClassFilters.Add(Filter.ToSharedRef());

	const FText TitleText = LOCTEXT("PickParentClass", "Pick Parent Class for New Gameplay Ability");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGameplayAbility::StaticClass());

	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UGameplayAbilityFactory::FactoryCreateNew(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	if ((ParentClass == nullptr) || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass))
	{
		FFormatNamedArguments Args;
		Args.Add( TEXT("ClassName"), (ParentClass != nullptr) ? FText::FromString( ParentClass->GetName() ) : LOCTEXT("Null", "(null)") );
		FMessageDialog::Open( EAppMsgType::Ok, FText::Format( LOCTEXT("CannotCreateBlueprintFromClass", "Cannot create a blueprint based on the class '{ClassName}'."), Args ) );
		return nullptr;
	}

	UClass* BlueprintClass = nullptr;
	UClass* BlueprintGeneratedClass = nullptr;

	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
	KismetCompilerModule.GetBlueprintTypesForClass(ParentClass, BlueprintClass, BlueprintGeneratedClass);

	UBlueprint* Blueprint =
		FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, InName, BPTYPE_Normal, BlueprintClass, BlueprintGeneratedClass, CallingContext);

	// Mark the Blueprint as Data-Only
	Blueprint->bRecompileOnLoad = false;

	return Blueprint;
}

//////////////////////////////////////////////////////////////////////////
/// UGameplayEffectFactory

UGameplayEffectFactory::UGameplayEffectFactory()
{
	SupportedClass = UGameplayEffect::StaticClass();
	ParentClass = UGameplayEffect::StaticClass();

	bCreateNew = true;
	bEditAfterNew = true;

	CommonEffectClasses.Add(UGameplayEffect::StaticClass());
}

FText UGameplayEffectFactory::GetDisplayName() const
{
	return LOCTEXT("GameplayEffectFactoryDisplayName", "Gameplay Effect");
}

bool UGameplayEffectFactory::ConfigureProperties()
{
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(ParentClass, DerivedClasses, false);
	if (DerivedClasses.Num() == 0 || bSkipClassPicker)
	{
		// If there is only one derived class, skip the class picker
		return true;
	}
	
	// Null the parent class to ensure one is selected
	ParentClass = nullptr;

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.ExtraPickerCommonClasses.Empty();
	Options.DisplayMode = EClassViewerDisplayMode::ListView;
	Options.bAllowViewOptions = true;
	Options.bExpandRootNodes = true;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowObjectRootClass = true;

	TArray<UClass*> CommonClasses = CommonEffectClasses;
	Options.ExtraPickerCommonClasses = CommonClasses;

	TSharedPtr<FModularGameplayAbilitiesClassFilter> Filter = MakeShareable(new FModularGameplayAbilitiesClassFilter(UGameplayEffect::StaticClass()));
	Filter->DisallowedChildrenOfClasses.Add(UBlueprintMapLibrary::StaticClass());
	Options.ClassFilters.Add(Filter.ToSharedRef());

	const FText TitleText = LOCTEXT("PickParentClass", "Pick Parent Class for New Gameplay Ability");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UGameplayEffect::StaticClass());

	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}

UObject* UGameplayEffectFactory::FactoryCreateNew(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	if ((ParentClass == nullptr) || !FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass))
	{
		FFormatNamedArguments Args;
		Args.Add( TEXT("ClassName"), (ParentClass != nullptr) ? FText::FromString( ParentClass->GetName() ) : LOCTEXT("Null", "(null)") );
		FMessageDialog::Open( EAppMsgType::Ok, FText::Format( LOCTEXT("CannotCreateBlueprintFromClass", "Cannot create a blueprint based on the class '{ClassName}'."), Args ) );
		return nullptr;
	}

	UClass* BlueprintClass = nullptr;
	UClass* BlueprintGeneratedClass = nullptr;

	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
	KismetCompilerModule.GetBlueprintTypesForClass(ParentClass, BlueprintClass, BlueprintGeneratedClass);

	UBlueprint* Blueprint =
		FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, InName, BPTYPE_Normal, BlueprintClass, BlueprintGeneratedClass, CallingContext);

	// Mark the Blueprint as Data-Only
	Blueprint->bRecompileOnLoad = false;

	return Blueprint;
}

#undef LOCTEXT_NAMESPACE
