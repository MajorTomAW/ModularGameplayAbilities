// Copyright © 2024 MajorT. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "Styles/ModularGameplayAbilitiesEditorStyle.h"

#define LOCTEXT_NAMESPACE "FModularGameplayAbilitiesEditorModule"

class FModularGameplayAbilitiesEditorModule : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FModularGameplayAbilitiesEditorStyle::Get();
	}
};

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FModularGameplayAbilitiesEditorModule, ModularGameplayAbilitiesEditor)