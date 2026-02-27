// Author: Tom Werner (MajorT), 2025


#include "ModularGameplayCueManager.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "GameplayCueSet.h"
#include "GameplayTagsManager.h"
#include "Engine/AssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularGameplayCueManager)


//////////////////////////////////////////////////////////////////////////
/// Editor Load Mode

enum class EModularGameplayCueEditorLoadMode
{
	/** Loads all cues upfront. Longer loading speed in the editor but short PIE times and effects never fail to play. */
	LoadUpfront,

	/**
	 * Outside the editor:		Async loads as cue tags are registered.
	 * Inside the editor:		Async loads when cues are invoked.
	 * @note This can cause some 'why didn't I see the effect for X' issue in PIE and is good for iteration speed but otherwise bad for designers.
	 */
	PreloadAsCuesAreReferenced_GameOnly,

	/** Async loads as cue tags are registered. */
	PreloadAsCuesAreReferenced,
};

namespace ModularGameplayCueManagerCVars
{
	static FAutoConsoleCommand CVarDumpGameplayCues
	(
		TEXT("AbilitySystem.DumpGameplayCues"),
		TEXT("Shows all the assets that were loaded via the ModularGameplayCueManager and are currently in memory. (Args: Refs)"),
		FConsoleCommandDelegate::CreateStatic(UModularGameplayCueManager::DumpGameplayCues)
	);

	static EModularGameplayCueEditorLoadMode LoadMode = EModularGameplayCueEditorLoadMode::LoadUpfront;
}

const bool bPreloadEvenInEditor = true;

//////////////////////////////////////////////////////////////////////////
/// Async Loading

struct FGameplayCueTagThreadSynchronizeGraphTask : public FAsyncGraphTaskBase
{
	TFunction<void()> TheTask;
	FGameplayCueTagThreadSynchronizeGraphTask(TFunction<void()>&& Task) : TheTask(MoveTemp((Task))) {}
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) const
	{
		TheTask();
	}
	ENamedThreads::Type GetDesiredThread()
	{
		return ENamedThreads::GameThread;
	}
};

//////////////////////////////////////////////////////////////////////////
/// UModularGameplayCueManager

UModularGameplayCueManager::UModularGameplayCueManager(const FObjectInitializer& ObjectInitializer)
	: UGameplayCueManager(ObjectInitializer)
{
}

UModularGameplayCueManager* UModularGameplayCueManager::Get()
{
	return Cast<UModularGameplayCueManager>(UAbilitySystemGlobals::Get().GetGameplayCueManager());
}

void UModularGameplayCueManager::DumpGameplayCues()
{
	TArray<FString> Args;
	UModularGameplayCueManager* GCM = Get();
	if (!GCM)
	{
		ABILITY_LOG(Error, TEXT("UModularGameplayCueManager::DumpGameplayCues: No ModularGameplayCueManager found."));
		return;
	}

	const bool bIncludeRefs = Args.Contains(TEXT("Refs"));

	ABILITY_LOG(Log, TEXT("=========== Dumping Always Loaded Gameplay Cue Notifies ==========="));
	for (const UClass* CueClass : GCM->AlwaysLoadedCues)
	{
		ABILITY_LOG(Log, TEXT("		%s"), *GetPathNameSafe(CueClass));
	}

	ABILITY_LOG(Log, TEXT("=========== Dumping Preloaded Gameplay Cue Notifies ==========="));
	for (const UClass* CueClass : GCM->PreloadedCues)
	{
		TSet<FObjectKey>* ReferencerSet = GCM->PreloadedCueReferencers.Find(CueClass);
		int32 NumRefs = ReferencerSet ? ReferencerSet->Num() : 0;
		ABILITY_LOG(Log, TEXT("		%s (%d refs)"), *GetPathNameSafe(CueClass), NumRefs);
		if (bIncludeRefs && ReferencerSet)
		{
			for (const FObjectKey& Ref : *ReferencerSet)
			{
				UObject* RefObject = Ref.ResolveObjectPtr();
				ABILITY_LOG(Log, TEXT("		^- %s"), *GetPathNameSafe(RefObject));
			}
		}
	}

	ABILITY_LOG(Log, TEXT("=========== Dumping Gameplay Cue Notifies loaded on demand ==========="));
	int32 NumMissingCuesLoaded = 0;
	if (GCM->RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (const auto& CueData : GCM->RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData)
		{
			if (CueData.LoadedGameplayCueClass &&
				!GCM->AlwaysLoadedCues.Contains(CueData.LoadedGameplayCueClass) &&
				!GCM->PreloadedCues.Contains(CueData.LoadedGameplayCueClass))
			{
				NumMissingCuesLoaded++;
				ABILITY_LOG(Log, TEXT("		%s"), *GetPathNameSafe(CueData.LoadedGameplayCueClass));
			}
		}
	}

	ABILITY_LOG(Log, TEXT("=========== Gameplay Cue Notify Summary ==========="));
	ABILITY_LOG(Log, TEXT("	... %d cues in always loaded list."), GCM->AlwaysLoadedCues.Num());
	ABILITY_LOG(Log, TEXT("	... %d cues in preloaded list."), GCM->PreloadedCues.Num());
	ABILITY_LOG(Log, TEXT("	... %d cues loaded on demand."), NumMissingCuesLoaded);
	ABILITY_LOG(Log, TEXT("	... %d cues in total."), GCM->AlwaysLoadedCues.Num() + GCM->PreloadedCues.Num() + NumMissingCuesLoaded);
}

void UModularGameplayCueManager::OnCreated()
{
	Super::OnCreated();

	UpdateDelayLoadDelegateListeners();
}

bool UModularGameplayCueManager::ShouldAsyncLoadRuntimeObjectLibraries() const
{
	switch (ModularGameplayCueManagerCVars::LoadMode) {
	case EModularGameplayCueEditorLoadMode::LoadUpfront:
		return true;
	case EModularGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return false;
		}
#endif
		break;
	case EModularGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	return !ShouldDelayLoadGameplayCues();
}

bool UModularGameplayCueManager::ShouldSyncLoadMissingGameplayCues() const
{
	return false;
}

bool UModularGameplayCueManager::ShouldAsyncLoadMissingGameplayCues() const
{
	return true;
}

void UModularGameplayCueManager::LoadAlwaysLoadedCues()
{
	if (ShouldDelayLoadGameplayCues())
	{
		UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();

		//@TODO: Try to collect these by filtering GameplayCue. tags out of native gameplay tags?
		TArray<FName> AdditionalAlwaysLoadedCueTags;

		for (const FName& CueTagName : AdditionalAlwaysLoadedCueTags)
		{
			FGameplayTag CueTag = TagManager.RequestGameplayTag(CueTagName, /*ErrorIfNotFound=*/ false);
			if (CueTag.IsValid())
			{
				ProcessTagToPreload(CueTag, nullptr);
			}
			else
			{
				ABILITY_LOG(Warning, TEXT("UModularGameplayCueManager::AdditionalAlwaysLoadedCueTags contains invalid tag %s"), *CueTagName.ToString());
			}
		}
	}
}

void UModularGameplayCueManager::UpdateDelayLoadDelegateListeners()
{
	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.RemoveAll(this);
	FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

	switch (ModularGameplayCueManagerCVars::LoadMode) {
	case EModularGameplayCueEditorLoadMode::LoadUpfront:
		return;
	case EModularGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return;
		}
#endif
	case EModularGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	UGameplayTagsManager::Get().OnGameplayTagLoadedDelegate.AddUObject(this, &ThisClass::OnGameplayTagLoaded);
	FCoreUObjectDelegates::GetPostGarbageCollect().AddUObject(this, &ThisClass::HandlePostGarbageCollect);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);
}

bool UModularGameplayCueManager::ShouldDelayLoadGameplayCues() const
{
	const bool bClientDelayLoadGameplayCues = true;
	return !IsRunningDedicatedServer() && bClientDelayLoadGameplayCues;
}

void UModularGameplayCueManager::OnGameplayTagLoaded(const FGameplayTag& Tag)
{
	FScopeLock ScopeLock(&LoadedGameplayTagsToProcessCS);
	bool bStartTask = LoadedGameplayTagsToProcess.Num() == 0;
	FUObjectSerializeContext* LoadContext = FUObjectThreadContext::Get().GetSerializeContext();
	UObject* OwningObject = LoadContext ? LoadContext->SerializedObject : nullptr;
	LoadedGameplayTagsToProcess.Emplace(Tag, OwningObject);
	if (bStartTask)
	{
		TGraphTask<FGameplayCueTagThreadSynchronizeGraphTask>::CreateTask().ConstructAndDispatchWhenReady([]()
		{
			if (GIsRunning)
			{
				if (UModularGameplayCueManager* StrongThis = Get())
				{
					if (IsGarbageCollecting())
					{
						StrongThis->bProcessLoadedTagsAfterGC = true;
					}
					else
					{
						StrongThis->ProcessLoadedTags();
					}
				}
			}
		});
	}
}

void UModularGameplayCueManager::HandlePostGarbageCollect()
{
	if (bProcessLoadedTagsAfterGC)
	{
		ProcessLoadedTags();
	}
	bProcessLoadedTagsAfterGC = false;
}

void UModularGameplayCueManager::ProcessLoadedTags()
{
	TArray<FLoadedGameplayTagProcessData> TaskLoadedGameplayTagsToProcess;
	{
		FScopeLock TaskScopeLock(&LoadedGameplayTagsToProcessCS);
		TaskLoadedGameplayTagsToProcess = LoadedGameplayTagsToProcess;
		LoadedGameplayTagsToProcess.Empty();
	}

	if (GIsRunning)
	{
		if (RuntimeGameplayCueObjectLibrary.CueSet)
		{
			for (const auto& Loaded : TaskLoadedGameplayTagsToProcess)
			{
				if (RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Contains(Loaded.Tag))
				{
					if (!Loaded.WeakOwner.IsStale())
					{
						ProcessTagToPreload(Loaded.Tag, Loaded.WeakOwner.Get());
					}
				}
			}
		}
		else
		{
			ABILITY_LOG(Warning, TEXT("UModularGameplayCueManager::ProcessLoadedTags: RuntimeGameplayCueObjectLibrary.CueSet is null."));
		}
	}
}

void UModularGameplayCueManager::ProcessTagToPreload(const FGameplayTag& Tag, UObject* OwningObject)
{
	switch (ModularGameplayCueManagerCVars::LoadMode)
	{
	case EModularGameplayCueEditorLoadMode::LoadUpfront:
		return;
	case EModularGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced_GameOnly:
#if WITH_EDITOR
		if (GIsEditor)
		{
			return;
		}
#endif
		break;
	case EModularGameplayCueEditorLoadMode::PreloadAsCuesAreReferenced:
		break;
	}

	check(RuntimeGameplayCueObjectLibrary.CueSet);

	int32* DataIdx = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueDataMap.Find(Tag);
	if (DataIdx && RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData.IsValidIndex(*DataIdx))
	{
		const FGameplayCueNotifyData& CueData = RuntimeGameplayCueObjectLibrary.CueSet->GameplayCueData[*DataIdx];

		UClass* LoadedGameplayCueClass = FindObject<UClass>(nullptr, *CueData.GameplayCueNotifyObj.ToString());
		if (LoadedGameplayCueClass)
		{
			RegisterPreloadedCue(LoadedGameplayCueClass, OwningObject);
		}
		else
		{
			bool bAlwaysLoadedCue = OwningObject == nullptr;
			TWeakObjectPtr<UObject> WeakOwner = OwningObject;
			StreamableManager.RequestAsyncLoad(CueData.GameplayCueNotifyObj, FStreamableDelegate::CreateUObject(this, &ThisClass::OnPreloadCueComplete, CueData.GameplayCueNotifyObj, WeakOwner, bAlwaysLoadedCue), FStreamableManager::DefaultAsyncLoadPriority, false, false, TEXT("GameplayCueManager"));
		}
	}
}

void UModularGameplayCueManager::OnPreloadCueComplete(FSoftObjectPath Path, TWeakObjectPtr<UObject> OwningObject, bool bAlwaysLoadedCue)
{
	if (bAlwaysLoadedCue || OwningObject.IsValid())
	{
		if (UClass* LoadedCueClass = Cast<UClass>(Path.ResolveObject()))
		{
			RegisterPreloadedCue(LoadedCueClass, OwningObject.Get());
		}
	}
}

void UModularGameplayCueManager::RegisterPreloadedCue(UClass* LoadedGameplayCueClass, UObject* OwningObject)
{
	check(LoadedGameplayCueClass);

	const bool bAlwaysLoadedCue = OwningObject == nullptr;
	if (bAlwaysLoadedCue)
	{
		AlwaysLoadedCues.Add(LoadedGameplayCueClass);
		PreloadedCues.Remove(LoadedGameplayCueClass);
		PreloadedCueReferencers.Remove(LoadedGameplayCueClass);
	}
	else if ((OwningObject != LoadedGameplayCueClass) && (OwningObject != LoadedGameplayCueClass->GetDefaultObject()) && !AlwaysLoadedCues.Contains(LoadedGameplayCueClass))
	{
		PreloadedCues.Add(LoadedGameplayCueClass);
		TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindOrAdd(LoadedGameplayCueClass);
		ReferencerSet.Add(OwningObject);
	}
}

void UModularGameplayCueManager::HandlePostLoadMap(UWorld* NewWorld)
{
	if (RuntimeGameplayCueObjectLibrary.CueSet)
	{
		for (UClass* CueClass : AlwaysLoadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}

		for (UClass* CueClass : PreloadedCues)
		{
			RuntimeGameplayCueObjectLibrary.CueSet->RemoveLoadedClass(CueClass);
		}
	}

	for (auto CueIt = PreloadedCues.CreateIterator(); CueIt; ++CueIt)
	{
		TSet<FObjectKey>& ReferencerSet = PreloadedCueReferencers.FindChecked(*CueIt);
		for (auto RefIt = ReferencerSet.CreateIterator(); RefIt; ++RefIt)
		{
			if (!RefIt->ResolveObjectPtr())
			{
				RefIt.RemoveCurrent();
			}
		}
		if (ReferencerSet.Num() == 0)
		{
			PreloadedCueReferencers.Remove(*CueIt);
			CueIt.RemoveCurrent();
		}
	}
}

const FPrimaryAssetType UAssetManager_GameplayCueRefsType = TEXT("GameplayCueRefs");
const FName UAssetManager_GameplayCueRefsName = TEXT("GameplayCueReferences");
const FName UAssetManager_LoadStateClient = TEXT("Client");


void UModularGameplayCueManager::RefreshGameplayCuePrimaryAsset()
{
	TArray<FSoftObjectPath> CuePaths;
	UGameplayCueSet* RuntimeGameplayCueSet = GetRuntimeCueSet();
	if (RuntimeGameplayCueSet)
	{
		RuntimeGameplayCueSet->GetSoftObjectPaths(CuePaths);
	}

	FAssetBundleData BundleData;
	BundleData.AddBundleAssetsTruncated(UAssetManager_LoadStateClient, CuePaths);

	FPrimaryAssetId PrimaryAssetId = FPrimaryAssetId(UAssetManager_GameplayCueRefsType, UAssetManager_GameplayCueRefsName);
	UAssetManager::Get().AddDynamicAsset(PrimaryAssetId, FSoftObjectPath(), BundleData);
}
