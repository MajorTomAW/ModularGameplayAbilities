// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"

#include "AbilityTask_SpawnAndPossessPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnAndPossessPawnDelegate, APawn*, SpawnedPawn);

/** Spawns a pawn and possesses it. */
UCLASS()
class MODULARGAMEPLAYABILITIES_API UAbilityTask_SpawnAndPossessPawn : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FSpawnAndPossessPawnDelegate OnSpawned;

	UPROPERTY(BlueprintAssignable)
	FSpawnAndPossessPawnDelegate DidNotSpawn;

	/** Spawn the pawn on network authority (server) */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UAbilityTask_SpawnAndPossessPawn* SpawnAndPossessPawn(UGameplayAbility* OwningAbility, FVector SpawnLocation, FRotator SpawnRotation, TSubclassOf<APawn> PawnClass);

	virtual void Activate() override;

protected:
	FVector CachedSpawnLocation;
	FRotator CachedSpawnRotation;

	UPROPERTY()
	TObjectPtr<UClass> ClassToSpawn;
};
