// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularAbilityCost.h"

#include "ModularAbilityCost_AdditionalGameplayEffect.generated.h"

class UGameplayEffect;

/** Represents a cost that provides an additional gameplay effect to be applied to the ability system.
 * 
 * Note that this cost is pretty much pointless
 * and only exists for the sake of giving an example of how these costs can be used.
 */
USTRUCT(DisplayName="Additional Cost Effect")
struct FModularAbilityCost_AdditionalGameplayEffect : public FModularAbilityCost
{
	GENERATED_BODY()

public:
	FModularAbilityCost_AdditionalGameplayEffect();

	//~ Begin FModularAbilityCost Interface
	virtual bool CheckCost(const UModularGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ApplyCost(const UModularGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	//~ End FModularAbilityCost Interface

protected:
	/** The gameplay effect to apply. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Costs)
	TSubclassOf<UGameplayEffect> AdditionalCostEffect;

	/** The failure tag to apply if the cost check fails. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Costs)
	FGameplayTag FailureTag;
};