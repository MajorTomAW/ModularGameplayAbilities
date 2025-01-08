// Copyright © 2024 Playton. All Rights Reserved.


#include "ModularAbilityTagRelationshipMapping.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularAbilityTagRelationshipMapping)

UModularAbilityTagRelationshipMapping::UModularAbilityTagRelationshipMapping()
{
}

void UModularAbilityTagRelationshipMapping::GetAbilityTagsToBlockAndCancel(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer* OutTagsToBlock,
	FGameplayTagContainer* OutTagsToCancel) const
{
	for (int i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const auto& Relationship = AbilityTagRelationships[i];
		const bool bTagMatches = Relationship.bMatchPartialTag
			? AbilityTags.HasTag(Relationship.AbilityTag)
			: AbilityTags.HasTagExact(Relationship.AbilityTag);
		
		if (bTagMatches)
		{
			if (OutTagsToBlock)
			{
				OutTagsToBlock->AppendTags(Relationship.AbilityTagsToBlock);
			}

			if (OutTagsToCancel)
			{
				OutTagsToCancel->AppendTags(Relationship.AbilityTagsToCancel);
			}
		}
	}
}

void UModularAbilityTagRelationshipMapping::GetActivationRequiredAndBlockedTags(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer* OutActivationRequiredTags,
	FGameplayTagContainer* OutActivationBlockedTags) const
{
	for (int i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const auto& Relationship = AbilityTagRelationships[i];
		const bool bTagMatches = Relationship.bMatchPartialTag
			? AbilityTags.HasTag(Relationship.AbilityTag)
			: AbilityTags.HasTagExact(Relationship.AbilityTag);

		if (bTagMatches)
		{
			if (OutActivationRequiredTags)
			{
				OutActivationRequiredTags->AppendTags(Relationship.ActivationRequiredTags);
			}

			if (OutActivationBlockedTags)
			{
				OutActivationBlockedTags->AppendTags(Relationship.ActivationBlockedTags);
			}
		}
	}
}

bool UModularAbilityTagRelationshipMapping::IsAbilityCancelledByTag(
	const FGameplayTagContainer& AbilityTags,
	const FGameplayTag& ActionTag) const
{
	for (int i = 0; i < AbilityTagRelationships.Num(); i++)
	{
		const auto& Relationship = AbilityTagRelationships[i];

		if (Relationship.AbilityTag == ActionTag && Relationship.AbilityTagsToCancel.HasAny(AbilityTags))
		{
			return true;
		}
	}

	return false;
}
