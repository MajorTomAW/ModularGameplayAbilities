// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_StartComboSequence.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FComboSequenceDelegate, int32, ComboCount, float, TimePassed);

/**
 * An ability task that starts a combo.
 */
UCLASS()
class MODULARGAMEPLAYABILITIES_API UAbilityTask_StartComboSequence : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_StartComboSequence(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(BlueprintAssignable)
	FComboSequenceDelegate OnComboIncreased;

	UPROPERTY(BlueprintAssignable)
	FComboSequenceDelegate OnComboFinished;

	UPROPERTY(BlueprintAssignable)
	FComboSequenceDelegate OnComboInterrupted;
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_StartComboSequence* StartComboSequence(UGameplayAbility* OwningAbility, int32 MaxComboCount, float MaxTimeBetweenCombos);

protected:
	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual FString GetDebugString() const override;

private:
	uint16 MaxComboCount;
	uint16 CurrentComboCount;
	double TimePassed;
	double MaxTimeBetweenCombos;
};
