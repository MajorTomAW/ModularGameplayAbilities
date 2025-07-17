// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Styling/SlateStyle.h"

class FModularGameplayAbilitiesEditorStyle final : public FSlateStyleSet
{
public:
	FModularGameplayAbilitiesEditorStyle();
	virtual ~FModularGameplayAbilitiesEditorStyle();
	friend class FModularGameplayAbilitiesEditorModule;

	/** Returns the singleton instance of the style set */
	static FModularGameplayAbilitiesEditorStyle& Get();

protected:
	void Initialize();
	static void Shutdown();

private:
	static TUniquePtr<FModularGameplayAbilitiesEditorStyle> StyleSet;
};