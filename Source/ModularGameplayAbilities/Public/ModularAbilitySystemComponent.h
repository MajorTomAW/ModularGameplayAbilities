// Copyright © 2024 MajorT. All Rights Reserved.

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

	//~ Begin UAbilitySystemComponent Interface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End UAbilitySystemComponent Interface

	typedef TFunctionRef<bool(const UModularGameplayAbility* Ability, FGameplayAbilitySpecHandle)> TShouldCancelAbilityFunc;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);

	void CancelInputActivatedAbilities(bool bReplicateCancelAbility);
	
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();

	/** Whether we currently allow processing ability input. */
	virtual bool IsAbilityInputAllowed() const { return true; }

	/** Deferred set base attribute value from replication. */
	virtual void DeferredSetBaseAttributeValueFromReplication(const FGameplayAttribute& Attribute, float NewValue);
	virtual void DeferredSetBaseAttributeValueFromReplication(const FGameplayAttribute& Attribute, FGameplayAttributeData NewValue);

	/** Tries to activate all passive abilities on spawn. */
	void TryActivateAbilitiesOnSpawn();

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

public:
	//~ Begin UAbilitySystemComponent Interface
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;
	//~ End UAbilitySystemComponent Interface

protected:
	TArray<FGameplayAbilitySpecHandle> InputPressedHandles;		// Handles to abilities that input activated this frame
	TArray<FGameplayAbilitySpecHandle> InputReleasedHandles;	// Handles to abilities that input released this frame
	TArray<FGameplayAbilitySpecHandle> InputHeldHandles;		// Handles to abilities that are currently input held
	
	/** Cached number of abilities running in each activation group. */
	int32 ActivationGroupCounts[static_cast<uint8>(EGameplayAbilityActivationGroup::MAX)];
};
