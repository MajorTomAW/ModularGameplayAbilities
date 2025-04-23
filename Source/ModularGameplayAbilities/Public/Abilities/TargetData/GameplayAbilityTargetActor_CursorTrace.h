// Copyright © 2024 Boring Games. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor_Trace.h"

#include "GameplayAbilityTargetActor_CursorTrace.generated.h"

/** Base class for all line-trace type targeting actors that use the player's cursor to determine the target location. */
UCLASS(Abstract, MinimalAPI)
class AGameplayAbilityTargetActor_CursorTrace : public AGameplayAbilityTargetActor_Trace
{
	GENERATED_BODY()

public:
	AGameplayAbilityTargetActor_CursorTrace(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	

	MODULARGAMEPLAYABILITIES_API void AimWithCursor(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& TraceEnd) const;
};
