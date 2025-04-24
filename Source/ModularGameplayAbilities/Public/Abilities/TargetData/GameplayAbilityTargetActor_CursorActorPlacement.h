// Copyright © 2024 Boring Games. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityTargetActor_CursorGroundTrace.h"

#include "GameplayAbilityTargetActor_CursorActorPlacement.generated.h"

class AGameplayAbilityWorldReticle_GhostActor;
class AGameplayAbilityWorldReticle_ActorVisualization;
class AActor;

/** Generic ability target actor that uses a cursor to place an actor in the world. */
UCLASS(Blueprintable, DisplayName="Cursor Actor Placement")
class MODULARGAMEPLAYABILITIES_API
	AGameplayAbilityTargetActor_CursorActorPlacement : public AGameplayAbilityTargetActor_CursorGroundTrace
{
	GENERATED_BODY()

public:
	AGameplayAbilityTargetActor_CursorActorPlacement(
		const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** The actor we want to place. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(ExposeOnSpawn=true), Category=Targeting)
	TObjectPtr<UClass> PlacedActorClass;

	/** The "Ghost" material used to visualize the actor placement. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(ExposeOnSpawn=true), Category=Targeting)
	TObjectPtr<UMaterialInterface> GhostMaterial;

protected:
	/** Visualization for the intended location of the placed actor. */
	TWeakObjectPtr<AGameplayAbilityWorldReticle_GhostActor> GhostActorReticle;
};
