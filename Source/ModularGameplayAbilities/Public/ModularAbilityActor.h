// Copyright © 2024 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AttributeSet.h"
#include "GameFramework/Actor.h"

#include "ModularAbilityActor.generated.h"

class UModularAbilitySystemComponent;
struct FGameplayAttributeData;
struct FGameplayAttribute;

/**
 * Defines how an Ability System should be loaded (if ever) on the client.
 */
UENUM(BlueprintType)
namespace EModularAbilitySystemCreationPolicy
{
	enum Type : int
	{
		/** The Ability System will always be null on the client. */
		Never,

		/** The Ability System will be null on the client until it is used on the server. */
		Lazy,

		/** The Ability System is always loaded whenever the owning actor is loaded. */
		Always,
	};
}

/** A pending attribute replication */
struct FPendingAttributeReplication
{
public:
	FPendingAttributeReplication() = default;
	FPendingAttributeReplication(const FGameplayAttribute& InAttribute, const FGameplayAttributeData& InNewValue)
		: Attribute(InAttribute), NewValue(InNewValue) {}

	FGameplayAttribute Attribute;
	FGameplayAttributeData NewValue;
};

/**
 * A base class for all actors that are meant to use the Ability System.
 * It provides a way to lazy-load the Ability System for optimal performance.
 */
UCLASS(Blueprintable, BlueprintType, Abstract, meta = (ShortTooltip = "Base class for all Ability Actors that support lazy loading of Ability System."))
class MODULARGAMEPLAYABILITIES_API AModularAbilityActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AModularAbilityActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin AActor Interface
	virtual void PreInitializeComponents() override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
	//~ End AActor Interface

	//~ Begin IAbilitySystemInterface Interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface Interface

	/** Called to initialize the Ability System of this Actor. */
	void InitializeAbilitySystem();

	/** Called after the Ability System Component has been initialized. */
	virtual void OnAbilitySystemInitialized() {}

	/** Called to update any pending attributes that were set before the Ability System was initialized. */
	virtual void SetPendingAttributeFromReplication(const FGameplayAttribute& Attribute, const FGameplayAttributeData& NewValue);

	/** Called to apply any pending attribute replications that were set before the Ability System was initialized. */
	virtual void ApplyPendingAttributeReplications();

protected:
	/** The Ability System Component for this Actor. */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
	TObjectPtr<UModularAbilitySystemComponent> AbilitySystemComponent;

	/** The class of the Ability System Component to use for this Actor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TSubclassOf<UModularAbilitySystemComponent> AbilitySystemClass;

	/** The policy to use when creating the Ability System Component on the client. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TEnumAsByte<EModularAbilitySystemCreationPolicy::Type> CreationPolicy;

protected:
	/** Called to create the Ability System Component for this Actor. */
	virtual void CreateAbilitySystemComponent();

	UFUNCTION()
	virtual void OnRep_TransientAbilitySystem();

private:
	/** A transient Ability System Component that is used to store pending attribute replications. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TransientAbilitySystem)
	TObjectPtr<UModularAbilitySystemComponent> TransientAbilitySystem;

	/** Pending attribute replications */
	TArray<FPendingAttributeReplication> PendingAttributeReplications;
};
