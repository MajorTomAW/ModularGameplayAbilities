// Author: Tom Werner (MajorT), 2026 February


#include "ModularAbilitySystemGlobals.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularAbilitySystemGlobals)

#if WITH_EDITOR
TArray<FName> UModularAbilitySystemGlobals::GetCategoryNames()
{
	const auto& Tables =
		static_cast<UModularAbilitySystemGlobals&>(UAbilitySystemGlobals::Get()).GetGlobalAttributeDefaultsTables();

	TSet<FName> CategoryNames;
	CategoryNames.Add(NAME_None); // Add NONE as an option
	for (const auto& Table : Tables)
	{
		const auto& RowMap = Table->GetRowMap();

		// We will loop over all row maps and extract the characters until the first period
		for (const auto& RowPair : RowMap)
		{
			FString RowNameString = RowPair.Key.ToString();

			TArray<FString> Tokens;
			RowNameString.ParseIntoArray(Tokens, TEXT("."));
			if (!Tokens.IsEmpty())
			{
				CategoryNames.Add(*Tokens[0]);
			}
		}
	}

	return CategoryNames.Array();
}
#endif
