// Author: Tom Werner (MajorT), 2025


#include "Tasks/AbilityTask_SpawnAndPossessPawn.h"

UAbilityTask_SpawnAndPossessPawn* UAbilityTask_SpawnAndPossessPawn::SpawnAndPossessPawn(
	UGameplayAbility* OwningAbility,
	FVector SpawnLocation,
	FRotator SpawnRotation,
	TSubclassOf<APawn> PawnClass)
{
	UAbilityTask_SpawnAndPossessPawn* NewTask = NewAbilityTask<UAbilityTask_SpawnAndPossessPawn>(OwningAbility);
	NewTask->CachedSpawnLocation = SpawnLocation;
	NewTask->CachedSpawnRotation = SpawnRotation;
	NewTask->ClassToSpawn = PawnClass;
	return NewTask;
}

void UAbilityTask_SpawnAndPossessPawn::Activate()
{
	if (Ability && Ability->GetCurrentActorInfo()->IsNetAuthority())
	{
		UWorld* const World = GEngine->GetWorldFromContextObject(Ability, EGetWorldErrorMode::LogAndReturnNull);

		if (IsValid(World))
		{
			APawn* NewPawn =
				World->SpawnActorDeferred<APawn>(ClassToSpawn, FTransform::Identity);

			if (!IsValid(NewPawn) && ShouldBroadcastAbilityTaskDelegates())
			{
				DidNotSpawn.Broadcast(nullptr);
				return;
			}

			FTransform SpawnTransform;
			SpawnTransform.SetLocation(CachedSpawnLocation);
			SpawnTransform.SetRotation(FQuat(CachedSpawnRotation));

			NewPawn->FinishSpawning(SpawnTransform);

			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnSpawned.Broadcast(NewPawn);
			}
		}
	}
}
