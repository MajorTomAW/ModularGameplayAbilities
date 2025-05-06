// Copyright © 2025 MajorT. All Rights Reserved.


#include "Tasks/AbilityTask_TakePlayerSnapshot.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(AbilityTask_TakePlayerSnapshot)

void UAbilityTask_TakePlayerSnapshot::RestoreSnapshot()
{
	// Clean up all current abilities
	ClearExistingAbilities();

	// Restore the snapshot
#if WITH_GAMEPLAYTASK_DEBUG
	ABILITY_LOG(Display, TEXT("\n==================== Restoring Abilities ===================="));
#endif
	for (const auto& Pair : CachedAbilities)
	{
		FGameplayAbilitySpec Spec(Pair.Key, 1, INDEX_NONE, Ability->GetOwningActorFromActorInfo());
		Spec.DynamicAbilityTags.AppendTags(Pair.Value);

		AbilitySystemComponent->GiveAbility(Spec);

#if WITH_GAMEPLAYTASK_DEBUG
		ABILITY_LOG(Display, TEXT("Restore Ability: %s [%s]"),
			*Pair.Key->GetName(), *Pair.Value.ToString());
#endif
	}
#if WITH_GAMEPLAYTASK_DEBUG
	ABILITY_LOG(Display, TEXT("============================================================"));
#endif


	// Restore snapshot attributes
#if WITH_GAMEPLAYTASK_DEBUG
	ABILITY_LOG(Display, TEXT("\n==================== Restoring Attributes ===================="));
#endif
	TArray<FGameplayAttribute> Attributes;
	AbilitySystemComponent->GetAllAttributes(Attributes);
	
	// Restore the attributes
	// Do this 2x as some attributes are capped by other attributes
	bool bIsFirstTime = true;
loop:
	for (const auto& Pair : CachedAttributes)
	{
		FGameplayAttribute* Attribute = Attributes.FindByPredicate([Pair](const FGameplayAttribute& Other) -> bool
		{
			return Other.GetUProperty() == Pair.Key;
		});

		if (ensure(Attribute))
		{
			//const UAttributeSet* Set = AbilitySystemComponent->GetAttributeSet(Attribute->GetAttributeSetClass());
			float NewVal = Pair.Value.GetCurrentValue();
			const float OldValue = AbilitySystemComponent->GetNumericAttribute(*Attribute);
			
			//Attribute->SetNumericValueChecked(NewVal, const_cast<UAttributeSet*>(Set));
			AbilitySystemComponent->SetNumericAttributeBase(*Attribute, NewVal);

#if WITH_GAMEPLAYTASK_DEBUG
			if (!bIsFirstTime)
			ABILITY_LOG(Display, TEXT("Restore Attribute: %s \t\t Old: %.1f \t [B:%.1f -> C:%.1f -> R:%.1f]"),
				*Pair.Key->GetName(), OldValue, Pair.Value.GetBaseValue(), NewVal, AbilitySystemComponent->GetNumericAttribute(*Attribute));
#endif
		}
	}
	if (bIsFirstTime)
	{
		bIsFirstTime = false;
		goto loop;
	}
#if WITH_GAMEPLAYTASK_DEBUG
	ABILITY_LOG(Display, TEXT("============================================================"));
#endif
}

UAbilityTask_TakePlayerSnapshot* UAbilityTask_TakePlayerSnapshot::TakePlayerSnapshot(
	UGameplayAbility* OwningAbility,
	FGameplayTagQuery AbilityQuery,
	bool bClearCachedAbilities,
	bool bCacheAttributes,
	bool bAutoRestoreWhenEndedOrCanceled)
{
	UAbilityTask_TakePlayerSnapshot* Task = NewAbilityTask<UAbilityTask_TakePlayerSnapshot>(OwningAbility);
	Task->AbilityQueryToRun = AbilityQuery;
	Task->bShouldClearCachedAbilities = bClearCachedAbilities;
	Task->bShouldCacheAttributes = bCacheAttributes;
	Task->bAutoRestoreOnEndOrCancel = bAutoRestoreWhenEndedOrCanceled;

	return Task;
}

void UAbilityTask_TakePlayerSnapshot::Activate()
{
	check(AbilitySystemComponent.IsValid())
	
	// Cache the abilities that match the query
	const TArray<FGameplayAbilitySpec>& Abilities =
		AbilitySystemComponent->GetActivatableAbilities();
	
	TArray<FGameplayAbilitySpecHandle> AbilitiesToClear = {};

	for (const FGameplayAbilitySpec& Spec : Abilities)
	{
		// We don't want to cache ourselves
		if (Spec.Handle == Ability->GetCurrentAbilitySpecHandle())
		{
			continue;
		}
		
		if (AbilityQueryToRun.Matches(Spec.Ability->AbilityTags))
		{
			// Cache ability class and dynamic tags to restore them later
			CachedAbilities.Add(
				Spec.Ability->GetClass(),
				Spec.DynamicAbilityTags);

			// We kinda check this twice, but it's not a big deal
			if (bShouldClearCachedAbilities)
			{
				AbilitiesToClear.Add(Spec.Handle);
			}
		}
	}
	// Clear all the cached abilities if we should
	if (!CachedAbilities.IsEmpty() && bShouldClearCachedAbilities)
	{
		for (const FGameplayAbilitySpecHandle& Handle : AbilitiesToClear)
		{
			AbilitySystemComponent->ClearAbility(Handle);
		}
	}

	// Cache the player attributes if needed
	if (bShouldCacheAttributes)
	{
		TArray<FGameplayAttribute> Attributes;
		AbilitySystemComponent->GetAllAttributes(Attributes);

		CachedAttributes.Reset();
		for (FGameplayAttribute& Attribute : Attributes)
		{
			// We don't want to cache meta-attributes
			if (!Attribute.GetUProperty()->HasAnyPropertyFlags(CPF_Net))
			{
				continue;
			}
			
			if (ensure(Attribute.IsValid()))
			{
				const UAttributeSet* Set =
					AbilitySystemComponent->GetAttributeSet(Attribute.GetAttributeSetClass());

				if (ensure(Set))
				{
					CachedAttributes.Add(
						Attribute.GetUProperty(),
						Attribute.GetNumericValue(Set));	
				}
			}
		}	
	}



#if WITH_GAMEPLAYTASK_DEBUG
	// Log out all the cached abilities and attributes
	ABILITY_LOG(Display, TEXT("==================== Cached Abilities ===================="));
	for (const auto& Pair : CachedAbilities)
	{
		ABILITY_LOG(Display, TEXT("Ability: %s"), *Pair.Key->GetName());
	}

	// Log out all the cached attributes
	ABILITY_LOG(Display, TEXT("==================== Cached Attributes ===================="));
	for (const auto& Pair : CachedAttributes)
	{
		ABILITY_LOG(Display, TEXT("Attribute: %s	\t\t [%.1f -> %.1f]"),
			*Pair.Key->GetName(), Pair.Value.GetBaseValue(), Pair.Value.GetCurrentValue());
	}

	ABILITY_LOG(Display, TEXT("============================================================"));
	ABILITY_LOG(Display, TEXT("Total Cached Abilities: %d"), CachedAbilities.Num());
	ABILITY_LOG(Display, TEXT("Total Cached Attributes: %d"), CachedAttributes.Num());
	ABILITY_LOG(Display, TEXT("============================================================"));
#endif
}

void UAbilityTask_TakePlayerSnapshot::ExternalCancel()
{
	if (bAutoRestoreOnEndOrCancel)
	{
		RestoreSnapshot();
	}
	
	Super::ExternalCancel();
}

void UAbilityTask_TakePlayerSnapshot::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_TakePlayerSnapshot::ClearExistingAbilities()
{
	// Copy so we can modify the array while iterating
	TArray<FGameplayAbilitySpec> Abilities =
		AbilitySystemComponent->GetActivatableAbilities();

	for (const FGameplayAbilitySpec& AbilityToClear : Abilities)
	{
		if (AbilityToClear.Handle == Ability->GetCurrentAbilitySpecHandle())
		{
			continue;
		}

		AbilitySystemComponent->ClearAbility(AbilityToClear.Handle);
	}
}
