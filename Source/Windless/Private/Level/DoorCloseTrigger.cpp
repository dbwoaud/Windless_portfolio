#include "Level/DoorCloseTrigger.h"
#include "Level/BaseDoor.h"

void ADoorCloseTrigger::BeginPlay()
{
	Super::BeginPlay();
	onTriggerEnter.AddDynamic(this, &ADoorCloseTrigger::HandleClose);
}

void ADoorCloseTrigger::HandleClose(AActor* activator)
{
	if (targetDoor)
		targetDoor->CloseDoor();
}