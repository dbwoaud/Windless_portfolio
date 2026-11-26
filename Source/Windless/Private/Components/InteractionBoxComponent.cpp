#include "Components/InteractionBoxComponent.h"
#include "Components/PlayerInteractionComponent.h"
#include "Interfaces/InteractableInterface.h"

UInteractionBoxComponent::UInteractionBoxComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	InitBoxExtent(FVector(50.f));
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SetGenerateOverlapEvents(true);
}

void UInteractionBoxComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UInteractionBoxComponent::OnBoxBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UInteractionBoxComponent::OnBoxEndOverlap);
}

void UInteractionBoxComponent::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UPlayerInteractionComponent* interaction = OtherActor ? OtherActor->FindComponentByClass<UPlayerInteractionComponent>() : nullptr;
	if (!interaction)
		return;

	AActor* owner = GetOwner();
	if (!owner || !owner->Implements<UInteractableInterface>())
		return;

	interaction->RegisterCandidate(owner);
}

void UInteractionBoxComponent::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (UPlayerInteractionComponent* interaction = OtherActor ? OtherActor->FindComponentByClass<UPlayerInteractionComponent>() : nullptr)
		interaction->UnregisterCandidate(GetOwner());
}
