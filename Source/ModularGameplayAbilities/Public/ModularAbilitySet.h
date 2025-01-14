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

/**
 * Data used by the ability set to grant attributes.
 */
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
	
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& InHandle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& InHandle);
	void AddAttributeSet(UAttributeSet* InAttributeSet);
	void TakeFromAbilitySystem(UAbilitySystemComponent* AbilitySystem);

	const TArray<FGameplayAbilitySpecHandle>& GetAbilitySpecHandles() const { return GrantedAbilityHandles; }
	TArray<FGameplayAbilitySpecHandle>& GetAbilitySpecHandles() { return GrantedAbilityHandles; }
	const TArray<FActiveGameplayEffectHandle>& GetGameplayEffectHandles() const { return GrantedEffectHandles; }
	TArray<FActiveGameplayEffectHandle>& GetGameplayEffectHandles() { return GrantedEffectHandles; }
	const TArray<UAttributeSet*>& GetAttributeSets() const { return GrantedAttributeSets; }
	TArray<TObjectPtr<UAttributeSet>>& GetAttributeSets() { return GrantedAttributeSets; }

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

/**
 * Non-mutable set of abilities that can be granted or removed to an actor that has an ability system component.
 */
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<FModularAbilitySet_GameplayAbility> Abilities;

	/** Effects to grant when this set is given. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<FModularAbilitySet_GameplayEffect> Effects;

	/** Attribute sets to grant when this set is given. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TArray<FModularAbilitySet_AttributeSet> AttributeSets;

protected:
#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	//~ End UObject Interface
#endif
};
