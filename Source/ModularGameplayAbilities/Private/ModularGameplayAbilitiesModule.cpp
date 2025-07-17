// Author: Tom Werner (MajorT), 2025

#include "ModularGameplayAbilitiesModule.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ModularGameplayAbilitiesSettings.h"
#endif

#define LOCTEXT_NAMESPACE "FModularGameplayAbilitiesModule"

class FModularGameplayAbilitiesModule final : public IModularGameplayAbilitiesModule
{
public:
	virtual void StartupModule() override
	{
#if WITH_EDITOR
		ISettingsModule& Settings = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
		Settings.RegisterSettings("Project", "Game", "ModularGameplayAbilities",
			LOCTEXT("ModularGameplayAbilitiesSettingsName", "Modular Abilities Settings"),
			LOCTEXT("ModularGameplayAbilitiesSettingsDescription", "Configure the Modular Gameplay Abilities settings."),
			GetMutableDefault<UModularGameplayAbilitiesSettings>());
#endif
	}

	virtual void ShutdownModule() override
	{
#if WITH_EDITOR
		ISettingsModule& Settings = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
		Settings.UnregisterSettings("Project", "Game", "ModularGameplayAbilities");
#endif
	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FModularGameplayAbilitiesModule, ModularGameplayAbilities)