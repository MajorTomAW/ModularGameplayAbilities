// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"
#include "UObject/Object.h"

#include "ModularAbilityCost.generated.h"

class UObject;
class UGameplayAbility;
class UModularGameplayAbility;
struct FGameplayTagContainer;
struct FGameplayAbilityActorInfo;

/** Base struct for all per-ability costs that can be applied with the ability system. */
USTRUCT(BlueprintType)
struct MODULARGAMEPLAYABILITIES_API FModularAbilityCost
{
	GENERATED_BODY()
	friend class UModularGameplayAbility;
	
public:
	FModularAbilityCost()
		: bOnlyApplyCostOnHit(false)
	{
	}
	
	virtual ~FModularAbilityCost() = default;

public:
	/**
	 * Checks if we can afford this cost.
	 *
	 * A failure reason tag can be added to OptionalRelevantTags (if non-null),
	 * which can be queried elsewhere to determine how to provide user feedback.
	 * (e.g., a clicking noise if a weapon is out of ammo)
	 *
	 * Ability and ActorInfo are guaranteed to be non-null on entry, but OptionalRelevantTags can be nullptr.
	 *
	 * @returns True, if we can pay for the ability, false otherwise.
	 */
	virtual bool CheckCost(const UModularGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
	{
		// By default, we can always afford the cost.
		return true;
	}

	/**
	 * Applies the ability's cost to the target.
	 *
	 * Notes:
	 *	– Your implementation doesn't need to check ShouldOnlyApplyCostOnHit(), the caller does that for you.
	 *	– Ability and ActorInfo are guaranteed to be non-null on entry.
	 */
	virtual void ApplyCost(const UModularGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
	{
		// By default, we don't apply any cost.
	}

	/** Returns whether this cost should only be applied if this ability hits successfully. */
	bool ShouldOnlyApplyCostOnHit() const { return bOnlyApplyCostOnHit; }

protected:
	/** If true, this cost should only be applied if this ability hits successfully (such as melee weapon durability) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Costs)
	uint8 bOnlyApplyCostOnHit : 1;
};
