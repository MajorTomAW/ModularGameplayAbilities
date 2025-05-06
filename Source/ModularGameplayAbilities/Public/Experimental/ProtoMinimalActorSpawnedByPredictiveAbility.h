// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProtoMinimalActorSpawnedByPredictiveAbility.generated.h"

UCLASS()
class MODULARGAMEPLAYABILITIES_API AProtoMinimalActorSpawnedByPredictiveAbility : public AActor
{
	GENERATED_BODY()

public:
	AProtoMinimalActorSpawnedByPredictiveAbility();

	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;
};
