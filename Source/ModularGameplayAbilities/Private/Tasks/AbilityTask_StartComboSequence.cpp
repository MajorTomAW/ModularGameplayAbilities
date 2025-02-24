// Copyright © 2025 Playton. All Rights Reserved.


#include "Tasks/AbilityTask_StartComboSequence.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_StartComboSequence)

UAbilityTask_StartComboSequence::UAbilityTask_StartComboSequence(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxComboCount = 0;
	CurrentComboCount = 0;
	TimePassed = 0.0;
	MaxTimeBetweenCombos = 0.0;
}

UAbilityTask_StartComboSequence* UAbilityTask_StartComboSequence::StartComboSequence(UGameplayAbility* OwningAbility, int32 MaxComboCount, float MaxTimeBetweenCombos)
{
	UAbilityTask_StartComboSequence* Task = NewAbilityTask<UAbilityTask_StartComboSequence>(OwningAbility);
	Task->MaxComboCount = MaxComboCount;
	Task->MaxTimeBetweenCombos = MaxTimeBetweenCombos;
	return Task;
}

void UAbilityTask_StartComboSequence::Activate()
{
	Super::Activate();
}

void UAbilityTask_StartComboSequence::ExternalCancel()
{
	Super::ExternalCancel();
}

FString UAbilityTask_StartComboSequence::GetDebugString() const
{
	return Super::GetDebugString();
}
