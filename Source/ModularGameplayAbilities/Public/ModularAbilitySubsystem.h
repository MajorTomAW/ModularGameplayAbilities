// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "ModularAbilitySystemComponent.h"
#include "Subsystems/WorldSubsystem.h"

#include "ModularAbilitySubsystem.generated.h"

/** Struct containing all abilities that are applied to all actors. */
USTRUCT()
struct FGloballyAppliedAbilities
{
	GENERATED_BODY()

public:
	void AddToAbilitySystem(TSubclassOf<UGameplayAbility> Ability, UModularAbilitySystemComponent* AbilitySystem);
	void RemoveFromAbilitySystem(UModularAbilitySystemComponent* AbilitySystem);
	void RemoveFromAll();

public:
	UPROPERTY()
	TMap<TObjectPtr<UModularAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;
};

/** Struct containing all effects that are applied to all actors. */
USTRUCT()
struct FGloballyAppliedEffects
{
	GENERATED_BODY()

public:
	void AddToAbilitySystem(TSubclassOf<UGameplayEffect> Effect, UModularAbilitySystemComponent* AbilitySystem);
	void RemoveFromAbilitySystem(UModularAbilitySystemComponent* AbilitySystem);
	void RemoveFromAll();

public:
	UPROPERTY()
	TMap<TObjectPtr<UModularAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;
};

/**
 * Global subsystem for modular gameplay abilities.
 * Manages granting and removing abilities from actors.
 */
UCLASS(Config = Game)
class MODULARGAMEPLAYABILITIES_API UModularAbilitySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UModularAbilitySubsystem();

	//~ Begin UWorldSubsystem Interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~ End UWorldSubsystem Interface

	static UModularAbilitySubsystem* Get(const UObject* WorldContextObject);

	/**
	 * Registers an ability system component with the subsystem.
	 *
	 * @param AbilitySystem The ability system component to register.
	 * @param bGrantPendingAbilities If true, abilities that were added to all actors before this ability system was registered will be granted.
	 * @param bGrantPendingEffects If true, effects that were added to all actors before this ability system was registered will be granted.
	 */
	virtual void RegisterAbilitySystem(UModularAbilitySystemComponent* AbilitySystem, bool bGrantPendingAbilities = true, bool bGrantPendingEffects = true);

	/** Unregisters an ability system component with the subsystem. */
	virtual void UnregisterAbilitySystem(UModularAbilitySystemComponent* AbilitySystem);

	/** Applies a gameplay ability to all actors. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|Global")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	/** Applies a gameplay effect to all actors. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|Global")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	/** Removes a gameplay ability from all actors. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|Global")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	/** Removes a gameplay effect from all actors. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|Global")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

protected:
	/** Gets the list of all registered ability system components. */
	const TArray<TObjectPtr<UModularAbilitySystemComponent>>& GetRegisteredAbilitySystems() const { return RegisteredAbilitySystems; }

private:
	/** List of all globally applied abilities. */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGloballyAppliedAbilities> GloballyAppliedAbilities;

	/** List of all globally applied effects. */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGloballyAppliedEffects> GloballyAppliedEffects;
	
	/** List of all registered ability system components. */
	UPROPERTY()
	TArray<TObjectPtr<UModularAbilitySystemComponent>> RegisteredAbilitySystems;
};
