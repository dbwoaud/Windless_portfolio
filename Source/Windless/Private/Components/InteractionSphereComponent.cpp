#include "Components/InteractionSphereComponent.h"
#include "Windless/DebugMacro.h"
#include "Characters/WindlessCharacter.h"
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
	AWindlessCharacter* player = Cast<AWindlessCharacter>(OtherActor);
	if (!player)
		return;

	AActor* owner = GetOwner();
	if (!owner || !owner->Implements<UInteractableInterface>()) // 소유 액터가 인터페이스 미구현이면 무시
		return;

	player->RegisterInteractable(owner);
}

void UInteractionSphereComponent::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AWindlessCharacter* player = Cast<AWindlessCharacter>(OtherActor);
	if (!player)
		return;

	player->UnregisterInteractable(GetOwner());
}