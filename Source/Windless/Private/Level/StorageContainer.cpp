#include "Level/StorageContainer.h"
#include "Characters/WindlessCharacter.h"
#include "Components/InventoryComponent.h"
#include "Components/InteractionSphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Windless/DebugMacro.h"
#include "Items/Storage/SharedStorageSubsystem.h"

AStorageContainer::AStorageContainer()
{
	PrimaryActorTick.bCanEverTick = false;

	mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(mesh);

	interactionSphere = CreateDefaultSubobject<UInteractionSphereComponent>(TEXT("InteractionSphere"));
	interactionSphere->SetupAttachment(GetRootComponent());
	interactionSphere->InitSphereRadius(200.f);

	storageInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("StorageInventory"));
}

void AStorageContainer::Interact_Implementation(AActor* interactor)
{
	AWindlessCharacter* player = Cast<AWindlessCharacter>(interactor);
	if (!player || !storageInventory)
		return;

	player->OpenStorage(this);
}

FText AStorageContainer::GetInteractionText_Implementation()
{
	return FText::FromString(FString::Printf(TEXT("%s  %s"), *interactionText.ToString(), *containerName.ToString()));
}

bool AStorageContainer::CanInteract_Implementation(AActor* interactor)
{
	return true;
}

void AStorageContainer::BeginPlay()
{
	Super::BeginPlay();	
}