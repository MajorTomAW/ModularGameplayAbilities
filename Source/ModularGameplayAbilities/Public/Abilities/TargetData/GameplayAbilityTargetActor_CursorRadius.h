// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityTargetActor_SingleCursorTrace.h"

#include "GameplayAbilityTargetActor_CursorRadius.generated.h"

/** Selects everything within a given radius of the cursor hit. */
UCLASS(Blueprintable, NotPlaceable)
class MODULARGAMEPLAYABILITIES_API AGameplayAbilityTargetActor_CursorRadius
	: public AGameplayAbilityTargetActor_SingleCursorTrace
{
	GENERATED_BODY()

public:
	AGameplayAbilityTargetActor_CursorRadius();

protected:
	//~ Begin AGameplayAbilityTargetActor Interface
	virtual void ConfirmTargetingAndContinue() override;
	//~ End AGameplayAbilityTargetActor Interface

	/** Radius of target acquisition around the ability's start location. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Radius)
	float Radius;

protected:
	virtual TArray<TWeakObjectPtr<AActor>> PerformOverlap(const FVector& Origin);
	virtual FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<TWeakObjectPtr<AActor>>& Actors, const FVector& Origin) const;

	FHitResult LastHitResult;
};
