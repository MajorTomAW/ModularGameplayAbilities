// Author: Tom Werner (MajorT), 2025

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

public:
	/** Whether to inherit the owners rotation when aiming with the cursor. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ExposeOnSpawn=true), Category = Targeting)
	bool bInheritRotation;

	/** The trace type to use when gathering the cursor world location. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (ExposeOnSpawn=true), Category = Targeting)
	TEnumAsByte<ETraceTypeQuery> CursorTraceType;

	/** Whether to use the hit result from the cursor trace to determine the target location. */
	bool bShouldUseCursorHitResult = false;

protected:
	virtual void Tick(float DeltaSeconds) override;
};
