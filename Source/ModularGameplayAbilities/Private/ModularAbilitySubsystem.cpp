// Author: Tom Werner (MajorT), 2025


#include "ModularAbilitySubsystem.h"

#include "AbilitySystemLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularAbilitySubsystem)

//////////////////////////////////////////////////////////////////////////
/// FGloballyAppliedAbilities

void FGloballyAppliedAbilities::AddToAbilitySystem(
	TSubclassOf<UGameplayAbility> Ability, UModularAbilitySystemComponent* AbilitySystem)
{
	if (void* Handle = Handles.Find(AbilitySystem))
	{
		RemoveFromAbilitySystem(AbilitySystem);
	}

	UGameplayAbility* CDO = Ability->GetDefaultObject<UGameplayAbility>();
	const FGameplayAbilitySpec Spec(CDO);
	const FGameplayAbilitySpecHandle Handle = AbilitySystem->GiveAbility(Spec);
	Handles.Add(AbilitySystem, Handle);
}

void FGloballyAppliedAbilities::RemoveFromAbilitySystem(UModularAbilitySystemComponent* AbilitySystem)
{
	if (const FGameplayAbilitySpecHandle* Handle = Handles.Find(AbilitySystem))
	{
		AbilitySystem->ClearAbility(*Handle);
		Handles.Remove(AbilitySystem);
	}
}

void FGloballyAppliedAbilities::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->ClearAbility(KVP.Value);
		}
	}

	Handles.Empty();
}

//////////////////////////////////////////////////////////////////////////
/// FGloballyAppliedEffects

void FGloballyAppliedEffects::AddToAbilitySystem(
	TSubclassOf<UGameplayEffect> Effect, UModularAbilitySystemComponent* AbilitySystem)
{
	if (void* Handle = Handles.Find(AbilitySystem))
	{
		RemoveFromAbilitySystem(AbilitySystem);
	}

	const UGameplayEffect* CDO = Effect->GetDefaultObject<UGameplayEffect>();
	const FActiveGameplayEffectHandle Handle = AbilitySystem->ApplyGameplayEffectToSelf(CDO, 1.f, AbilitySystem->MakeEffectContext());
	Handles.Add(AbilitySystem, Handle);
}

void FGloballyAppliedEffects::RemoveFromAbilitySystem(UModularAbilitySystemComponent* AbilitySystem)
{
	if (const FActiveGameplayEffectHandle* Handle = Handles.Find(AbilitySystem))
	{
		AbilitySystem->RemoveActiveGameplayEffect(*Handle);
		Handles.Remove(AbilitySystem);
	}
}

void FGloballyAppliedEffects::RemoveFromAll()
{
	for (auto& KVP : Handles)
	{
		if (KVP.Key != nullptr)
		{
			KVP.Key->RemoveActiveGameplayEffect(KVP.Value);
		}
	}

	Handles.Empty();
}

//////////////////////////////////////////////////////////////////////////
/// UModularAbilitySubsystem

UModularAbilitySubsystem::UModularAbilitySubsystem()
{
}

bool UModularAbilitySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	// We don't want to create the subsystem if there are any derived classes
	// This is to prevent multiple subsystems from being created
	TArray<UClass*> DerivedClasses;
	GetDerivedClasses(GetClass(), DerivedClasses, false);
	return DerivedClasses.Num() == 0;
}

UModularAbilitySubsystem* UModularAbilitySubsystem::Get(const UObject* WorldContextObject)
{
	return WorldContextObject ? WorldContextObject->GetWorld()->GetSubsystem<UModularAbilitySubsystem>() : nullptr;
}

void UModularAbilitySubsystem::RegisterAbilitySystem(UModularAbilitySystemComponent* AbilitySystem, bool bGrantPendingAbilities, bool bGrantPendingEffects)
{
	check(AbilitySystem);

	if (bGrantPendingAbilities)
	{
		for (auto& Entry : GloballyAppliedAbilities)
		{
			Entry.Value.AddToAbilitySystem(Entry.Key, AbilitySystem);
		}
	}

	if (bGrantPendingEffects)
	{
		for (auto& Entry : GloballyAppliedEffects)
		{
			Entry.Value.AddToAbilitySystem(Entry.Key, AbilitySystem);
		}
	}

	RegisteredAbilitySystems.AddUnique(AbilitySystem);
}

void UModularAbilitySubsystem::UnregisterAbilitySystem(UModularAbilitySystemComponent* AbilitySystem)
{
	check(AbilitySystem);

	for (auto& Entry : GloballyAppliedAbilities)
	{
		Entry.Value.RemoveFromAbilitySystem(AbilitySystem);
	}

	for (auto& Entry : GloballyAppliedEffects)
	{
		Entry.Value.RemoveFromAbilitySystem(AbilitySystem);
	}

	RegisteredAbilitySystems.Remove(AbilitySystem);
}

void UModularAbilitySubsystem::ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability)
{
	if (Ability.Get() == nullptr)
	{
		ABILITY_LOG(Error, TEXT("Attempted to apply a null ability to all ability systems."));
		return;
	}

	if (!GloballyAppliedAbilities.Contains(Ability))
	{
		FGloballyAppliedAbilities& Entry = GloballyAppliedAbilities.Add(Ability);
		for (UModularAbilitySystemComponent* AbilitySystem : RegisteredAbilitySystems)
		{
			Entry.AddToAbilitySystem(Ability, AbilitySystem);
		}
	}
}

void UModularAbilitySubsystem::ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect)
{
	if (Effect.Get() == nullptr)
	{
		ABILITY_LOG(Error, TEXT("Attempted to apply a null effect to all ability systems."));
		return;
	}

	if (!GloballyAppliedEffects.Contains(Effect))
	{
		FGloballyAppliedEffects& Entry = GloballyAppliedEffects.Add(Effect);
		for (UModularAbilitySystemComponent* AbilitySystem : RegisteredAbilitySystems)
		{
			Entry.AddToAbilitySystem(Effect, AbilitySystem);
		}
	}
}

void UModularAbilitySubsystem::RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability)
{
	if (Ability.Get() == nullptr)
	{
		ABILITY_LOG(Error, TEXT("Attempted to remove a null ability from all ability systems."));
		return;
	}

	if (GloballyAppliedAbilities.Contains(Ability))
	{
		FGloballyAppliedAbilities& Entry = GloballyAppliedAbilities[Ability];
		Entry.RemoveFromAll();
		GloballyAppliedAbilities.Remove(Ability);
	}
}

void UModularAbilitySubsystem::RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect)
{
	if (Effect.Get() == nullptr)
	{
		ABILITY_LOG(Error, TEXT("Attempted to remove a null effect from all ability systems."));
		return;
	}

	if (GloballyAppliedEffects.Contains(Effect))
	{
		FGloballyAppliedEffects& Entry = GloballyAppliedEffects[Effect];
		Entry.RemoveFromAll();
		GloballyAppliedEffects.Remove(Effect);
	}
}
