// Author: Tom Werner (MajorT), 2025


#include "Attributes/ModularAttributeSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularAttributeSet)

UModularAttributeSet::UModularAttributeSet()
{
}

UWorld* UModularAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);
	
	return Outer->GetWorld();
}
