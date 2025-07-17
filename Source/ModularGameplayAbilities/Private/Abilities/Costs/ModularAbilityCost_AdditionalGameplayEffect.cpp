// Author: Tom Werner (MajorT), 2025


#include "Abilities/Costs/ModularAbilityCost_AdditionalGameplayEffect.h"

#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "Abilities/ModularGameplayAbility.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_AbilityFail_Cost, "Ability.ActivateFail.Cost");

FModularAbilityCost_AdditionalGameplayEffect::FModularAbilityCost_AdditionalGameplayEffect()
{
	FailureTag = TAG_AbilityFail_Cost;
}

bool FModularAbilityCost_AdditionalGameplayEffect::CheckCost(
	const UModularGameplayAbility* Ability,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (AdditionalCostEffect == nullptr)
	{
		return true;
	}
	
	if (const UGameplayEffect* CostEffect = AdditionalCostEffect->GetDefaultObject<UGameplayEffect>())
	{
		UAbilitySystemComponent* const AbilitySystem = ActorInfo->AbilitySystemComponent.Get();
		check(AbilitySystem != nullptr);

		if (!AbilitySystem->CanApplyAttributeModifiers(
			CostEffect,
			Ability->GetAbilityLevel(),
			Ability->MakeEffectContext(Handle, ActorInfo)))
		{
			if (OptionalRelevantTags && FailureTag.IsValid())
			{
				OptionalRelevantTags->AddTag(FailureTag);
			}
			return false;
		}
	}

	return true;
}

void FModularAbilityCost_AdditionalGameplayEffect::ApplyCost(
	const UModularGameplayAbility* Ability,
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (AdditionalCostEffect == nullptr)
	{
		return;
	}

	if (const UGameplayEffect* CostEffect = AdditionalCostEffect->GetDefaultObject<UGameplayEffect>())
	{
		UAbilitySystemComponent* const AbilitySystem = ActorInfo->AbilitySystemComponent.Get();
		check(AbilitySystem != nullptr);

		if (Ability->HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
		{
			FGameplayEffectSpecHandle SpecHandle =
				Ability->MakeOutgoingGameplayEffectSpec(
					Handle,
					ActorInfo,
					ActivationInfo, CostEffect->GetClass(),
					Ability->GetAbilityLevel());

			if (SpecHandle.IsValid())
			{
				SpecHandle.Data->SetStackCount(1);
				AbilitySystem->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get(), AbilitySystem->GetPredictionKeyForNewAction());
			}
		}
	}
}
