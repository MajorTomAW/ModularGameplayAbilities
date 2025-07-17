// Author: Tom Werner (MajorT), 2025


#include "Abilities/TargetData/Reticles/GameplayAbilityWorldReticle_GhostActor.h"

#include "Abilities/GameplayAbilityTargetActor.h"
#include "Components/CapsuleComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayAbilityWorldReticle_GhostActor)

AGameplayAbilityWorldReticle_GhostActor::AGameplayAbilityWorldReticle_GhostActor(
	const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{
	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionCapsule0"));
	CollisionComponent->InitCapsuleSize(0.f, 0.f);
	CollisionComponent->AlwaysLoadOnClient = true;
	CollisionComponent->SetUsingAbsoluteScale(true);
	//CollisionComponent->AlwaysLoadOnServer = true;
	CollisionComponent->SetCanEverAffectNavigation(false);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//USceneComponent* SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponent0"));
	RootComponent = CollisionComponent;
}

void AGameplayAbilityWorldReticle_GhostActor::InitGhostActor(
	AGameplayAbilityTargetActor* InTargetActor,
	AActor* GhostActor,
	UMaterialInterface* GhostMaterial)
{
	if (GhostActor == nullptr)
	{
		return;
	}

	// Get components
	TInlineComponentArray<UMeshComponent*> MeshComps;
	GhostActor->GetComponents(MeshComps);

	USceneComponent* MyRoot = GetRootComponent();
	check(MyRoot);

	TargetingActor = InTargetActor;
	
	//We want the reticle to tick after the targeting actor so that designers have the final say on the position
	AddTickPrerequisiteActor(TargetingActor);

	for (UMeshComponent* MeshComp : MeshComps)
	{
		// Special case:
		// If we don't clear the root component explicitly,
		// the component will be destroyed along with the original visualization actor.
		if (MeshComp == GhostActor->GetRootComponent())
		{
			GhostActor->SetRootComponent(nullptr);
		}

		// Disable collision on visualization mesh parts so it doesn't interfere with aiming or any other client-side collision/prediction/physics stuff
		//All mesh components are primitive components, so no cast is needed
		MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);		

		//Move components from one actor to the other, attaching as needed.
		// Hierarchy shouldn't be important, but we can do fixups if it becomes important later.
		MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		MeshComp->AttachToComponent(MyRoot, FAttachmentTransformRules::KeepRelativeTransform);
		MeshComp->Rename(nullptr, this);
		if (GhostMaterial)
		{
			MeshComp->SetMaterial(0, GhostMaterial);
		}
	}
}

void AGameplayAbilityWorldReticle_GhostActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}
