// Author: Tom Werner (MajorT), 2025


#include "ModularAbilitySystemComponent.h"

#include "AbilitySystemLog.h"
#include "ModularAbilitySubsystem.h"
#include "ModularAbilityTagRelationshipMapping.h"
#include "ModularGameplayAbilitiesSettings.h"
#include "Abilities/ModularGameplayAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularAbilitySystemComponent)

UModularAbilitySystemComponent::UModularAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputHeldHandles.Reset();
	InputPressedHandles.Reset();
	InputReleasedHandles.Reset();
	
	FMemory::Memset(ActivationGroupCounts, 0 , sizeof(ActivationGroupCounts));
}

void UModularAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unregister from the global ability system
	if (UModularAbilitySubsystem* AbilitySub = UModularAbilitySubsystem::Get(this))
	{
		AbilitySub->UnregisterAbilitySystem(this);
	}
	
	Super::EndPlay(EndPlayReason);
}


bool UModularAbilitySystemComponent::IsActivationGroupBlocked(EGameplayAbilityActivationGroup::Type Group) const
{
	bool bBlocked = false;

	switch (Group) {
	case EGameplayAbilityActivationGroup::Independent:
		{ // Independent abilities are never blocked
			bBlocked = false;
			break;
		}
	case EGameplayAbilityActivationGroup::Exclusive_Replaceable:
	case EGameplayAbilityActivationGroup::Exclusive_Blocking:
		{ // Exclusive abilities can activate if nothing is blocked
			bBlocked = (ActivationGroupCounts[(uint8)EGameplayAbilityActivationGroup::Exclusive_Blocking] > 0);
			break;
		}
	default:
		{ // Invalid group
			ensureAlwaysMsgf(false, TEXT("Invalid activation group: %d"), (uint8)Group);
			break;
		}
	}

	return bBlocked;
}

void UModularAbilitySystemComponent::AddAbilityToActivationGroup(
	EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);
	
	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group) {
	case EGameplayAbilityActivationGroup::Independent:
		{ // Independent abilities can't cancel other abilities
			break;
		}
	case EGameplayAbilityActivationGroup::Exclusive_Replaceable:
	case EGameplayAbilityActivationGroup::Exclusive_Blocking:
		{
			CancelActivationGroupAbilities(EGameplayAbilityActivationGroup::Exclusive_Replaceable, Ability, bReplicateCancelAbility);
			break;
		}
	default:
		{ // Invalid group
			ensureAlwaysMsgf(false, TEXT("Invalid activation group: %d"), (uint8)Group);
			break;
		}
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)EGameplayAbilityActivationGroup::Exclusive_Blocking] +
		ActivationGroupCounts[(uint8)EGameplayAbilityActivationGroup::Exclusive_Replaceable];

	ensureMsgf(ExclusiveCount <= 1, TEXT("%hs: Multiple exclusive abilities are running at the same time!"), __func__);
}

void UModularAbilitySystemComponent::RemoveAbilityFromActivationGroup(
	EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

void UModularAbilitySystemComponent::CancelActivationGroupAbilities(
	EGameplayAbilityActivationGroup::Type Group, UModularGameplayAbility* AbilityToIgnore, bool bReplicateCancelAbilities)
{
	auto ShouldCancelFunc = [this, Group, AbilityToIgnore](const UModularGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
	{
		return ((Ability->GetActivationGroup() == Group) && (Ability != AbilityToIgnore));
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbilities);
}

void UModularAbilitySystemComponent::SetTagRelationshipMapping(UModularAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

void UModularAbilitySystemComponent::ClearTagRelationshipMapping()
{
	if (IsValid(TagRelationshipMapping))
	{
		// Do some cleanup ??
	}

	TagRelationshipMapping = nullptr;
}

void UModularAbilitySystemComponent::GetAdditionalActivationTagRequirements(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutActivationRequired,
	FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetActivationRequiredAndBlockedTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

FGameplayAbilitySpecHandle UModularAbilitySystemComponent::GetTrackedActorsForAbility(
	const UGameplayAbility* Ability,
	TArray<FAbilityTrackedActorEntry>& OutTrackedActors) const
{
	OutTrackedActors.Reset();

	// Make sure we have a valid ability
	if (!Ability || !Ability->GetCurrentActorInfo())
	{
		return FGameplayAbilitySpecHandle();
	}

	// Only instantiated abilities can have tracked actors
	if (!Ability->IsInstantiated())
	{
		return FGameplayAbilitySpecHandle();
	}
	
	UModularAbilitySystemComponent* AbilitySystem =
		Cast<UModularAbilitySystemComponent>(Ability->GetCurrentActorInfo()->AbilitySystemComponent.Get());
	if (!AbilitySystem)
	{
		return FGameplayAbilitySpecHandle();
	}

	FGameplayAbilitySpecHandle SpecHandle = Ability->GetCurrentAbilitySpecHandle();
	if (!SpecHandle.IsValid())
	{
		return FGameplayAbilitySpecHandle();
	}

	const auto& TrackedActorsMap = AbilitySystem->AbilitySpecTrackedActors;
	if (!TrackedActorsMap.Contains(SpecHandle))
	{
		return FGameplayAbilitySpecHandle();
	}

	if (const TArray<FAbilityTrackedActorEntry>* TrackedActors = TrackedActorsMap.Find(SpecHandle))
	{
		OutTrackedActors = *TrackedActors;
	}

	return SpecHandle;
}

int32 UModularAbilitySystemComponent::GetNumTrackedActorsForAbility(const UGameplayAbility* Ability) const
{
	// Make sure we have a valid ability
	if (!Ability || !Ability->GetCurrentActorInfo())
	{
		return 0;
	}

	// Only instantiated abilities can have tracked actors
	if (!Ability->IsInstantiated())
	{
		return 0;
	}
	
	UModularAbilitySystemComponent* AbilitySystem =
		Cast<UModularAbilitySystemComponent>(Ability->GetCurrentActorInfo()->AbilitySystemComponent.Get());
	if (!IsValid(AbilitySystem))
	{
		return 0;
	}

	FGameplayAbilitySpecHandle SpecHandle = Ability->GetCurrentAbilitySpecHandle();
	if (!SpecHandle.IsValid())
	{
		return 0;
	}

	const TArray<FAbilityTrackedActorEntry>* TrackedActors = AbilitySpecTrackedActors.Find(SpecHandle);
	if (TrackedActors == nullptr)
	{
		return 0;
	}

	return TrackedActors->Num();
}

bool UModularAbilitySystemComponent::StartTrackingActorForAbility(AActor* ActorToTrack, const UGameplayAbility* Ability)
{
	// Make sure we have a valid ability
	if (!Ability || !Ability->GetCurrentActorInfo())
	{
		return false;
	}

	// Only instantiated abilities can have tracked actors
	if (!Ability->IsInstantiated())
	{
		return false;
	}
	
	UModularAbilitySystemComponent* AbilitySystem =
		Cast<UModularAbilitySystemComponent>(Ability->GetCurrentActorInfo()->AbilitySystemComponent.Get());
	if (!IsValid(AbilitySystem))
	{
		return false;
	}

	FGameplayAbilitySpecHandle SpecHandle = Ability->GetCurrentAbilitySpecHandle();
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	// Add and initialize the new tracked actor entry
	FAbilityTrackedActorEntry& TrackedActor = AbilitySpecTrackedActors.FindOrAdd(SpecHandle).AddDefaulted_GetRef();
	TrackedActor.TrackedActor = ActorToTrack;
	TrackedActor.TrackedTime = AbilitySystem->GetWorld()->GetTimeSeconds();

	return true;
}

bool UModularAbilitySystemComponent::StartTrackingActorsForTag(AActor* ActorToTrack, const FGameplayTag& GroupTag)
{
	if (!GroupTag.IsValid())
	{
		return false;
	}

	// Add and initialize the new tracked actor entry
	FAbilityTrackedActorEntry& TrackedActor = TagTrackedActors.FindOrAdd(GroupTag).AddDefaulted_GetRef();
	TrackedActor.TrackedActor = ActorToTrack;
	TrackedActor.TrackedTime = GetWorld()->GetTimeSeconds();

	return true;
}

void UModularAbilitySystemComponent::GetTrackedActorsForTag(
	const FGameplayTag& Tag,
	TArray<FAbilityTrackedActorEntry>& OutTrackedActors) const
{
	OutTrackedActors.Reset();

	if (!Tag.IsValid())
	{
		return;
	}
	
	if (!TagTrackedActors.Contains(Tag))
	{
		return;
	}

	if (const TArray<FAbilityTrackedActorEntry>* TrackedActors = TagTrackedActors.Find(Tag))
	{
		OutTrackedActors = *TrackedActors;
	}
}

void UModularAbilitySystemComponent::ClearTrackedActorsForAbility(const UGameplayAbility* Ability, bool bDestroyActors)
{
	// Make sure we have a valid ability
	if (!Ability || !Ability->GetCurrentActorInfo())
	{
		return;
	}

	// Only instantiated abilities can have tracked actors
	if (!Ability->IsInstantiated())
	{
		return;
	}
	
	UModularAbilitySystemComponent* AbilitySystem =
		Cast<UModularAbilitySystemComponent>(Ability->GetCurrentActorInfo()->AbilitySystemComponent.Get());
	if (!IsValid(AbilitySystem))
	{
		return;
	}

	FGameplayAbilitySpecHandle SpecHandle = Ability->GetCurrentAbilitySpecHandle();
	if (!SpecHandle.IsValid())
	{
		return;
	}

	if (!AbilitySpecTrackedActors.Contains(SpecHandle))
	{
		return;
	}

	if (bDestroyActors)
	{
		if (const TArray<FAbilityTrackedActorEntry>* TrackedActors = AbilitySpecTrackedActors.Find(SpecHandle))
		{
			for (const FAbilityTrackedActorEntry& Entry : *TrackedActors)
			{
				if (!Entry.TrackedActor.IsValid())
				{
					continue;
				}


				//@TODO: Naively destroy the actor and hope everything goes well ???
				Entry.TrackedActor->Destroy();
			}
		}
	}

	//@TODO: Simply remove? Nothing else to do here?
	AbilitySpecTrackedActors.Remove(SpecHandle);
}

void UModularAbilitySystemComponent::ClearTrackedGroupedActors(FGameplayTag GroupTag, bool bDestroyActors)
{
	if (!TagTrackedActors.Contains(GroupTag))
	{
		return;
	}

	if (bDestroyActors)
	{
		if (const TArray<FAbilityTrackedActorEntry>* TrackedActors = TagTrackedActors.Find(GroupTag))
		{
			for (const FAbilityTrackedActorEntry& Entry : *TrackedActors)
			{
				if (!Entry.TrackedActor.IsValid())
				{
					continue;
				}


				//@TODO: Naively destroy the actor and hope everything goes well ???
				Entry.TrackedActor->Destroy();
			}
		}
	}

	//@TODO: Simply remove? Nothing else to do here?
	TagTrackedActors.Remove(GroupTag);
}

void UModularAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		if (const UGameplayAbility* PrimaryInstance = Spec.GetPrimaryInstance())
		{
			const FPredictionKey PredictionKey = PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey();
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, PredictionKey);
		}
		else
		{
			for (const UGameplayAbility* Instance : Spec.GetAbilityInstances())
			{
				const FPredictionKey PredictionKey = Instance->GetCurrentActivationInfo().GetActivationPredictionKey();
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, PredictionKey);
			}
		}
	}
}

void UModularAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);
	
	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		if (const UGameplayAbility* PrimaryInstance = Spec.GetPrimaryInstance())
		{
			const FPredictionKey PredictionKey = PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey();
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, PredictionKey);
		}
		else
		{
			for (const UGameplayAbility* Instance : Spec.GetAbilityInstances())
			{
				const FPredictionKey PredictionKey = Instance->GetCurrentActivationInfo().GetActivationPredictionKey();
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, PredictionKey);
			}
		}
	}
}

void UModularAbilitySystemComponent::NotifyAbilityActivated(
	const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UModularGameplayAbility* ModularAbility = Cast<UModularGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(ModularAbility->GetActivationGroup(), ModularAbility);
	}
}

void UModularAbilitySystemComponent::NotifyAbilityFailed(
	const FGameplayAbilitySpecHandle Handle,
	UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (const APawn* Pawn = Cast<APawn>(GetAvatarActor()))
	{
		if (!Pawn->IsLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}
void UModularAbilitySystemComponent::HandleAbilityFailed(
	const UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureReason)
{
	if (const UModularGameplayAbility* ModularAbility = Cast<UModularGameplayAbility>(Ability))
	{
		ModularAbility->OnAbilityFailedToActivate(FailureReason);
	}
}

void UModularAbilitySystemComponent::ClientNotifyAbilityFailed_Implementation(
	const UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureReason)
{
	HandleAbilityFailed(Ability, FailureReason);
}

void UModularAbilitySystemComponent::NotifyAbilityEnded(
	FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UModularGameplayAbility* ModularAbility = Cast<UModularGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(ModularAbility->GetActivationGroup(), ModularAbility);
	}
}

void UModularAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	const bool bHasNewPawnAvatar = Cast<APawn>(InAvatarActor) && (InAvatarActor != ActorInfo->AvatarActor);
	
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	if (!bHasNewPawnAvatar)
	{
		return;
	}

	// Notify all active abilities that the avatar actor has changed
	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		UModularGameplayAbility* AbilityCDO = Cast<UModularGameplayAbility>(Spec.Ability);
		if (!AbilityCDO)
		{
			continue;
		}
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			for (UGameplayAbility* Instance : Instances)
			{
				if (UModularGameplayAbility* ModularInstance = Cast<UModularGameplayAbility>(Instance))
				{
					ModularInstance->OnPawnAvatarSet();
				}
			}
		}
		else
		{
			AbilityCDO->OnPawnAvatarSet();
		}
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}

	// Register with the global ability subsystem.
	if (UModularAbilitySubsystem* AbilitySub = UModularAbilitySubsystem::Get(this))
	{
		AbilitySub->RegisterAbilitySystem(this);
	}

	TryActivateAbilitiesOnSpawn();
}

void UModularAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(
	const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
	const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer BlockTagsCopy = BlockTags;
	FGameplayTagContainer CancelTagsCopy = CancelTags;

	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &BlockTagsCopy, &CancelTagsCopy);
	}
	
	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, BlockTagsCopy, bExecuteCancelTags, CancelTagsCopy);
}

void UModularAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	OnAbilityAddedEvent.Broadcast(Cast<UModularGameplayAbility>(AbilitySpec.GetPrimaryInstance()));
}

void UModularAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);

	OnAbilityRemovedEvent.Broadcast(Cast<UModularGameplayAbility>(AbilitySpec.GetPrimaryInstance()));
}

void UModularAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		const UModularGameplayAbility* AbilityCDO = Cast<UModularGameplayAbility>(Spec.Ability);
		if (!AbilityCDO)
		{
			continue;
		}

		bool bAnyInstancesFound = false;
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			for (UGameplayAbility* Instance : Instances)
			{
				if (const UModularGameplayAbility* ModularInstance = Cast<UModularGameplayAbility>(Instance))
				{
					ModularInstance->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), Spec);
				}
			}

			bAnyInstancesFound = Instances.Num() > 0;
		}
		
		if (!bAnyInstancesFound)
		{
			AbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), Spec);
		}
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}
}

void UModularAbilitySystemComponent::GetAbilityTargetData(
	const FGameplayAbilitySpecHandle AbilityHandle,
	const FGameplayAbilityActivationInfo& ActivationInfo,
	FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedDataCache =
		AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));

	if (ReplicatedDataCache.IsValid())
	{
		OutTargetDataHandle = ReplicatedDataCache->TargetData;
	}
}

void UModularAbilitySystemComponent::CancelAbilitiesByFunc(
	TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();

	for (const auto& Spec : ActivatableAbilities.Items)
	{
		if (!Spec.IsActive())
		{
			continue;
		}

		UModularGameplayAbility* AbilityCDO = Cast<UModularGameplayAbility>(Spec.Ability);
		if (!AbilityCDO)
		{
			continue;
		}
PRAGMA_DISABLE_DEPRECATION_WARNINGS
		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			// Cancel all instances of the ability
			TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
			for (UGameplayAbility* Instance : Instances)
			{
				UModularGameplayAbility* ModularAbility = CastChecked<UModularGameplayAbility>(Instance);

				if (ShouldCancelFunc(ModularAbility, Spec.Handle))
				{
					if (ModularAbility->CanBeCanceled())
					{
						ModularAbility->CancelAbility(Spec.Handle, AbilityActorInfo.Get(), ModularAbility->GetCurrentActivationInfo(), bReplicateCancelAbility);
					}
					else
					{
						ABILITY_LOG(Error, TEXT("%hs: Can't cancel ability [%s] because CanBeCanceled() is false."), __func__, *ModularAbility->GetName());
					}
				}
			}
		}
		else
		{
			// Cancel CDO
			if (ShouldCancelFunc(AbilityCDO, Spec.Handle))
			{
				// CDO can always be canceled
				check(AbilityCDO->CanBeCanceled());
				AbilityCDO->CancelAbility(Spec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
			}
		}
PRAGMA_ENABLE_DEPRECATION_WARNINGS
	}
}

void UModularAbilitySystemComponent::CancelInputActivatedAbilities(bool bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](const UModularGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
	{
		const EGameplayAbilityActivationPolicy::Type ActivationPolicy = Ability->GetActivationPolicy();
		return (ActivationPolicy == EGameplayAbilityActivationPolicy::Active) || Ability->IsForceReceiveInput();
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UModularAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (UModularGameplayAbilitiesSettings::IsUsingExperimentalInput())
	{
		checkf(false, TEXT("UModularGameplayAbilitiesSettings::IsUsingExperimentalInput() is enabled. Please use the new input system instead of the old one."));
	}
	
	if (!InputTag.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
		if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
#else
		if (Spec.Ability && Spec.DynamicAbilityTags.HasTagExact(InputTag))
#endif
		{
			UModularGameplayAbility* CDO = Cast<UModularGameplayAbility>(Spec.Ability);
			if (CDO == nullptr)
			{
				continue;
			}

			if ((CDO->GetActivationPolicy() != EGameplayAbilityActivationPolicy::Active) &&
				!CDO->IsForceReceiveInput())
			{
				continue;
			}

			// If the ability is already held, don't add it again
			// Otherwise the ability will keep activating
			if (InputHeldHandles.Contains(Spec.Handle))
			{
				continue;
			}

			InputPressedHandles.AddUnique(Spec.Handle);
			InputHeldHandles.AddUnique(Spec.Handle);
		}
	}
}

void UModularAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (UModularGameplayAbilitiesSettings::IsUsingExperimentalInput())
	{
		checkf(false, TEXT("UModularGameplayAbilitiesSettings::IsUsingExperimentalInput() is enabled. Please use the new input system instead of the old one."));
	}
	
	if (!InputTag.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
		if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
#else
		if (Spec.Ability && Spec.DynamicAbilityTags.HasTagExact(InputTag))
#endif
		{
			InputHeldHandles.Remove(Spec.Handle);
			InputReleasedHandles.AddUnique(Spec.Handle);
		}
	}
}

void UModularAbilitySystemComponent::AbilityInputIdPressed(int32 InputId)
{
	if (UModularGameplayAbilitiesSettings::IsNotUsingExperimentalInput())
	{
		checkf(false, TEXT("UModularGameplayAbilitiesSettings::IsUsingExperimentalInput() is disabled. Please use the old input system instead of the new one."));
	}

	if (InputId == INDEX_NONE)
	{
		return;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && Spec.InputID == InputId)
		{
			UModularGameplayAbility* CDO = Cast<UModularGameplayAbility>(Spec.Ability);
			if (CDO == nullptr)
			{
				continue;
			}

			if ((CDO->GetActivationPolicy() != EGameplayAbilityActivationPolicy::Active) &&
				!CDO->IsForceReceiveInput())
			{
				continue;
			}

			// If the ability is already held, don't add it again
			// Otherwise the ability will keep activating
			if (InputHeldHandles.Contains(Spec.Handle))
			{
				continue;
			}

			InputPressedHandles.AddUnique(Spec.Handle);
			InputHeldHandles.AddUnique(Spec.Handle);
		}
	}
}

void UModularAbilitySystemComponent::AbilityInputIdReleased(int32 InputId)
{
	if (UModularGameplayAbilitiesSettings::IsNotUsingExperimentalInput())
	{
		checkf(false, TEXT("UModularGameplayAbilitiesSettings::IsUsingExperimentalInput() is disabled. Please use the old input system instead of the new one."));
	}

	if (InputId == INDEX_NONE)
	{
		return;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && Spec.InputID == InputId)
		{
			InputHeldHandles.Remove(Spec.Handle);
			InputReleasedHandles.AddUnique(Spec.Handle);
		}
	}
}

void UModularAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (bGamePaused)
	{
		ClearAbilityInput();
		return;
	}
	
	if (!IsAbilityInputAllowed())
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> HandlesToActivate;
	HandlesToActivate.Reset();

	// Process input for abilities that are held
	for (const auto& Handle : InputHeldHandles)
	{
		if (const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle))
		{
			if (!Spec->Ability)
			{
				continue;
			}
			
			const UModularGameplayAbility* CDO = Cast<UModularGameplayAbility>(Spec->Ability);
			if (CDO && (CDO->GetActivationPolicy() == EGameplayAbilityActivationPolicy::Active) && CDO->IsForceReceiveInput())
			{
				HandlesToActivate.AddUnique(Handle);
			}
		}
	}

	// Process input for abilities that are pressed
	for (const auto& Handle : InputPressedHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle))
		{
			if (!Spec->Ability)
			{
				continue;
			}

			Spec->InputPressed = true;

			if (Spec->IsActive())
			{
				// Invoke the input pressed event
				AbilitySpecInputPressed(*Spec);
			}
			else
			{
				const UModularGameplayAbility* CDO = Cast<UModularGameplayAbility>(Spec->Ability);

				if (CDO && (
					(CDO->GetActivationPolicy() == EGameplayAbilityActivationPolicy::Active) || CDO->IsForceReceiveInput()))
				{
					HandlesToActivate.AddUnique(Handle);
				}
			}
		}
	}

	// Try to activate abilities
	for (const auto& Handle : HandlesToActivate)
	{
		bool bDidActivate = TryActivateAbility(Handle);

		ABILITY_LOG(Display, TEXT("TryActivateAbility %s: %s"), *Handle.ToString(), bDidActivate ? TEXT("Success") : TEXT("Failed"));
	}

	// Process input for abilities that are released
	for (const auto& Handle : InputReleasedHandles)
	{
		if (FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle))
		{
			if (!Spec->Ability)
			{
				continue;
			}

			Spec->InputPressed = false;

			if (Spec->IsActive())
			{
				// Invoke the input released event
				AbilitySpecInputReleased(*Spec);
			}
		}
	}

	InputPressedHandles.Reset();
	InputReleasedHandles.Reset();
}

void UModularAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedHandles.Reset();
	InputHeldHandles.Reset();
	InputReleasedHandles.Reset();
}

void UModularAbilitySystemComponent::DeferredSetBaseAttributeValueFromReplication(
	const FGameplayAttribute& Attribute, float NewValue)
{
	const float OldValue = ActiveGameplayEffects.GetAttributeBaseValue(Attribute);
	ActiveGameplayEffects.SetAttributeBaseValue(Attribute, NewValue);
	SetBaseAttributeValueFromReplication(Attribute, OldValue, NewValue);
}
void UModularAbilitySystemComponent::DeferredSetBaseAttributeValueFromReplication(
	const FGameplayAttribute& Attribute, FGameplayAttributeData NewValue)
{
	DeferredSetBaseAttributeValueFromReplication(Attribute, NewValue.GetBaseValue());
}





