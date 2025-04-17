// Copyright © 2024 MajorT. All Rights Reserved.


#include "ModularAbilitySet.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ModularAbilitySet"
#endif

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularAbilitySet)

////////////////////////////////////////////////////////////////////////
/// FModularAbilitySet_GrantedHandles

void FModularAbilitySet_GrantedHandles::AppendHandles(const FModularAbilitySet_GrantedHandles& InGrantedHandles)
{
	AppendAttributeSets(InGrantedHandles.GrantedAttributeSets);
	AppendAbilitySpecHandles(InGrantedHandles.GrantedAbilityHandles);
	AppendGameplayEffectHandles(InGrantedHandles.GrantedEffectHandles);
}




void FModularAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& InHandle)
{
	if (InHandle.IsValid())
	{
		GrantedAbilityHandles.Add(InHandle);
	}
}
void FModularAbilitySet_GrantedHandles::AppendAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& InHandles)
{
	for (const auto& Handle : InHandles)
	{
		AddAbilitySpecHandle(Handle);
	}
}




void FModularAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& InHandle)
{
	if (InHandle.IsValid())
	{
		GrantedEffectHandles.Add(InHandle);
	}
}
void FModularAbilitySet_GrantedHandles::AppendGameplayEffectHandles(const TArray<FActiveGameplayEffectHandle>& InHandles)
{
	for (const auto& Handle : InHandles)
	{
		AddGameplayEffectHandle(Handle);
	}
}





void FModularAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* InAttributeSet)
{
	if (InAttributeSet)
	{
		GrantedAttributeSets.Add(InAttributeSet);
	}
}
void FModularAbilitySet_GrantedHandles::AppendAttributeSets(const TArray<UAttributeSet*>& InAttributeSets)
{
	for (UAttributeSet* Set : InAttributeSets)
	{
		AddAttributeSet(Set);
	}
}





void FModularAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* AbilitySystem)
{
	check(AbilitySystem);

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
}

////////////////////////////////////////////////////////////////////////
/// UModularAbilitySet

UModularAbilitySet::UModularAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UModularAbilitySet::GiveToAbilitySystem(
	UAbilitySystemComponent* AbilitySystem, FModularAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(AbilitySystem);

	// Only authority can give or take abilities
	if (!AbilitySystem->IsOwnerActorAuthoritative())
	{
		return;
	}

	// Grant the abilities
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

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
		Spec.GetDynamicSpecSourceTags().AddTag(Ability.InputTag);
#else
		Spec.DynamicAbilityTags.AddTag(Ability.InputTag);
#endif

		const FGameplayAbilitySpecHandle Handle = AbilitySystem->GiveAbility(Spec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(Handle);
		}
	}

	// Grant the effects
	for (int32 Idx = 0; Idx < Effects.Num(); ++Idx)
	{
		const auto& Effect = Effects[Idx];

		if (!IsValid(Effect.EffectClass))
		{
			ABILITY_LOG(Error, TEXT("Effect at index %d is invalid."), Idx);
			continue;
		}

		const UGameplayEffect* EffectCDO = Effect.EffectClass->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle Handle = AbilitySystem->ApplyGameplayEffectToSelf(EffectCDO, Effect.EffectLevel, AbilitySystem->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(Handle);
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
		AbilitySystem->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
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
	
	FModularAbilitySet_GrantedHandles TempHandles;
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
	for (const auto& Ability : Abilities)
	{
		if (Ability.AbilityClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(LOCTEXT("InvalidAbilityClass", "Ability class is invalid in {0}."),
				FText::FromString(GetPathName())));
		}
	}

	// Validate the effects
	for (const auto& Effect : Effects)
	{
		if (!Effect.EffectClass)
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