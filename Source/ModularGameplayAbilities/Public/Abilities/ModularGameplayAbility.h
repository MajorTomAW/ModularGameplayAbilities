// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGameplayAbilityTypes.h"
#include "Abilities/GameplayAbility.h"
#include "StructUtils/InstancedStruct.h"
#include "ModularGameplayAbility.generated.h"

struct FModularAbilityCost;
/**
 * Extended version of the UGameplayAbility
 * Adds more functionality and customization options in the context of activation, failure, etc.
 */
UCLASS(Blueprintable, HideCategories = Input)
class MODULARGAMEPLAYABILITIES_API UModularGameplayAbility
	: public UGameplayAbility
	, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	friend class UModularAbilitySystemComponent;
	
public:
	UModularGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FModularAbilitySignature,
		const UModularGameplayAbility*, ModularAbility);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnApplyCooldownDelegate,
		const UModularGameplayAbility*, Ability, float, Duration, const FGameplayTagContainer&, CooldownTags);

public:
	// ----------------------------------------------------------------------------------------------------------------
	//	Ability Helper Functions
	// ----------------------------------------------------------------------------------------------------------------

	/** Returns the controller executing this ability. May be null. */
	UFUNCTION(BlueprintCallable, Category = Ability)
	AController* GetControllerFromActorInfo() const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Accessors
	// ----------------------------------------------------------------------------------------------------------------

	/** Returns how the ability is meant to be activated. */
	EGameplayAbilityActivationPolicy::Type GetActivationPolicy() const
	{
		return ActivationPolicy;
	}

	/** Returns how the ability activates in relation to other abilities. */
	EGameplayAbilityActivationGroup::Type GetActivationGroup() const
	{
		return ActivationGroup;
	}

	/** Returns whether the ability should still receive input when passive. */
	bool IsForceReceiveInput() const
	{
		return bForceReceiveInput;
	}

	/** Returns true if this ability should have an explicit cooldown duration. (aka it is not 0)*/
	bool HasExplicitCooldownDuration() const
	{
		return ExplicitCooldownDuration.Value > 0.0f;
	}

	/** Returns the explicit cooldown duration of this ability. */
	FScalableFloat GetExplicitCooldownDuration() const
	{
		return ExplicitCooldownDuration;
	}

	/** Returns true if the requested activation group is a valid transition from the current activation group. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = Ability, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool CanChangeActivationGroup(EGameplayAbilityActivationGroup::Type DesiredGroup) const;

	/** Attempts to change the activation group of this ability. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = Ability, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool ChangeActivationGroup(EGameplayAbilityActivationGroup::Type DesiredGroup);

	/** Tries to activate this ability on spawn. (For Passive abilities) */
	virtual void TryActivateAbilityOnSpawn(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) const;

	//~ Begin IGameplayTagAssetInterface Interface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~ End IGameplayTagAssetInterface Interface

	/**
	 * A delegate that gets fired whenever the ApplyCooldown() function is called on this ability.
	 * @Note This will only work property for an ability that is instantiated per actor.
	 */
	UPROPERTY(BlueprintAssignable)
	FOnApplyCooldownDelegate OnApplyCooldownDelegate;

	/** Returns true if this ability should receive input events. */
	virtual bool ShouldReceiveInputEvents() const;

	/** Called when this ability failed to activate for some reason. */
	inline void OnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const
	{
		FGameplayTagContainer OverridenFailedReason;
		K2_OverrideFailedReason(FailedReason,OverridenFailedReason);
		
		NativeOnAbilityFailedToActivate(OverridenFailedReason);
		ScriptOnAbilityFailedToActivate(OverridenFailedReason);
	}

protected:
	//~ Begin UGameplayAbility Interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void SetCanBeCanceled(bool bCanBeCanceled) override;

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual bool DoesAbilitySatisfyTagRequirements(const UAbilitySystemComponent& AbilitySystemComponent, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	//~ End UGameplayAbility Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~ End UObject Interface

	/** Called when the ability system is initialized with a pawn avatar. */
	virtual void OnPawnAvatarSet();

	/** Called when the ability fails to activate for some reason. */
	virtual void NativeOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	/** Called when the ability fails to activate for some reason. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability)
	void ScriptOnAbilityFailedToActivate(const FGameplayTagContainer& FailedReason) const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Blueprint Events
	// ----------------------------------------------------------------------------------------------------------------

	/** Optionally overrides the default ability failure reason for specific cases. */
	UFUNCTION(BlueprintNativeEvent, Category = Ability, DisplayName = OverrideFailedReason)
	void K2_OverrideFailedReason(const FGameplayTagContainer& FailedReason, FGameplayTagContainer& OverridenFailedReason) const;

	/** Called when this ability is granted to the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = OnAbilityAdded)
	void K2_OnAbilityAdded();

	/** Called when this ability is removed from the ability system component. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = OnAbilityRemoved)
	void K2_OnAbilityRemoved();

	/** Called when the ability system is initialized with a pawn avatar. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = OnPawnAvatarSet)
	void K2_OnPawnAvatarSet();

	/**
	 * Input-related events.
	 * These will only be called if this ability is set to receive input events,
	 * which can either be set by the ActivationPolicy or by setting bForceReceiveInput to true.
	 */
	
	/** Called when this ability's input is pressed. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = OnAbilityInputPressed)
	void K2_OnAbilityInputPressed(float TimeWaited);
	bool bHasBlueprintInputPressed;	// If not implemented, no need to listen or send replicated input events.

	/** Called when this ability's input is released. */
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, DisplayName = OnAbilityInputReleased)
	void K2_OnAbilityInputReleased(float TimeHeld);
	bool bHasBlueprintInputReleased; // If not implemented, no need to listen or send replicated input events.

	/** Callback function for when the input is pressed or released. */
	UFUNCTION()
	void OnInputChangedCallback(bool bIsPressed);

	/**
	 * Last time the input state of this ability was changed.
	 * Can be used to determine how long the input was pressed or released.
	 */
	float LastInputCallbackTime;

protected:
	// ----------------------------------------------------------------------------------------------------------------
	//	Activation
	// ----------------------------------------------------------------------------------------------------------------
	
	/** How the ability is meant to be activated. This can be used to gate abilities from being activated in certain ways. */
	UPROPERTY(EditDefaultsOnly, Category = Activation)
	TEnumAsByte<EGameplayAbilityActivationPolicy::Type> ActivationPolicy;

	/** Useful for passive or triggered abilities that should still receive input. */
	UPROPERTY(EditDefaultsOnly, Category = Activation, meta = (EditConditionHides, EditCondition = "ActivationPolicy != EGameplayAbilityActivationPolicy::Active"))
	uint8 bForceReceiveInput : 1;

	/** How the ability activates in relation to other abilities. */
	UPROPERTY(EditDefaultsOnly, Category = Activation)
	TEnumAsByte<EGameplayAbilityActivationGroup::Type> ActivationGroup;

	/** If true, the ability will be activated automatically when activation required tags are already present. */
	UPROPERTY(EditDefaultsOnly, Category = Activation, meta = (DisplayName = "Activate if Tags Already Present"))
	uint8 bActivateIfTagsAlreadyPresent : 1;

	// ----------------------------------------------------------------------------------------------------------------
	//	Display
	// ----------------------------------------------------------------------------------------------------------------

	/** Display name text of this ability to show in the HUD. */
	UPROPERTY(EditDefaultsOnly, Category = Display)
	FText DisplayName = FText::GetEmpty();

	/** Optional description text of this ability to explain what it does. */
	UPROPERTY(EditDefaultsOnly, Category = Display, meta=(MultiLine))
	FText Description = FText::GetEmpty();

	// ----------------------------------------------------------------------------------------------------------------
	//	Cooldowns
	// ----------------------------------------------------------------------------------------------------------------

	/** If true, the ability will start the cooldown the moment it gets activated. */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldowns", meta = (DisplayName = "Start with Cooldown"))
	uint8 bStartWithCooldown : 1;

	/** If true, the cooldown won't be reset when the owner dies. */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldowns", meta = (DisplayName = "Persist on Death"))
	uint8 bPersistCooldownOnDeath : 1;

	/** Explicit cooldown duration to use if no cooldown gameplay effect is given. */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldowns|Explicit")
	FScalableFloat ExplicitCooldownDuration;

	/** Explicit cooldown tags to apply when the explicit cooldown duration is used. */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldowns|Explicit")
	FGameplayTagContainer ExplicitCooldownTags;

	/** Explicit cooldown asset tags to apply when the explicit cooldown duration is used. */
	UPROPERTY(EditDefaultsOnly, Category = "Cooldowns|Explicit")
	FGameplayTagContainer ExplicitCooldownAssetTags;

	// ----------------------------------------------------------------------------------------------------------------
	//	Costs
	// ----------------------------------------------------------------------------------------------------------------

	/** Determines if costs are forcibly disabled for this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Costs)
	uint8 bApplyingCostsEnabled : 1;

	/** List of additional costs that must be paid to activate this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Costs, NoClear, meta=(ExcludeBaseStruct,ShowOnlyInnerProperties))
	TArray<TInstancedStruct<FModularAbilityCost>> AbilityCosts;

	// ----------------------------------------------------------------------------------------------------------------
	//	AI
	// ----------------------------------------------------------------------------------------------------------------

	/** If true, when AI activates the ability, it will stop the Behavior Logic until the ability is finished/aborted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AI, meta = (DisplayName = "Stops AI Behavior Logic"))
	uint8 bStopsAIBehaviorLogic : 1;

	/** If true, when AI activates the ability, it will stop the Movement until the ability is finished/aborted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AI, meta = (DisplayName = "Stops AI Movement"))
	uint8 bStopsAIMovement : 1;

	/** If true, when AI activates the ability, it will stop the RVO Avoidance until the ability is finished/aborted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AI, meta = (DisplayName = "Stops AI RVO Avoidance"))
	uint8 bStopsAIRVOAvoidance : 1;

	/** The range of the noise event that will be sent when the ability is activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AI, meta = (Units=cm))
	float ActivationNoiseRange;

	/** The loudness of the noise event that will be sent when the ability is activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = AI)
	float ActivationNoiseLoudness;
};
