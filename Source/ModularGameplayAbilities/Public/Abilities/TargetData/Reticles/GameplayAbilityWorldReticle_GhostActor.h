// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityWorldReticle.h"
#include "GameplayAbilityWorldReticle_GhostActor.generated.h"

/** Duplicate class of the ActorVisualization reticle because the original class is very annoying as it is not exposed to the API. */
UCLASS(NotPlaceable)
class MODULARGAMEPLAYABILITIES_API AGameplayAbilityWorldReticle_GhostActor
	: public AGameplayAbilityWorldReticle
{
	GENERATED_BODY()

public:
	AGameplayAbilityWorldReticle_GhostActor(
		const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitGhostActor(AGameplayAbilityTargetActor* InTargetActor, AActor* GhostActor, UMaterialInterface* GhostMaterial);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	/** Hardcoded collision component, so other objects don't think they can collide with this ghost actor. */
	UPROPERTY()
	TObjectPtr<class UCapsuleComponent> CollisionComponent;

	/** List of actor components that represent our ghost actor. */
	UPROPERTY()
	TArray<TObjectPtr<UActorComponent>> GhostComponents;
};
