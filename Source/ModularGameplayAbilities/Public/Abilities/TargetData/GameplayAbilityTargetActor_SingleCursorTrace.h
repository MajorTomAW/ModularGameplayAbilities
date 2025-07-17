// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilityTargetActor_CursorTrace.h"
#include "GameplayAbilityTargetActor_SingleCursorTrace.generated.h"

UCLASS()
class MODULARGAMEPLAYABILITIES_API
	AGameplayAbilityTargetActor_SingleCursorTrace
	: public AGameplayAbilityTargetActor_CursorTrace
{
	GENERATED_BODY()

public:
	AGameplayAbilityTargetActor_SingleCursorTrace(
		const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin AGameplayAbilityTargetActor_Trace Interface
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;
	//~ End AGameplayAbilityTargetActor_Trace Interface
};
