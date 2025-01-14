// Copyright © 2024 MajorT. All Rights Reserved.


#include "GameplayAbilityFactory.h"

#include "ClassViewerModule.h"
#include "KismetCompilerModule.h"
#include "ModularGameplayAbilitiesClassFilter.h"
#include "Abilities/GameplayAbility.h"
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
	BlueprintType = BPTYPE_Normal;

	CommonAbilityClasses.Add(UGameplayAbility::StaticClass());

	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(UGameplayAbility::StaticClass(), DerivedClasses);
	bSkipClassPicker = DerivedClasses.Num() == 0;
}

FText UGameplayAbilityFactory::GetDisplayName() const
{
	return LOCTEXT("GameplayAbilityFactoryDisplayName", "Gameplay Ability");
}

bool UGameplayAbilityFactory::ConfigureProperties()
{
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(ParentClass, DerivedClasses, false);
	if (bSkipClassPicker || DerivedClasses.Num() == 0)
	{
		// If there is only one derived class, skip the class picker
		return true;
	}
	
	// Null the parent class to ensure one is selected
	ParentClass = nullptr;

	// Fill in options
	TSharedPtr<FModularGameplayAbilitiesClassFilter> Filter = MakeShareable(new FModularGameplayAbilitiesClassFilter(UGameplayAbility::StaticClass()));
	FClassViewerInitializationOptions Options;
	{
		Options.Mode = EClassViewerMode::ClassPicker;
		Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
		Options.bShowNoneOption = false;
		Options.ClassFilters.Add(Filter.ToSharedRef());
		Options.ExtraPickerCommonClasses = CommonAbilityClasses;
		Options.bShowDefaultClasses = false;
	}

	const FText TitleText = LOCTEXT("PickParentClass", "Pick Parent Class for New Gameplay Ability");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UBlueprint::StaticClass());

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
		FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, InName, BlueprintType, BlueprintClass, BlueprintGeneratedClass, CallingContext);

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
	BlueprintType = BPTYPE_Normal;
	bSkipClassPicker = false;

	OnConfigurePropertiesDelegate.BindLambda([&](FClassViewerInitializationOptions* Options)
	{
		Options->bShowNoneOption = false;
		Options->ClassFilters.Reset();
		Options->ClassFilters.Add(MakeShareable(new FModularGameplayAbilitiesClassFilter(UGameplayEffect::StaticClass())));
		Options->ExtraPickerCommonClasses.Reset();
		Options->ExtraPickerCommonClasses = CommonEffectClasses;
		Options->bIsBlueprintBaseOnly = false;
		Options->DisplayMode = EClassViewerDisplayMode::ListView;
		Options->bAllowViewOptions = false;
		Options->bShowDefaultClasses = false;
	});
}

FText UGameplayEffectFactory::GetDisplayName() const
{
	return LOCTEXT("GameplayEffectFactoryDisplayName", "Gameplay Effect");
}

bool UGameplayEffectFactory::ConfigureProperties()
{
	return Super::ConfigureProperties();
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
		FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, InName, BlueprintType, BlueprintClass, BlueprintGeneratedClass, CallingContext);

	// Mark the Blueprint as Data-Only
	Blueprint->bRecompileOnLoad = false;

	return Blueprint;
}

#undef LOCTEXT_NAMESPACE
