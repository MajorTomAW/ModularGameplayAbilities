// Copyright © 2024 Playton. All Rights Reserved.


#include "ModularAbilitySystemComponent.h"

#include "AbilitySystemLog.h"
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
	const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);
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

	TryActivateAbilitiesOnSpawn();
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
		return ActivationPolicy == EGameplayAbilityActivationPolicy::Active;
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UModularAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			UModularGameplayAbility* CDO = Cast<UModularGameplayAbility>(Spec.Ability);
			if (CDO == nullptr)
			{
				continue;
			}

			if (CDO->GetActivationPolicy() != EGameplayAbilityActivationPolicy::Active)
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
	if (!InputTag.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.Ability && Spec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
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
			if (CDO && CDO->GetActivationPolicy() == EGameplayAbilityActivationPolicy::Active)
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

				if (CDO && CDO->GetActivationPolicy() == EGameplayAbilityActivationPolicy::Active)
				{
					HandlesToActivate.AddUnique(Handle);
				}
			}
		}
	}

	// Try to activate abilities
	for (const auto& Handle : HandlesToActivate)
	{
		TryActivateAbility(Handle);
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





