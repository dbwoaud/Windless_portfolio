#include "Characters/Notifies/DropItemSpawnNotify.h"
#include "Components/ItemDropComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UDropItemSpawnNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (UItemDropComponent* itemDrop = owner ? owner->FindComponentByClass<UItemDropComponent>() : nullptr)
		itemDrop->SpawnPendingDrop();
}

FString UDropItemSpawnNotify::GetNotifyName_Implementation() const
{
	return TEXT("Drop Item Spawn");
}