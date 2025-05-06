// Author: Tom Werner (MajorT), 2025


#include "Experimental/ProtoMinimalActorSpawnedByPredictiveAbility.h"


AProtoMinimalActorSpawnedByPredictiveAbility::AProtoMinimalActorSpawnedByPredictiveAbility()
{
	PrimaryActorTick.bCanEverTick = true;
	
	NetDormancy = DORM_DormantAll;
	bReplicates = true;

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
	SetNetUpdateFrequency(1.0f);
#else
	NetUpdateFrequency = 1.0f;
#endif
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
