// Copyright © 2024 Boring Games. All rights reserved.


#include "Abilities/TargetData/GameplayAbilityTargetActor_CursorTrace.h"

#include "Abilities/GameplayAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbilityTargetActor_CursorTrace)

AGameplayAbilityTargetActor_CursorTrace::AGameplayAbilityTargetActor_CursorTrace(
	const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{
	// The server can't do this as it doesn't know about the players cursor
	ShouldProduceTargetDataOnServer = false;
	bInheritRotation = true;
	CursorTraceType = TraceTypeQuery1;
	MaxRange = 10000.f;
}

void AGameplayAbilityTargetActor_CursorTrace::AimWithCursor(
	const AActor* InSourceActor,
	FCollisionQueryParams Params,
	const FVector& TraceStart,
	FVector& TraceEnd) const
{
	if (!OwningAbility)
	{
		return;
	}

	APlayerController* PC = OwningAbility->GetCurrentActorInfo()->PlayerController.Get();
	check(PC);

	FVector Direction;
	FHitResult CursorHit;
	bool bSuccess =
		bShouldUseCursorHitResult
		? PC->GetHitResultUnderCursorByChannel(CursorTraceType, false, CursorHit)
		: false;

	if (bSuccess && CursorHit.bBlockingHit)
	{
		Direction = (CursorHit.Location - TraceStart).GetSafeNormal();
	}
	else // Fail-safe, try find world dir by using the cursor screen position
	{
		// Gather cursor screen position
		float PosX, PosY;
		PC->GetMousePosition(PosX, PosY);

		// Convert to world position & direction
		FVector OutPos;
		bSuccess = PC->DeprojectScreenPositionToWorld(PosX, PosY, OutPos, Direction);

		// Normalize just in case
		Direction = Direction.GetSafeNormal();	
	}

	if (bSuccess)
	{
		TraceEnd = TraceStart + (Direction * MaxRange);
	}
}

void AGameplayAbilityTargetActor_CursorTrace::Tick(float DeltaSeconds)
{
	// Let's override the parent tick function to implement our own custom logic
	if (SourceActor && SourceActor->GetLocalRole() != ROLE_SimulatedProxy)
	{
		FHitResult HitResult = PerformTrace(SourceActor);
		FVector EndPoint = HitResult.Component.IsValid() ? HitResult.ImpactPoint : HitResult.TraceEnd;

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
			DrawDebugLine(GetWorld(), SourceActor->GetActorLocation(), EndPoint, FColor::Green, false);
			DrawDebugSphere(GetWorld(), EndPoint, 16, 10, FColor::Red, false);
		}
#endif

		// Whether we want to inherit the rotation we need to set the actor rotation
		if (bInheritRotation)
		{
			SetActorLocationAndRotation(EndPoint, SourceActor->GetActorRotation());
		}
		else
		{
			SetActorLocation(EndPoint);
		}
	}
}
