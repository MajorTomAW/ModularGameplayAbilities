// Copyright © 2024 Boring Games. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityTargetActor_CursorTrace.h"
#include "GameplayAbilityTargetData_CursorRadius.generated.h"

/** Selects everything within a given radius of the cursor hit. */
UCLASS(Blueprintable, NotPlaceable)
class MODULARGAMEPLAYABILITIES_API AGameplayAbilityTargetData_CursorRadius
	: public AGameplayAbilityTargetActor_CursorTrace
{
	GENERATED_BODY()

public:
	AGameplayAbilityTargetData_CursorRadius();

protected:
	//~ Begin AGameplayAbilityTargetActor Interface
	virtual void ConfirmTargetingAndContinue() override;
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;
	//~ End AGameplayAbilityTargetActor Interface

	/** Radius of target acquisition around the ability's start location. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Radius)
	float Radius;

protected:
	virtual TArray<TWeakObjectPtr<AActor>> PerformOverlap(const FVector& Origin);
	virtual FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<TWeakObjectPtr<AActor>>& Actors, const FVector& Origin) const;

	FHitResult LastHitResult;
};
