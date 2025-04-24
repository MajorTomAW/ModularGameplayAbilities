// Copyright © 2024 Boring Games. All rights reserved.


#include "Abilities/TargetData/GameplayAbilityTargetActor_CursorActorPlacement.h"

#include "Abilities/GameplayAbilityWorldReticle_ActorVisualization.h"
#include "Abilities/TargetData/Reticles/GameplayAbilityWorldReticle_GhostActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbilityTargetActor_CursorActorPlacement)

AGameplayAbilityTargetActor_CursorActorPlacement::AGameplayAbilityTargetActor_CursorActorPlacement(
	const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{
}

void AGameplayAbilityTargetActor_CursorActorPlacement::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	if (AActor* VisualizationActor = GetWorld()->SpawnActor(PlacedActorClass))
	{
		GhostActorReticle = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle_GhostActor>();
		GhostActorReticle->InitGhostActor(this, VisualizationActor, GhostMaterial);
		GetWorld()->DestroyActor(VisualizationActor);
	}

	if (AGameplayAbilityWorldReticle* CachedReticle = ReticleActor.Get())
	{
		GhostActorReticle->AttachToActor(CachedReticle, FAttachmentTransformRules::KeepRelativeTransform);
	}
	else
	{
		ReticleActor = GhostActorReticle;
		GhostActorReticle = nullptr;
	}
}

void AGameplayAbilityTargetActor_CursorActorPlacement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GhostActorReticle.IsValid())
	{
		GhostActorReticle.Get()->Destroy();
	}
	
	Super::EndPlay(EndPlayReason);
}
