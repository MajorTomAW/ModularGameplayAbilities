// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"

#include "ModularAbilitySet.generated.h"

class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UAbilitySystemComponent;

/**
 * Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct FModularAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	/** Gameplay ability class to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Ability")
	TSoftClassPtr<UGameplayAbility> AbilityClass = nullptr;

	/** Level of ability to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Ability", meta = (ClampMin = 1, UIMin = 1))
	int32 AbilityLevel = 1;

	/** Tag used to process input for this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Ability", meta = (Categories = "Input.InputTag"))
	FGameplayTag InputTag;
};

/** Data struct that can be used to enable input for a gameplay ability. */
USTRUCT(BlueprintType)
struct FAbilityActivatedByInputData
{
	GENERATED_BODY()

public:
	/** The gameplay ability class that is meant to receive input. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Ability")
	TSoftClassPtr<UGameplayAbility> AbilityClass = nullptr;

	/** The tag query to determine if the ability should be activated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Ability")
	FGameplayTagQuery ActivationTagQuery;
};

/**
 * Data used by the ability set to grant gameplay effects.
 */
USTRUCT(BlueprintType)
struct FModularAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:
	/** Gameplay effect class to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Effect")
	TSubclassOf<UGameplayEffect> EffectClass = nullptr;

	/** Level of effect to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Effect", meta = (ClampMin = 1, UIMin = 1))
	int32 EffectLevel = 1;
};

/** Data used by the ability set to grant attributes. */
USTRUCT(BlueprintType)
struct FModularAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	/** Attribute set class to grant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute Set")
	TSubclassOf<UAttributeSet> AttributeSetClass = nullptr;
};

/**
 * Data used to store granted handles for the ability set, such as ability handles, effect handles, or attribute sets.
 */
USTRUCT(BlueprintType)
struct FModularAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	FModularAbilitySet_GrantedHandles()
	{
	}
	virtual ~FModularAbilitySet_GrantedHandles() = default;

	/** Adds a new ability spec handle to the granted handles. */
	MODULARGAMEPLAYABILITIES_API void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& InHandle);

	/** Appends an array of ability spec handles to the granted handles. */
	MODULARGAMEPLAYABILITIES_API void AppendAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InHandles);

	/** Adds a new gameplay effect handle to the granted handles. */
	MODULARGAMEPLAYABILITIES_API void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& InHandle);

	/** Appends an array of gameplay effect handles to the granted handles. */
	MODULARGAMEPLAYABILITIES_API void AppendGameplayEffectHandles(const TArray<FActiveGameplayEffectHandle>& InHandles);

	/** Adds a new attribute set to the granted handles. */
	MODULARGAMEPLAYABILITIES_API void AddAttributeSet(UAttributeSet* InAttributeSet);

	/** Appends an array of attribute sets to the granted handles. */
	MODULARGAMEPLAYABILITIES_API void AppendAttributeSets(const TArray<UAttributeSet*>& InAttributeSets);

	/** Appends the granted handles from another granted handles object. */
	MODULARGAMEPLAYABILITIES_API void AppendHandles(const FModularAbilitySet_GrantedHandles& InGrantedHandles);

	/** Removes all granted handles. */
	MODULARGAMEPLAYABILITIES_API void TakeFromAbilitySystem(UAbilitySystemComponent* AbilitySystem);

	/** Returns all granted ability spec handles. */
	MODULARGAMEPLAYABILITIES_API const TArray<FGameplayAbilitySpecHandle>& GetAbilitySpecHandles() const { return GrantedAbilityHandles; }
	MODULARGAMEPLAYABILITIES_API TArray<FGameplayAbilitySpecHandle>& GetAbilitySpecHandles() { return GrantedAbilityHandles; }

	/** Returns all granted gameplay effect handles. */
	MODULARGAMEPLAYABILITIES_API const TArray<FActiveGameplayEffectHandle>& GetGameplayEffectHandles() const { return GrantedEffectHandles; }
	MODULARGAMEPLAYABILITIES_API TArray<FActiveGameplayEffectHandle>& GetGameplayEffectHandles() { return GrantedEffectHandles; }

	/** Returns all granted attribute sets. */
	MODULARGAMEPLAYABILITIES_API const TArray<UAttributeSet*>& GetAttributeSets() const { return GrantedAttributeSets; }
	MODULARGAMEPLAYABILITIES_API TArray<TObjectPtr<UAttributeSet>>& GetAttributeSets() { return GrantedAttributeSets; }

protected:
	/** Handles to granted abilities. */
	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	/** Handles to granted effects. */
	UPROPERTY(Transient)
	TArray<FActiveGameplayEffectHandle> GrantedEffectHandles;

	/** Handles to granted attribute sets. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};

/** Non-mutable set of abilities that can be granted or removed to an actor that has an ability system component. */
UCLASS(BlueprintType, Const)
class MODULARGAMEPLAYABILITIES_API UModularAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UModularAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Grants the abilities, effects, and attribute sets to the given ability system component. */
	virtual void GiveToAbilitySystem(UAbilitySystemComponent* AbilitySystem, FModularAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;
	virtual void GiveToAbilitySystem(UAbilitySystemComponent* AbilitySystem, UObject* SourceObject = nullptr) const;
	virtual void GiveToAbilitySystem(UAbilitySystemComponent* AbilitySystem, TArray<FGameplayAbilitySpecHandle>* OutAbilityHandles, TArray<FActiveGameplayEffectHandle>* OutEffectHandles, TArray<TObjectPtr<UAttributeSet>>* OutAttributeSets, UObject* SourceObject = nullptr) const;

protected:
	/** Abilities to grant when this set is given. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AbilitySystem, meta = (TitleProperty=AbilityClass))
	TArray<FModularAbilitySet_GameplayAbility> Abilities;

	/** Abilities to grant when this set is given. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = AbilitySystem)
	TArray<TSoftClassPtr<UGameplayAbility>> GameplayAbilities;

	/** Effects to grant when this set is given. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AbilitySystem, meta = (TitleProperty=EffectClass))
	TArray<FModularAbilitySet_GameplayEffect> GameplayEffects;

	/** List of gameplay abilities that are meant to be activated by input. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Input)
	TArray<FAbilityActivatedByInputData> InputActivatedAbilities;

	/** Attribute sets to grant when this set is given. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Attributes, meta = (TitleProperty=AttributeSetClass))
	TArray<FModularAbilitySet_AttributeSet> AttributeSets;

protected:
#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	//~ End UObject Interface
#endif
};
