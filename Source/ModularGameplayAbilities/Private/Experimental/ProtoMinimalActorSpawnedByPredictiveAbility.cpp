// Author: Tom Werner (MajorT), 2025


#include "Experimental/ProtoMinimalActorSpawnedByPredictiveAbility.h"


AProtoMinimalActorSpawnedByPredictiveAbility::AProtoMinimalActorSpawnedByPredictiveAbility()
{
	PrimaryActorTick.bCanEverTick = true;
	
	NetDormancy = DORM_DormantAll;
	bReplicates = true;
	SetNetUpdateFrequency(1.0f);
}

bool AProtoMinimalActorSpawnedByPredictiveAbility::IsNetRelevantFor(
	const AActor* RealViewer,
	const AActor* ViewTarget,
	const FVector& SrcLocation) const
{
	// The instigator who creates this shouldn't need to update it as it was predicted
	if (RealViewer == GetInstigator() ? GetInstigator()->GetController() : nullptr)
	{
		return false;
	}
	
	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}
