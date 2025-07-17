// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityTargetActor_CursorTrace.h"

#include "GameplayAbilityTargetActor_CursorGroundTrace.generated.h"

UCLASS(Blueprintable)
class MODULARGAMEPLAYABILITIES_API
	AGameplayAbilityTargetActor_CursorGroundTrace : public AGameplayAbilityTargetActor_CursorTrace
{
	GENERATED_BODY()

public:
	AGameplayAbilityTargetActor_CursorGroundTrace(
		const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void StartTargeting(UGameplayAbility* Ability) override;

public:
	/** The radius for a sphere or capsule. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	float CollisionRadius;

	/** The height for a capsule. Implicitly indicates a capsule is desired if this is greater than zero, */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	float CollisionHeight;

	/** When tracing, give this much extra height to avoid start-in-ground problems. Dealing with thick placement actors while standing near walls may be trickier. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Trace)
	float CollisionHeightOffset;

protected:
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;
	virtual bool IsConfirmTargetingAllowed() override;

protected:
	bool AdjustCollisionResultForShape(const FVector OriginalStartPoint, const FVector OriginalEndPoint, const FCollisionQueryParams Params, FHitResult& OutHitResult) const;

	FCollisionShape CollisionShape;
	bool bLastTraceWasGood;
};
