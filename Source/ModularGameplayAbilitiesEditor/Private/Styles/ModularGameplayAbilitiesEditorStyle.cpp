// Copyright © 2024 MajorT. All Rights Reserved.


#include "ModularGameplayAbilitiesEditorStyle.h"

#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

TUniquePtr<FModularGameplayAbilitiesEditorStyle> FModularGameplayAbilitiesEditorStyle::StyleSet = nullptr;

FModularGameplayAbilitiesEditorStyle::FModularGameplayAbilitiesEditorStyle()
	: FSlateStyleSet("ModularGameplayAbilitiesEditorStyle")
{
}

FModularGameplayAbilitiesEditorStyle::~FModularGameplayAbilitiesEditorStyle()
{
	Shutdown();
}

FModularGameplayAbilitiesEditorStyle& FModularGameplayAbilitiesEditorStyle::Get()
{
	if (!StyleSet.IsValid())
	{
		StyleSet = MakeUnique<FModularGameplayAbilitiesEditorStyle>();
		StyleSet->Initialize();
	}

	return *StyleSet.Get();
}

void FModularGameplayAbilitiesEditorStyle::Initialize()
{
	FSlateStyleSet::SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	FSlateStyleSet::SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	Set("ClassIcon.ModularAbilitySet", new IMAGE_BRUSH("Icons/AssetIcons/AbilitySystemComponent_16x", CoreStyleConstants::Icon16x16));
	Set("ClassThumbnail.ModularAbilitySet", new IMAGE_BRUSH("Icons/AssetIcons/AbilitySystemComponent_64x", CoreStyleConstants::Icon64x64));

	FSlateStyleRegistry::RegisterSlateStyle(*this);
}

void FModularGameplayAbilitiesEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet);
		StyleSet.Reset();
	}
}
