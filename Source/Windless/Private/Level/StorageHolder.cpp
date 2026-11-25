#include "Level/StorageHolder.h"
#include "Components/InventoryComponent.h"
#include "Level/StorageTypes.h"

AStorageHolder::AStorageHolder()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetReplicates(false);

	storageInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("StorageInventory"));
}