// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/ModularGameplayAbilityTypes.h"

#include "ModularAbilitySystemComponent.generated.h"

class UModularGameplayAbility;
/**
 * Extended version of the UAbilitySystemComponent
 */
UCLASS()
class MODULARGAMEPLAYABILITIES_API UModularAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UModularAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	typedef TFunctionRef<bool(const UModularGameplayAbility* Ability, FGameplayAbilitySpecHandle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	// ----------------------------------------------------------------------------------------------------------------
	//	Activation Groups
	// ----------------------------------------------------------------------------------------------------------------

	/** Returns true, if the specified activation group is blocked. */
	bool IsActivationGroupBlocked(EGameplayAbilityActivationGroup::Type Group) const;

	/** Adds the ability to the specified activation group count. */
	void AddAbilityToActivationGroup(EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* Ability);

	/** Removes the ability from the specified activation group count. */
	void RemoveAbilityFromActivationGroup(EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* Ability);

	/** Cancels all abilities in the specified activation group. */
	void CancelActivationGroupAbilities(EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* AbilityToIgnore, bool bReplicateCancelAbilities = true);

protected:
	/** Cached number of abilities running in each activation group. */
	int32 ActivationGroupCounts[static_cast<uint8>(EGameplayAbilityActivationGroup::MAX)];
};
