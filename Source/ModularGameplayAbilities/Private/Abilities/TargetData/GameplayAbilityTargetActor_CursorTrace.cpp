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
	bool bSuccess = PC->GetHitResultUnderCursor(ECC_Camera, false, CursorHit);

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
