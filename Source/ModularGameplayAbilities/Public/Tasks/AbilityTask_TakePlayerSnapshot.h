// Copyright © 2025 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_TakePlayerSnapshot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTakePlayerSnapshotDelegate);

/**
 * Takes a snapshot of the current player state (attributes, abilities, etc.) and stores it for later use.
 */
UCLASS()
class MODULARGAMEPLAYABILITIES_API UAbilityTask_TakePlayerSnapshot : public UAbilityTask
{
	GENERATED_BODY()

public:
	UAbilityTask_TakePlayerSnapshot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Manually call this if you want to restore the snapshot. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks")
	virtual void RestoreSnapshot();

	UFUNCTION(Blueprintable, Category="Ability|Tasks", meta=(HidePin=OwningAbility,DefaultToSelf=OwningAbility,BlueprintInternalUseOnly="true"), DisplayName="Take Player Snapshot")
	static UAbilityTask_TakePlayerSnapshot* TakePlayerSnapshot(UGameplayAbility* OwningAbility, const FGameplayTagQuery& AbilityQuery, bool bCacheAttributes = true, bool bAutoRestoreWhenEndedOrCanceled = true);

	/** Invoked if the player snapshot was restored. */
	UPROPERTY(BlueprintAssignable)
	FTakePlayerSnapshotDelegate OnSnapshotRestored;

protected:
	//~ Begin UGameplayTask Interface
	virtual void Activate() override;
	virtual void ExternalCancel() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	//~ End UGameplayTask Interface
	
private:
	bool bShouldCacheAbilities;		// Should we cache the player's abilities?
	bool bShouldCacheAttributes;	// Should we cache the player's attributes?
	
	FGameplayTagQuery AbilityQueryToRun;

	/** If true, will automatically restore the snapshot when the ability ends or is canceled. */
	bool bAutoRestoreOnEndOrCancel;

	UPROPERTY(Transient)
	TMap<TFieldPath<FProperty>, FGameplayAttributeData> CachedAttributes;

	UPROPERTY(Transient)
	TSet<TSoftClassPtr<UGameplayAbility>> CachedAbilities;
};
