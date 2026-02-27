// Author: Tom Werner (MajorT), 2026 February


#include "AttributeInitializationKeyPropertyCustomization.h"

#include "AbilitySystemGlobals.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "ModularAbilitySystemGlobals.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "ModularGameplayAbilitiesEditor"

FAttributeInitializationKeyPropertyCustomization::FAttributeInitializationKeyPropertyCustomization()
{
	for (TObjectIterator<UAttributeSet> It; It; ++It)
	{
		UAttributeSet* Set = *It;
		if (!Set)
		{
			continue;
		}

		AllSetNames.Add(Set->GetName());
	}
}

void FAttributeInitializationKeyPropertyCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	StructHandle = PropertyHandle;

	HeaderRow
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		PropertyHandle->CreatePropertyValueWidget(false)
	];
}

void FAttributeInitializationKeyPropertyCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	CategoryHandle = PropertyHandle->GetChildHandle("Category");
	SubcategoryHandle = PropertyHandle->GetChildHandle("Subcategory");
	const FName PropertyName = PropertyHandle->GetProperty()->GetFName();

	ChildBuilder.AddCustomRow(LOCTEXT("AttributeInitKey_CategoryName", "Category"))
		.RowTag(PropertyName)
		.NameContent()
		[
			CategoryHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MaxDesiredWidth(0.f)
		[
			CategoryHandle->CreatePropertyValueWidget()
		];

	FPropertyComboBoxArgs ComboArgs(SubcategoryHandle,
		FOnGetPropertyComboBoxStrings::CreateSP(this, &ThisClass::OnGetSubcategoryRowStrings),
		FOnGetPropertyComboBoxValue::CreateSP(this, &ThisClass::OnGetSubcategoryValueString));
	ComboArgs.ShowSearchForItemCount = 1;

	ChildBuilder.AddCustomRow(LOCTEXT("AttributeInitKey_SubcategoryName", "Subcategory"))
		.RowTag(PropertyName)
		.NameContent()
		[
			SubcategoryHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MaxDesiredWidth(0.f)
		[
			PropertyCustomizationHelpers::MakePropertyComboBox(ComboArgs)
		];
}

void FAttributeInitializationKeyPropertyCustomization::OnGetSubcategoryRowStrings(
	TArray<TSharedPtr<FString>>& OutStrings,
	TArray<TSharedPtr<SToolTip>>& OutToolTips,
	TArray<bool>& OutRestrictedItems) const
{
	TArray<FString> CategoryNames = GetSubcategoryNamesForCategory(GetCategoryValueString());

	for (const FString& Name : CategoryNames)
	{
		OutStrings.Add(MakeShared<FString>(Name));
		OutRestrictedItems.Add(false);
	}
}

FString FAttributeInitializationKeyPropertyCustomization::OnGetSubcategoryValueString() const
{
	if (!SubcategoryHandle.IsValid())
	{
		return TEXT("None");
	}

	FName NameValue;
	const FPropertyAccess::Result Result = SubcategoryHandle->GetValue(NameValue);
	if (Result == FPropertyAccess::Success)
	{
		if (NameValue.IsNone() || !GetSubcategoryNamesForCategory(GetCategoryValueString()).Contains(NameValue.ToString()))
		{
			return TEXT("None");
		}

		return NameValue.ToString();
	}

	if (Result == FPropertyAccess::Fail)
	{
		return TEXT("None");
	}

	return TEXT("Multiple Values");
}

FString FAttributeInitializationKeyPropertyCustomization::GetCategoryValueString() const
{
	if (!CategoryHandle.IsValid())
	{
		return TEXT("None");
	}

	FName NameValue;
	const FPropertyAccess::Result Result = CategoryHandle->GetValue(NameValue);
	if (Result == FPropertyAccess::Success)
	{
		if (NameValue.IsNone())
		{
			return TEXT("None");
		}

		return NameValue.ToString();
	}

	if (Result == FPropertyAccess::Fail)
	{
		return TEXT("None");
	}

	return TEXT("Multiple Values");
}

bool FAttributeInitializationKeyPropertyCustomization::IsSubcategoryAllowed(
	const FString& Subcategory) const
{
	return !AllSetNames.Contains(Subcategory);
}

TArray<FString> FAttributeInitializationKeyPropertyCustomization::GetSubcategoryNamesForCategory(
	const FString& Category) const
{
	const TArray<UCurveTable*>& Tables =
		static_cast<UModularAbilitySystemGlobals&>(UAbilitySystemGlobals::Get()).GetGlobalAttributeDefaultsTables();

	TArray<FString> CategoryNames;
	CategoryNames.Add(FName(NAME_None).ToString());
	for (const auto& Table : Tables)
	{
		const auto& RowMap = Table->GetRowMap();

		// We will loop over all row maps and extract the characters until the first period
		for (const auto& RowPair : RowMap)
		{
			FString RowNameString = RowPair.Key.ToString();

			TArray<FString> Tokens;
			RowNameString.ParseIntoArray(Tokens, TEXT("."));
			if (Tokens.Num() >= 2)
			{
				// Filter for category and make sure this token is NOT an attribute set
				if (Tokens[0] == Category && IsSubcategoryAllowed(Tokens[1]))

					CategoryNames.AddUnique(Tokens[1]);
			}
		}
	}

	return MoveTemp(CategoryNames);
}

#undef LOCTEXT_NAMESPACE
