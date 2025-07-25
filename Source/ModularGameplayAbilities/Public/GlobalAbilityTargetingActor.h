﻿// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "GlobalAbilityTargetingActor.generated.h"

UCLASS(Blueprintable, BlueprintType, Abstract)
class MODULARGAMEPLAYABILITIES_API AGlobalAbilityTargetingActor
	: public AInfo
{
	GENERATED_BODY()

public:
	AGlobalAbilityTargetingActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
