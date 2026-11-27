#include "Components/ItemDropComponent.h"
#include "Components/InventoryComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Items/DropItem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UItemDropComponent::UItemDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UItemDropComponent::SpawnPendingDrop()
{
	UWorld* world = GetWorld();
	if (world)
		world->GetTimerManager().ClearTimer(dropFallbackTimer);

	if (!pendingDropInstance.IsValid())
		return;

	const FItemInstance dropInstance = pendingDropInstance;
	pendingDropInstance = FItemInstance();

	AActor* owner = GetOwner();
	APawn* instigator = Cast<APawn>(owner);
	if (!world || !owner || !dropItemClass)
		return;

	const FVector forward = owner->GetActorForwardVector();
	const FVector spawnLocation = owner->GetActorLocation() + forward * dropForwardDistance + FVector(0.f, 0.f, dropSpawnHeightOffset);
	const FTransform spawnTransform(owner->GetActorRotation(), spawnLocation);

	ADropItem* drop = world->SpawnActorDeferred<ADropItem>(dropItemClass, spawnTransform, owner, instigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!drop)
		return;

	drop->InitializeItem(dropInstance.itemId, dropInstance.currentStackCount, dropInstance.currentDurability);
	UGameplayStatics::FinishSpawningActor(drop, spawnTransform);
	drop->DropItem(forward);
}

void UItemDropComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UInventoryComponent* inventory = GetOwner() ? GetOwner()->FindComponentByClass<UInventoryComponent>() : nullptr)
		inventory->OnItemDropped.AddDynamic(this, &UItemDropComponent::HandleItemDropped);
}

void UItemDropComponent::HandleItemDropped(FItemInstance droppedInstance)
{
	pendingDropInstance = droppedInstance;

	const ACharacter* character = Cast<ACharacter>(GetOwner());
	UAnimInstance* animInstance = (character && character->GetMesh()) ? character->GetMesh()->GetAnimInstance() : nullptr;

	if (dropMontage && animInstance && CanPlayDropMontage())
	{
		animInstance->Montage_Play(dropMontage);
		GetWorld()->GetTimerManager().SetTimer(dropFallbackTimer, this, &UItemDropComponent::SpawnPendingDrop, dropFallbackDelay, false);
		return;
	}

	SpawnPendingDrop();
}

bool UItemDropComponent::CanPlayDropMontage() const
{
	return !canPlayDropMontage.IsBound() || canPlayDropMontage.Execute();
}