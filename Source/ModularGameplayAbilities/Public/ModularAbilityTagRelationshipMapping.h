// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "ModularAbilityTagRelationshipMapping.generated.h"

/**
 * Struct that defines the relationship between different ability tags.
 */
USTRUCT(BlueprintType)
struct FModularAbilityTagRelationship
{
	GENERATED_BODY()

public:
	/** The tag that this container relationship is about. Single tag, but abilities can have multiple of these. */
	UPROPERTY(EditDefaultsOnly, Category = Relationship, meta = (Categories = "Gameplay.Action"))
	FGameplayTag AbilityTag;

	/** Whether the ability tag must be an exact match, or if it can be a partial match. */
	UPROPERTY(EditDefaultsOnly, Category = Relationship)
	uint8 bMatchPartialTag : 1 = true;

	/** The other ability tags that will be blocked by any ability using this tag. */
	UPROPERTY(EditDefaultsOnly, Category = Relationship)
	FGameplayTagContainer AbilityTagsToBlock;

	/** The other ability tags that will be canceled by any ability using this tag. */
	UPROPERTY(EditDefaultsOnly, Category = Relationship)
	FGameplayTagContainer AbilityTagsToCancel;

	/** If an ability has the tag, this is implicitly added to the activation-required tags of the ability. */
	UPROPERTY(EditDefaultsOnly, Category = Ability)
	FGameplayTagContainer ActivationRequiredTags;

	/** If an ability has the tag, this is implicitly added to the activation-blocked tags of the ability. */
	UPROPERTY(EditDefaultsOnly, Category = Ability)
	FGameplayTagContainer ActivationBlockedTags;
};

/**
 * Data asset that maps relationships between gameplay tags and gameplay abilities
 * Such as how a tag may block an ability from being activated or cancel an ability that is already active
 */
UCLASS(BlueprintType, Const)
class MODULARGAMEPLAYABILITIES_API UModularAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

public:
	UModularAbilityTagRelationshipMapping();

	/** Given a set of ability tags, parse the tag relationships and fill out tags to block and cancel. */
	virtual void GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const;

	/** Given a set of ability tags, add additional required and blocking tags. */
	virtual void GetActivationRequiredAndBlockedTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequiredTags, FGameplayTagContainer* OutActivationBlockedTags) const;

	/** Returns true if the specified ability tags are canceled by the passed in action tag. */
	virtual bool IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const;

protected:
	/** The list of relationships between different gameplay tags (which ones block or cancel others). */
	UPROPERTY(EditDefaultsOnly, Category = Relationships, meta = (TitleProperty = "AbilityTag"))
	TArray<FModularAbilityTagRelationship> AbilityTagRelationships;
};
