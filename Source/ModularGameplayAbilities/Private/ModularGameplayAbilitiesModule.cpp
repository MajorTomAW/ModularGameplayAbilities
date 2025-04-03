// Copyright © 2024 MajorT. All Rights Reserved.

#include "ModularGameplayAbilitiesModule.h"

#define LOCTEXT_NAMESPACE "FModularGameplayAbilitiesModule"

class FModularGameplayAbilitiesModule final : public IModularGameplayAbilitiesModule
{
public:
	virtual void StartupModule() override
	{
		
	}

	virtual void ShutdownModule() override
	{
		
	}
};

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FModularGameplayAbilitiesModule, ModularGameplayAbilities)