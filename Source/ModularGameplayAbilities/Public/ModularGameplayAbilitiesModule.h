// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/** Public interface to the ModularGameplayAbilities module */
class IModularGameplayAbilitiesModule : public IModuleInterface
{
public:
	/**
	 * Singleton-like access to this module's interface. This is just for convenience!
	 * Be careful calling this during the shutdown phase, as the module may have been unloaded already.
	 *
	 * @returns The singleton instance, loading the module on demand if needed
	 */
	static IModularGameplayAbilitiesModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IModularGameplayAbilitiesModule>("ModularGameplayAbilities");
	}

	/**
	 * Checks to see if this module is loaded and ready to use.
	 *
	 * @returns True if the module is loaded and ready to use
	 */
	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ModularGameplayAbilities");
	}
};
