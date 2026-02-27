// Author: Tom Werner (MajorT), 2025

#pragma once

#include "UObject/Interface.h"

#include "IPendingAttributeReceiver.generated.h"

struct FGameplayAttributeData;
struct FGameplayAttribute;

/** A pending attribute replication */
struct FPendingAttributeReplication
{
public:
	FPendingAttributeReplication() = default;
	FPendingAttributeReplication(const FGameplayAttribute& InAttribute, const FGameplayAttributeData& InNewValue)
		: Attribute(InAttribute), NewValue(InNewValue) {}

	FGameplayAttribute Attribute;
	FGameplayAttributeData NewValue;
};

UINTERFACE()
class UPendingAttributeReceiver : public UInterface
{
	GENERATED_BODY()
};

class IPendingAttributeReceiver
{
	GENERATED_BODY()

public:
	/** Called to update any pending attributes that were set before the Ability System was initialized. */
	virtual void SetPendingAttributeFromReplication(const FGameplayAttribute& Attribute, const FGameplayAttributeData& NewValue) = 0;
};
