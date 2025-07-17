// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/ModularGameplayAbilityTypes.h"

#include "ModularAbilitySystemComponent.generated.h"

class UModularAbilityTagRelationshipMapping;
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
	
	void AbilityInputIdPressed(int32 InputId);
	void AbilityInputIdReleased(int32 InputId);

	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();

	/** Whether we currently allow processing ability input. */
	virtual bool IsAbilityInputAllowed() const { return true; }

	/** Deferred set base attribute value from replication. */
	virtual void DeferredSetBaseAttributeValueFromReplication(const FGameplayAttribute& Attribute, float NewValue);
	virtual void DeferredSetBaseAttributeValueFromReplication(const FGameplayAttribute& Attribute, FGameplayAttributeData NewValue);

	/** Tries to activate all passive abilities on spawn. */
	void TryActivateAbilitiesOnSpawn();

	/** Gets the ability target data associated with the given ability handle and activation info. */
	virtual void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, const FGameplayAbilityActivationInfo& ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	// ----------------------------------------------------------------------------------------------------------------
	//	Activation Groups
	// ----------------------------------------------------------------------------------------------------------------

	/** Returns true if the specified activation group is blocked. */
	bool IsActivationGroupBlocked(EGameplayAbilityActivationGroup::Type Group) const;

	/** Adds the ability to the specified activation group count. */
	void AddAbilityToActivationGroup(EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* Ability);

	/** Removes the ability from the specified activation group count. */
	void RemoveAbilityFromActivationGroup(EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* Ability);

	/** Cancels all abilities in the specified activation group. */
	void CancelActivationGroupAbilities(EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* AbilityToIgnore, bool bReplicateCancelAbilities = true);

	/** Sets the current tag relationship mapping. */
	UFUNCTION(BlueprintCallable, Category = AbilitySystem)
	virtual void SetTagRelationshipMapping(UModularAbilityTagRelationshipMapping* NewMapping);

	/** Clears the current tag relationship mapping. */
	UFUNCTION(BlueprintCallable, Category = AbilitySystem)
	virtual void ClearTagRelationshipMapping();

	/** Looks at ability tags and gathers additional required and blocked tags. */
	virtual void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

	/** Handles the ability failed to activate. */
	virtual void HandleAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	/** Notifies the client about the ability failure. */
	UFUNCTION(Client, Unreliable)
	void ClientNotifyAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason);

	/** Returns all tracked actors for a specified ability. */
	UFUNCTION(BlueprintCallable, Category = Tracking)
	FGameplayAbilitySpecHandle GetTrackedActorsForAbility(const UGameplayAbility* Ability, TArray<FAbilityTrackedActorEntry>& OutTrackedActors) const;

	/** Returns the number of tracked actors for a specified ability. */
	UFUNCTION(BlueprintCallable, Category = Tracking)
	int32 GetNumTrackedActorsForAbility(const UGameplayAbility* Ability) const;

	/** Starts tracking the specified actor for the given ability. */
	UFUNCTION(BlueprintCallable, Category = Tracking)
	bool StartTrackingActorForAbility(AActor* ActorToTrack, const UGameplayAbility* Ability);

	/** Starts tracking the specified actor for the given ability with a group tag. */
	UFUNCTION(BlueprintCallable, Category = Tracking)
	bool StartTrackingActorsForTag(AActor* ActorToTrack, const FGameplayTag& GroupTag);

	/** Returns all tracked actors for a specified tag. */
	UFUNCTION(BlueprintCallable, Category = Tracking)
	void GetTrackedActorsForTag(const FGameplayTag& Tag, TArray<FAbilityTrackedActorEntry>& OutTrackedActors) const;

	/** Clears all tracked actors for the specified group tag. */
	UFUNCTION(BlueprintCallable, Category = Tracking)
	void ClearTrackedGroupedActors(FGameplayTag GroupTag, bool bDestroyActors = false);

	/** Clears all tracked actors for the specified ability. */
	UFUNCTION(BlueprintCallable, Category = Tracking)
	void ClearTrackedActorsForAbility(const UGameplayAbility* Ability, bool bDestroyActors = false);

public:
	//~ Begin UAbilitySystemComponent Interface
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;

	virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;

	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
	//~ End UAbilitySystemComponent Interface

protected:
	/** If set, this table is used to look up tag relationships for abilities. */
	UPROPERTY()
	TObjectPtr<UModularAbilityTagRelationshipMapping> TagRelationshipMapping;
	
	TArray<FGameplayAbilitySpecHandle> InputPressedHandles;		// Handles to abilities that input activated this frame
	TArray<FGameplayAbilitySpecHandle> InputReleasedHandles;	// Handles to abilities that input released this frame
	TArray<FGameplayAbilitySpecHandle> InputHeldHandles;		// Handles to abilities that are currently input held
	
	/** Cached number of abilities running in each activation group. */
	int32 ActivationGroupCounts[static_cast<uint8>(EGameplayAbilityActivationGroup::MAX)];

public:
	/** Currently tracked actors for each tag. */
	TMap<FGameplayTag, TArray<FAbilityTrackedActorEntry>> TagTrackedActors;

	/** Currently tracked actors for each ability spec. */
	TMap<FGameplayAbilitySpecHandle, TArray<FAbilityTrackedActorEntry>> AbilitySpecTrackedActors;
};
