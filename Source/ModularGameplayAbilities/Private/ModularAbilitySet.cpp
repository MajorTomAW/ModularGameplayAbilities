// Author: Tom Werner (MajorT), 2025


#include "ModularAbilitySet.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ModularAbilitySet"
#endif

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "GameplayEffectApplicationInfo.h"
#include "ModularGameplayAbilitiesSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularAbilitySet)

////////////////////////////////////////////////////////////////////////
/// FAbilitySetHandle

void FAbilitySetHandle::AppendHandles(const FAbilitySetHandle& InGrantedHandles)
{
	AppendAttributeSets(InGrantedHandles.GrantedAttributeSets);
	AppendAbilitySpecHandles(InGrantedHandles.GrantedAbilityHandles);
	AppendGameplayEffectHandles(InGrantedHandles.GrantedEffectHandles);
}


void FAbilitySetHandle::SetTargetAbilitySystem(UAbilitySystemComponent* InAbilitySystem)
{
	check(InAbilitySystem);
	TargetAbilitySystem = InAbilitySystem;
}

void FAbilitySetHandle::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& InHandle)
{
	if (InHandle.IsValid())
	{
		GrantedAbilityHandles.Add(InHandle);
	}
}
void FAbilitySetHandle::AppendAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InHandles)
{
	for (const auto& Handle : InHandles)
	{
		if (Handle.IsValid())
		{
			AddAbilitySpecHandle(Handle);
		}
	}
}




void FAbilitySetHandle::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& InHandle)
{
	if (InHandle.IsValid())
	{
		GrantedEffectHandles.Add(InHandle);
	}
}
void FAbilitySetHandle::AppendGameplayEffectHandles(const TArray<FActiveGameplayEffectHandle>& InHandles)
{
	for (const auto& Handle : InHandles)
	{
		if (Handle.IsValid())
		{
			AddGameplayEffectHandle(Handle);
		}
	}
}





void FAbilitySetHandle::AddAttributeSet(UAttributeSet* InAttributeSet)
{
	if (InAttributeSet)
	{
		GrantedAttributeSets.Add(InAttributeSet);
	}
}
void FAbilitySetHandle::AppendAttributeSets(const TArray<UAttributeSet*>& InAttributeSets)
{
	for (UAttributeSet* Set : InAttributeSets)
	{
		if (IsValid(Set))
		{
			AddAttributeSet(Set);
		}
	}
}





void FAbilitySetHandle::TakeFromAbilitySystem()
{
	check(TargetAbilitySystem.IsValid());
	UAbilitySystemComponent* AbilitySystem = TargetAbilitySystem.Get();

	// Only authority can give or take abilities
	if (!AbilitySystem->IsOwnerActorAuthoritative())
	{
		return;
	}

	// Remove granted abilities
	for (const auto& Handle : GrantedAbilityHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystem->ClearAbility(Handle);
		}
	}

	// Remove granted effects
	for (const auto& Handle : GrantedEffectHandles)
	{
		if (Handle.IsValid())
		{
			AbilitySystem->RemoveActiveGameplayEffect(Handle);
		}
	}

	// Remove granted attribute sets
	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		if (Set)
		{
			AbilitySystem->RemoveSpawnedAttribute(Set);
		}
	}

	GrantedAbilityHandles.Empty();
	GrantedEffectHandles.Empty();
	GrantedAttributeSets.Empty();
	TargetAbilitySystem.Reset();
}

////////////////////////////////////////////////////////////////////////
/// UModularAbilitySet

UModularAbilitySet::UModularAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UModularAbilitySet::GiveToAbilitySystem(
	UAbilitySystemComponent* AbilitySystem, FAbilitySetHandle* OutHandle, UObject* SourceObject) const
{
	check(AbilitySystem);

	// Only authority can give or take abilities
	if (!AbilitySystem->IsOwnerActorAuthoritative())
	{
		return;
	}

	// Assign ASC
	if (OutHandle)
	{
		OutHandle->SetTargetAbilitySystem(AbilitySystem);
	}

	// Grant the abilities
	if (UModularGameplayAbilitiesSettings::IsUsingExperimentalInput())
	{
		for (int32 Idx = 0; Idx < GameplayAbilities.Num(); ++Idx)
		{
			const auto& Ability = GameplayAbilities.Array()[Idx];
			if (Ability.IsNull())
			{
				ABILITY_LOG(Error, TEXT("Ability at index %d is invalid."), Idx);
				continue;
			}

			const UClass* AbilityClass = Ability.LoadSynchronous();
			UGameplayAbility* CDO = AbilityClass->GetDefaultObject<UGameplayAbility>();

			FGameplayAbilitySpec Spec(CDO, 1.f);
			Spec.SourceObject = SourceObject;

			const FGameplayAbilitySpecHandle Handle = AbilitySystem->GiveAbility(Spec);

			if (OutHandle)
			{
				OutHandle->AddAbilitySpecHandle(Handle);
			}
		}
	}
	else
	{
		for (int32 Idx = 0; Idx < Abilities.Num(); ++Idx)
		{
			const auto& Ability = Abilities[Idx];

			if (Ability.AbilityClass.IsNull())
			{
				ABILITY_LOG(Error, TEXT("Ability at index %d is invalid."), Idx);
				continue;
			}

			const UClass* AbilityClass = Ability.AbilityClass.LoadSynchronous();
			UGameplayAbility* CDO = AbilityClass->GetDefaultObject<UGameplayAbility>();

			FGameplayAbilitySpec Spec(CDO, Ability.AbilityLevel);
			Spec.SourceObject = SourceObject;
			Spec.GetDynamicSpecSourceTags().AddTag(Ability.InputTag);

			const FGameplayAbilitySpecHandle Handle = AbilitySystem->GiveAbility(Spec);

			if (OutHandle)
			{
				OutHandle->AddAbilitySpecHandle(Handle);
			}
		}
	}

	// Grant the effects
	for (int32 Idx = 0; Idx < GameplayEffects.Num(); ++Idx)
	{
		const auto& Effect = GameplayEffects[Idx];

		if (Effect.GameplayEffect.IsNull())
		{
			ABILITY_LOG(Error, TEXT("Effect at index %d is invalid."), Idx);
			continue;
		}

		const UGameplayEffect* EffectCDO = Effect.GameplayEffect.LoadSynchronous()->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle Handle = AbilitySystem->ApplyGameplayEffectToSelf(EffectCDO, Effect.Level.GetValue(), AbilitySystem->MakeEffectContext());

		if (OutHandle)
		{
			OutHandle->AddGameplayEffectHandle(Handle);
		}
	}

	// Grant the attribute sets
	for (int32 Idx = 0; Idx < AttributeSets.Num(); ++Idx)
	{
		const auto& AttributeSet = AttributeSets[Idx];

		if (!IsValid(AttributeSet.AttributeSetClass))
		{
			ABILITY_LOG(Error, TEXT("Attribute set at index %d is invalid."), Idx);
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(AbilitySystem->GetOwner(), AttributeSet.AttributeSetClass);
		AbilitySystem->AddSpawnedAttribute(NewSet);

		if (OutHandle)
		{
			OutHandle->AddAttributeSet(NewSet);
		}
	}
}

void UModularAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* AbilitySystem, UObject* SourceObject) const
{
	GiveToAbilitySystem(AbilitySystem, nullptr, SourceObject);
}

void UModularAbilitySet::GiveToAbilitySystem(
	UAbilitySystemComponent* AbilitySystem,
	TArray<FGameplayAbilitySpecHandle>* OutAbilityHandles,
	TArray<FActiveGameplayEffectHandle>* OutEffectHandles,
	TArray<TObjectPtr<UAttributeSet>>* OutAttributeSets,
	UObject* SourceObject) const
{
	OutAbilityHandles->Reset();
	OutEffectHandles->Reset();
	OutAttributeSets->Reset();

	FAbilitySetHandle TempHandles;
	GiveToAbilitySystem(AbilitySystem, &TempHandles, SourceObject);

	// Copy the handles and attribute sets
	OutAbilityHandles = &TempHandles.GetAbilitySpecHandles();
	OutEffectHandles = &TempHandles.GetGameplayEffectHandles();
	OutAttributeSets = &TempHandles.GetAttributeSets();
}

#if WITH_EDITOR
EDataValidationResult UModularAbilitySet::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	// Validate the abilities
	if (UModularGameplayAbilitiesSettings::IsUsingExperimentalInput())
	{
		for (const auto& Ability : GameplayAbilities.Array())
		{
			if (Ability.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("InvalidGameplayAbility", "Gameplay ability is invalid in {0}."),
					FText::FromString(GetPathName())));
			}
		}
	}
	else
	{
		for (const auto& Ability : Abilities)
		{
			if (Ability.AbilityClass.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				Context.AddError(FText::Format(LOCTEXT("InvalidAbilityClass", "Ability class is invalid in {0}."),
					FText::FromString(GetPathName())));
			}
		}
	}

	// Validate the effects
	for (const auto& Effect : GameplayEffects)
	{
		if (Effect.GameplayEffect.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("InvalidEffectClass", "Effect class is invalid in {0}."),
				FText::FromString(GetPathName())));
		}
	}

	// Validate the attribute sets
	for (const auto& AttributeSet : AttributeSets)
	{
		if (!AttributeSet.AttributeSetClass)
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("InvalidAttributeSetClass", "Attribute set class is invalid in {0}."),
				FText::FromString(GetPathName())));
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE
#endif
