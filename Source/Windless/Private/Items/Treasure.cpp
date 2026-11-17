#include "Items/Treasure.h"
#include "Interfaces/PickupInterface.h"

void ATreasure::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* pickupInterface = Cast<IPickupInterface>(OtherActor);
	if (pickupInterface)
	{
		pickupInterface->AddGold(this);
		SpawnPickupSound();
		Destroy();
	}
}