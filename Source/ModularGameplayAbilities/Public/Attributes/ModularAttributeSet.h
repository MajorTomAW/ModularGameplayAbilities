// Copyright © 2024 MajorT. All Rights Reserved.

#pragma once

#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ModularAbilityActor.h"

#include "ModularAttributeSet.generated.h"

class AActor;
class UModularAbilitySystemComponent;
class UGameplayEffect;
struct FGameplayEffectSpec;


/**
 * Delegate used to broadcast attribute events, some of these parameters may be null on clients:
 * @param EffectInstigator	The original instigating actor for this event
 * @param EffectCauser		The physical actor that caused the change
 * @param EffectSpec		The full effect spec for this change
 * @param EffectMagnitude	The raw magnitude, this is before clamping
 * @param OldValue			The value of the attribute before it was changed
 * @param NewValue			The value after it was changed
 */
DECLARE_MULTICAST_DELEGATE_SixParams(FGameplayAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec* /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/);

#define GAMEPLAYATTRIBUTE_NOTIFY(PropertyName)														\
	mutable FGameplayAttributeEvent On##PropertyName##Changed;										\
	float Cached##PropertyName##BeforeAttributeChange;

#define GAMEPLAYATTRIBUTE_NOTIFY_OUT(PropertyName)													\
	mutable FGameplayAttributeEvent OnOutOf##PropertyName;								\
	bool bOutOf##PropertyName;

/** Accessor macro for attribute properties */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)												\
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)										\
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)													\
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)													\
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)																								

/** Accessor macro for attribute properties with notify delegate */
#define ATTRIBUTE_ACCESSORS_NOTIFY(ClassName, PropertyName)											\
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)										\
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)													\
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)													\
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)													\
	GAMEPLAYATTRIBUTE_NOTIFY(PropertyName)															

/** Accessor macro for attribute properties with notify delegate and out of bounds delegate */
#define ATTRIBUTE_ACCESSORS_NOTIFY_DEPLETED(ClassName, PropertyName)									\
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)										\
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)													\
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)													\
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)													\
	GAMEPLAYATTRIBUTE_NOTIFY(PropertyName)															\
	GAMEPLAYATTRIBUTE_NOTIFY_OUT(PropertyName) \

/** Lazy RepNotify macro for attribute properties */
#define LAZY_ATTRIBUTE_REPNOTIFY(ClassName, PropertyName, OldValue)									\
{																									\
	static FProperty* ThisProperty = FindFieldChecked<FProperty>(ClassName::StaticClass(),			\
		GET_MEMBER_NAME_CHECKED(ClassName, PropertyName));											\
																									\
	if (!GetOwningAbilitySystemComponent())															\
	{																								\
		if (AModularAbilityActor* AbilityActor = Cast<AModularAbilityActor>(GetOwningActor()))		\
		{																							\
			AbilityActor->SetPendingAttributeFromReplication(ThisProperty, OldValue);				\
		}																							\
	}																								\
	else																							\
	{																								\
		GetOwningAbilitySystemComponentChecked()->SetBaseAttributeValueFromReplication(				\
				FGameplayAttribute(ThisProperty), PropertyName, OldValue);							\
	}																								\
}

/**
 * Extension of the base AttributeSet class that adds additional functionality for the ModularGameplayAbilities plugin.
 * Such as lazy initialization of attributes and attribute data.
 */
UCLASS(Abstract)
class MODULARGAMEPLAYABILITIES_API UModularAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UModularAttributeSet();

	//~ Begin UObject Interface
	virtual UWorld* GetWorld() const override;
	//~ End UObject Interface

protected:
	/** Called to clamp attribute values, override this to add custom clamping logic */
	virtual void ClampAttribute(const FGameplayAttribute& InAttribute, float& OutValue) const {}

protected:
	template <typename T>
	T* GetAbilitySystem() const
	{
		return Cast<T>(GetOwningAbilitySystemComponent());
	}
};
