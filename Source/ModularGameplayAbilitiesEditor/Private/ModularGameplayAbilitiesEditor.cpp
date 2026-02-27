// Author: Tom Werner (MajorT), 2025

#include "ISettingsModule.h"
#include "AssetTools/GameplayAbilityFactory.h"
#include "Customization/AttributeInitializationKeyPropertyCustomization.h"
#include "Modules/ModuleManager.h"
#include "Styles/ModularGameplayAbilitiesEditorStyle.h"

#define LOCTEXT_NAMESPACE "FModularGameplayAbilitiesEditorModule"

class FModularGameplayAbilitiesEditorModule : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FModularGameplayAbilitiesEditorStyle::Get();

		ISettingsModule& Settings = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
		Settings.RegisterSettings("Project", "Editor", "GameplayEffectFactory",
			LOCTEXT("GameplayEffectFactorySettingsName", "Gameplay Effect Factory"),
			LOCTEXT("GameplayEffectFactorySettingsDescription", "Configure the Gameplay Effect Factory settings."),
			GetMutableDefault<UGameplayEffectFactory>());

		Settings.RegisterSettings("Project", "Editor", "GameplayAbilityFactory",
			LOCTEXT("GameplayAbilityFactorySettingsName", "Gameplay Ability Factory"),
			LOCTEXT("GameplayAbilityFactorySettingsDescription", "Configure the Gameplay Ability Factory settings."),
			GetMutableDefault<UGameplayAbilityFactory>());

		FPropertyEditorModule& PropEdModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropEdModule.RegisterCustomPropertyTypeLayout("AttributeInitializationKey", FOnGetPropertyTypeCustomizationInstance::CreateStatic(FAttributeInitializationKeyPropertyCustomization::MakeInstance));
	}

	virtual void ShutdownModule() override
	{
		ISettingsModule& Settings = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
		Settings.UnregisterSettings("Project", "Editor", "GameplayEffectFactory");
		Settings.UnregisterSettings("Project", "Editor", "GameplayAbilityFactory");

		FPropertyEditorModule& PropEdModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropEdModule.UnregisterCustomPropertyTypeLayout("AttributeInitializationKey");
	}
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularGameplayAbilitiesEditorModule, ModularGameplayAbilitiesEditor)
