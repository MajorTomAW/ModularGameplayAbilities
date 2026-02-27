// Author: Tom Werner (MajorT), 2026 February

#pragma once


class FAttributeInitializationKeyPropertyCustomization : public IPropertyTypeCustomization
{
	using ThisClass = FAttributeInitializationKeyPropertyCustomization;
public:

	FAttributeInitializationKeyPropertyCustomization();

	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShared<FAttributeInitializationKeyPropertyCustomization>();
	}

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	void OnGetSubcategoryRowStrings(TArray<TSharedPtr<FString>>& OutStrings, TArray<TSharedPtr<SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems) const;
	FString OnGetSubcategoryValueString() const;
	FString GetCategoryValueString() const;
	bool IsSubcategoryAllowed(const FString& Subcategory) const;
	TArray<FString> GetSubcategoryNamesForCategory(const FString& Category) const;

	TSharedPtr<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyHandle> CategoryHandle;
	TSharedPtr<IPropertyHandle> SubcategoryHandle;

	TArray<FString> AllSetNames;
};
