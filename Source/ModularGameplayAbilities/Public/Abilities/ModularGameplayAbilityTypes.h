// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "Templates/SubclassOf.h"

#include "ModularGameplayAbilityTypes.generated.h"

class APlayerController;
class UGameplayAbility;
class UAbilitySystemComponent;

UENUM(BlueprintType)
namespace EGameplayAbilityActivationPolicy
{
	/**
	 * How the ability is meant to be activated. This can be used to gate abilities from being activated in certain ways.
	 * For example, passive abilities should be activated when given, others should be activated by the player, etc.
	 */
	enum Type : int
	{
		/** Abilities that always apply gameplay effects and/or tags. */
		Passive,

		/** Abilities that are activated by a trigger. */
		Triggered,

		/** Abilities that are activated by player actions. */
		Active,
	};
}

UENUM(BlueprintType)
namespace EGameplayAbilityActivationGroup
{
	/**
	 * Defines how an ability activates in relation to other abilities.
	 */
	enum Type : int
	{
		/** Abilities that run independently of each other. */
		Independent,

		/** Abilities that can be canceled and replaced by other exclusive abilities. */
		Exclusive_Replaceable		UMETA(DisplayName = "Exclusive (Replaceable)"),

		/** Abilities that block all other exclusive abilities from activating. */
		Exclusive_Blocking			UMETA(DisplayName = "Exclusive (Blocking)"),

		MAX							UMETA(Hidden),
	};
}

/** Tracking-info struct used to track actors that are being tracked by the ability system. */
USTRUCT(BlueprintType)
struct FAbilityTrackedActorEntry
{
	GENERATED_BODY()

	FAbilityTrackedActorEntry() = default;
	virtual ~FAbilityTrackedActorEntry() = default;

	/** The time the actor was started being tracked. */
	UPROPERTY(BlueprintReadWrite, Category=Tracking)
	float TrackedTime;

	/** The actor that is being tracked. */
	UPROPERTY(BlueprintReadWrite, Category=Tracking)
	TWeakObjectPtr<AActor> TrackedActor;
};