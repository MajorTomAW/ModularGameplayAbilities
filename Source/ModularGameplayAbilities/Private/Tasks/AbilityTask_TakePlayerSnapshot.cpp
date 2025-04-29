// Copyright © 2025 MajorT. All Rights Reserved.


#include "Tasks/AbilityTask_TakePlayerSnapshot.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_TakePlayerSnapshot)

UAbilityTask_TakePlayerSnapshot::UAbilityTask_TakePlayerSnapshot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShouldCacheAbilities = true;
	bShouldCacheAttributes = true;
	bAutoRestoreOnEndOrCancel = true;
}

void UAbilityTask_TakePlayerSnapshot::RestoreSnapshot()
{
}

UAbilityTask_TakePlayerSnapshot* UAbilityTask_TakePlayerSnapshot::TakePlayerSnapshot(
	UGameplayAbility* OwningAbility,
	const FGameplayTagQuery& AbilityQuery,
	bool bCacheAttributes,
	bool bAutoRestoreWhenEndedOrCanceled)
{
	UAbilityTask_TakePlayerSnapshot* Task = NewAbilityTask<UAbilityTask_TakePlayerSnapshot>(OwningAbility);
	Task->bShouldCacheAbilities = true;
	Task->AbilityQueryToRun = AbilityQuery;
	Task->bShouldCacheAttributes = bCacheAttributes;
	Task->bAutoRestoreOnEndOrCancel = bAutoRestoreWhenEndedOrCanceled;

	return Task;
}

void UAbilityTask_TakePlayerSnapshot::Activate()
{
	Super::Activate();
}

void UAbilityTask_TakePlayerSnapshot::ExternalCancel()
{
	Super::ExternalCancel();
}

void UAbilityTask_TakePlayerSnapshot::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
}
