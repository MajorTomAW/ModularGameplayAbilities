// Copyright © 2024 Boring Games. All rights reserved.


#include "Abilities/TargetData/GameplayAbilityTargetActor_SingleCursorTrace.h"

#include "Camera/CameraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbilityTargetActor_SingleCursorTrace)

AGameplayAbilityTargetActor_SingleCursorTrace::AGameplayAbilityTargetActor_SingleCursorTrace(
	const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{
}

FHitResult AGameplayAbilityTargetActor_SingleCursorTrace::PerformTrace(AActor* InSourceActor)
{
	bool bTraceComplex = false;
	TArray ActorsToIgnore = { InSourceActor };

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGameplayAbilityTargetActor_CursorTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);
	
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();

	// Bruh, this is a bit of a hack, but we need to check if the source actor is a camera
	if (StartLocation.SourceSocketName == "_Camera")
	{
		if (UCameraComponent* Camera = InSourceActor->GetComponentByClass<UCameraComponent>())
		{
			TraceStart = Camera->GetComponentTransform().GetLocation();
		}
	}
	
	FVector TraceEnd;
	AimWithCursor(InSourceActor, Params, TraceStart, TraceEnd);


	// -------------------------------------------------------

	FHitResult ReturnHitResult;
	LineTraceWithFilter(
		ReturnHitResult,
		InSourceActor->GetWorld(),
		Filter,
		TraceStart,
		TraceEnd,
		TraceProfile.Name,
		Params);

	// Default to trace end if no hit
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
	}

	if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
	{
		const bool bHitActor = (ReturnHitResult.bBlockingHit && (ReturnHitResult.HitObjectHandle.IsValid()));
		const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) 
			? ReturnHitResult.HitObjectHandle.GetLocation()
			: ReturnHitResult.Location;

		LocalReticleActor->SetActorLocation(ReticleLocation);
		LocalReticleActor->SetIsTargetAnActor(bHitActor);
	}

#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Blue, false);
		DrawDebugSphere(GetWorld(), TraceEnd, 250.0f, 16, FColor::Red);
	}
#endif

	return ReturnHitResult;	
}
