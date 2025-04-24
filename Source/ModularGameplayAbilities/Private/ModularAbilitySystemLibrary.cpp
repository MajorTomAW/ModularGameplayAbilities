// Copyright © 2025 MajorT. All Rights Reserved.


#include "ModularAbilitySystemLibrary.h"

#include "Abilities/GameplayAbilityTargetTypes.h"

void UModularAbilitySystemLibrary::GetForwardFacingVectorFromActor(
	AActor* SourceActor,
	float ForwardDistance,
	float HorizontalAdjustment,
	float VerticalAdjustment,
	FVector& ForwardVector)
{
	// Calculate the normalized forward vector n units long
	FVector MovedForwardDir = ( SourceActor->GetActorForwardVector().GetSafeNormal() ) * ForwardDistance;

	// Adjust the forward vector by the horizontal and vertical adjustments
	MovedForwardDir.Y += HorizontalAdjustment;
	MovedForwardDir.Z += VerticalAdjustment;

	// Compute the final location
	ForwardVector = ( SourceActor->GetActorLocation() + MovedForwardDir );
}
