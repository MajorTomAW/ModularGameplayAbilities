// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueManager.h"
#include "ModularGameplayCueManager.generated.h"

/**
 * Extended version of the UGameplayCueManager
 */
UCLASS()
class MODULARGAMEPLAYABILITIES_API UModularGameplayCueManager : public UGameplayCueManager
{
	GENERATED_BODY()

public:
	UModularGameplayCueManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static UModularGameplayCueManager* Get();
	static void DumpGameplayCues();

	//~ Begin UGameplayCueManager Interface
	virtual void OnCreated() override;
	virtual bool ShouldAsyncLoadRuntimeObjectLibraries() const override;
	virtual bool ShouldSyncLoadMissingGameplayCues() const override;
	virtual bool ShouldAsyncLoadMissingGameplayCues() const override;
	//~ End UGameplayCueManager Interface

	/** When delay loading cues, this will load the cues that must be always loaded anyway. */
	void LoadAlwaysLoadedCues();

	/** Updates the bundles for the singular gameplay cue primary asset. */
	void RefreshGameplayCuePrimaryAsset();

private:
	void OnGameplayTagLoaded(const FGameplayTag& Tag);
	void HandlePostGarbageCollect();
	void ProcessLoadedTags();
	void ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject);
	void OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue);
	void RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject);
	void HandlePostLoadMap(UWorld* NewWorld);
	void UpdateDelayLoadDelegateListeners();
	bool ShouldDelayLoadGameplayCues() const;

private:
	struct FLoadedGameplayTagProcessData
	{
		FGameplayTag Tag;
		TWeakObjectPtr<UObject> WeakOwner;

		FLoadedGameplayTagProcessData() {}
		FLoadedGameplayTagProcessData(const FGameplayTag& InTag, UObject* InOwner)
			: Tag(InTag), WeakOwner(InOwner) {}
		FLoadedGameplayTagProcessData(const FGameplayTag& InTag, const TWeakObjectPtr<UObject>& InWeakOwner)
			: Tag(InTag), WeakOwner(InWeakOwner) {}
	};

private:
	/** Cues that were preloaded on the client due to being referenced by content. */
	UPROPERTY(Transient)
	TSet<TObjectPtr<UClass>> PreloadedCues;
	TMap<FObjectKey, TSet<FObjectKey>> PreloadedCueReferencers;
	
	/** Cues that were preloaded on the client and will always be loaded (code referenced or explicitly loaded) */
	UPROPERTY(Transient)
	TSet<TObjectPtr<UClass>> AlwaysLoadedCues;

	TArray<FLoadedGameplayTagProcessData> LoadedGameplayTagsToProcess;
	FCriticalSection LoadedGameplayTagsToProcessCS;
	bool bProcessLoadedTagsAfterGC = false;
};
