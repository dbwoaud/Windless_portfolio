#include "Components/InteractionSphereComponent.h"
#include "Components/PlayerInteractionComponent.h"
#include "Interfaces/InteractableInterface.h"

UInteractionSphereComponent::UInteractionSphereComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	InitSphereRadius(180.f);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SetGenerateOverlapEvents(true);
}

void UInteractionSphereComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UInteractionSphereComponent::OnSphereBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UInteractionSphereComponent::OnSphereEndOverlap);
}

void UInteractionSphereComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UPlayerInteractionComponent* interaction = OtherActor ? OtherActor->FindComponentByClass<UPlayerInteractionComponent>() : nullptr;
	if (!interaction)
		return;

	AActor* owner = GetOwner();
	if (!owner || !owner->Implements<UInteractableInterface>())
		return;

	interaction->RegisterCandidate(owner);
}

void UInteractionSphereComponent::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UPlayerInteractionComponent* interaction = OtherActor ? OtherActor->FindComponentByClass<UPlayerInteractionComponent>() : nullptr)
		interaction->UnregisterCandidate(GetOwner());
}