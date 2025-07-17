// Copyright © 2024 MajorT. All Rights Reserved.


#include "GlobalAbilityTargetingActor.h"

AGlobalAbilityTargetingActor::AGlobalAbilityTargetingActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject("Sprite"))
{
	NetPriority = 1.f;
	bNetLoadOnClient = true;
	SetNetUpdateFrequency(10.f);
	SetMinNetUpdateFrequency(1.f);
}
