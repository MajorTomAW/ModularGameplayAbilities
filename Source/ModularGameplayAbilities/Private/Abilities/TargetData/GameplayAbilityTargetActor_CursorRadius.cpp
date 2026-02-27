// Author: Tom Werner (MajorT), 2025


#include "Abilities/TargetData/GameplayAbilityTargetActor_CursorRadius.h"

#include "AbilitySystemLog.h"
#include "Camera/CameraComponent.h"
#include "Engine/OverlapResult.h"


AGameplayAbilityTargetActor_CursorRadius::AGameplayAbilityTargetActor_CursorRadius()
{
	ShouldProduceTargetDataOnServer = true;
	Radius = 0.f;
}


void AGameplayAbilityTargetActor_CursorRadius::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (SourceActor)
	{
		FVector Origin = LastHitResult.ImpactPoint;
		FGameplayAbilityTargetDataHandle Handle = MakeTargetData(PerformOverlap(Origin), Origin);
		TargetDataReadyDelegate.Broadcast(Handle);
	}
}

/*FHitResult AGameplayAbilityTargetActor_CursorRadius::PerformTrace(AActor* InSourceActor)
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

	// -------------------------------------------------

	FHitResult ReturnHitResult;
	//Use a line trace initially to see where the player is actually pointing
	LineTraceWithFilter(ReturnHitResult, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);
	//Default to end of trace line if we don't hit anything.
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
	}

	LastHitResult = ReturnHitResult;

	if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
	{
		LocalReticleActor->SetActorLocation(ReturnHitResult.Location);
	}

	return ReturnHitResult;
}*/

TArray<TWeakObjectPtr<AActor>> AGameplayAbilityTargetActor_CursorRadius::PerformOverlap(const FVector& Origin)
{
	bool bTraceComplex = false;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RadiusTargetingOverlap), bTraceComplex);
	Params.bReturnPhysicalMaterial = false;

	TArray<FOverlapResult> Overlaps;

	SourceActor->GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(Radius), Params);

	TArray<TWeakObjectPtr<AActor>>	HitActors;

	for (int32 i = 0; i < Overlaps.Num(); ++i)
	{
		// Should this check to see if these pawns are in the AimTarget list?
		APawn* PawnActor = Overlaps[i].OverlapObjectHandle.FetchActor<APawn>();
		if (PawnActor && !HitActors.Contains(PawnActor) && Filter.FilterPassesForActor(PawnActor))
		{
			HitActors.Add(PawnActor);
		}
	}

	if (bDebug)
	{
		const FColor& DebugColor = HitActors.Num() == 0 ? FColor::Red : FColor::Green;
		UE_VLOG_SPHERE(SourceActor, LogAbilitySystem, Log, Origin, Radius, DebugColor, TEXT("TargetActor Radius"));

#if UE_ENABLE_DEBUG_DRAWING
		constexpr bool bPersistent = false;
		constexpr float LifeTime = 2.0f;
		DrawDebugSphere(GetWorld(), Origin, Radius, 16, DebugColor, bPersistent, LifeTime);
#endif // UE_ENABLE_DEBUG_DRAWING
	}

	return HitActors;
}

FGameplayAbilityTargetDataHandle AGameplayAbilityTargetActor_CursorRadius::MakeTargetData(
	const TArray<TWeakObjectPtr<AActor>>& Actors,
	const FVector& Origin) const
{
	if (OwningAbility)
	{
		// Use the source location instead of the literal origin
		return StartLocation.MakeTargetDataHandleFromActors(Actors, false);
	}

	return FGameplayAbilityTargetDataHandle();
}
