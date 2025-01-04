// Copyright © 2024 Playton. All Rights Reserved.


#include "ModularAbilityActor.h"

#include "ModularAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularAbilityActor)

AModularAbilityActor::AModularAbilityActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AbilitySystemClass = UModularAbilitySystemComponent::StaticClass();
	CreationPolicy = EModularAbilitySystemCreationPolicy::Lazy;

	bReplicates = true;
}

void AModularAbilityActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, TransientAbilitySystem, Params);
}

void AModularAbilityActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	if ((CreationPolicy == EModularAbilitySystemCreationPolicy::Always) &&
		(GetNetMode() != NM_Client))
	{
		check(!AbilitySystemComponent);
		CreateAbilitySystemComponent();
		InitializeAbilitySystem();
		ForceNetUpdate();
	}
}

void AModularAbilityActor::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if ((TransientAbilitySystem == nullptr) &&
		(AbilitySystemComponent) &&
		(CreationPolicy != EModularAbilitySystemCreationPolicy::Never))
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, TransientAbilitySystem, this);
		TransientAbilitySystem = AbilitySystemComponent;
	}
}

void AModularAbilityActor::InitializeAbilitySystem()
{
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	OnAbilitySystemInitialized();
}

void AModularAbilityActor::SetPendingAttributeFromReplication(const FGameplayAttribute& Attribute, const FGameplayAttributeData& NewValue)
{
	PendingAttributeReplications.Emplace(FPendingAttributeReplication(Attribute, NewValue));
}

void AModularAbilityActor::ApplyPendingAttributeReplications()
{
	check(AbilitySystemComponent);

	// If there are no pending attribute replications, we don't need to do anything
	if (PendingAttributeReplications.Num() <= 0)
	{
		return;
	}

	for (const auto& Pending : PendingAttributeReplications)
	{
		AbilitySystemComponent->DeferredSetBaseAttributeValueFromReplication(Pending.Attribute, Pending.NewValue);
	}

	PendingAttributeReplications.Empty();
}

void AModularAbilityActor::CreateAbilitySystemComponent()
{
	AbilitySystemComponent = NewObject<UModularAbilitySystemComponent>(this, AbilitySystemClass);
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AbilitySystemComponent->RegisterComponent();
}

void AModularAbilityActor::OnRep_TransientAbilitySystem()
{
	AbilitySystemComponent = TransientAbilitySystem;
	if (AbilitySystemComponent)
	{
		InitializeAbilitySystem();
		ApplyPendingAttributeReplications();
	}
}

UAbilitySystemComponent* AModularAbilityActor::GetAbilitySystemComponent() const
{
	if ((AbilitySystemComponent == nullptr) &&
		HasAuthority() &&
		(CreationPolicy == EModularAbilitySystemCreationPolicy::Lazy) &&
		GetWorld() &&
		!IsUnreachable())
	{
		AModularAbilityActor* MutableThis = const_cast<AModularAbilityActor*>(this);
		MutableThis->CreateAbilitySystemComponent();
		MutableThis->InitializeAbilitySystem();
		MutableThis->ForceNetUpdate();
	}

	return AbilitySystemComponent;
}
