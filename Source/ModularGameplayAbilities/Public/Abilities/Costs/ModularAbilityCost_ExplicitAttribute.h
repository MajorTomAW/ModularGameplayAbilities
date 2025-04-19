// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularAbilityCost.h"

#include "ModularAbilityCost_ExplicitAttribute.generated.h"

/** Represents a cost that uses up an explicitly set attribute and value. */
USTRUCT(DisplayName="Explicit Attribute Cost")
struct FModularAbilityCost_ExplicitAttribute : public FModularAbilityCost
{
	GENERATED_BODY()

public:
	FModularAbilityCost_ExplicitAttribute();

	//~ Begin FModularAbilityCost Interface
	virtual bool CheckCost(const UModularGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UModularGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~ End FModularAbilityCost Interface

protected:
	/** The attribute to use for the cost. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Costs)
	FGameplayAttribute Attribute;

	/** How much of the attribute to consume. (keyed on ability level).
	 * Note that this value will be subtracted from the attribute value, so it should be positive. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Costs)
	FScalableFloat CostValue;

	/** Explicit failure tag to send back as a response in case this cost can't be paid. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Costs)
	FGameplayTag FailureTag;
};