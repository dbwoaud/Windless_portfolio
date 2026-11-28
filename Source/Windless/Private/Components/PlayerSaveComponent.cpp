#include "Components/PlayerSaveComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/PlayerProgressionComponent.h"
#include "Save/WindlessSaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

UPlayerSaveComponent::UPlayerSaveComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerSaveComponent::CollectSaveData(FInventorySaveRecord& outInventory, FPlayerStatSaveRecord& outStats) const
{
	const AActor* owner = GetOwner();
	if (!owner)
		return;

	if (const UInventoryComponent* inventory = owner->FindComponentByClass<UInventoryComponent>())
		inventory->SaveToRecord(outInventory);

	if (const UAttributeComponent* attributes = owner->FindComponentByClass<UAttributeComponent>())
		attributes->SaveToRecord(outStats);
}

void UPlayerSaveComponent::ApplySaveData(const FInventorySaveRecord& inventoryRecord, const FPlayerStatSaveRecord& statRecord)
{
	AActor* owner = GetOwner();
	if (!owner)
		return;

	UAttributeComponent* attributes = owner->FindComponentByClass<UAttributeComponent>();
	UInventoryComponent* inventory = owner->FindComponentByClass<UInventoryComponent>();
	UBodyPartComponent* bodyParts = owner->FindComponentByClass<UBodyPartComponent>();
	UPlayerProgressionComponent* progression = owner->FindComponentByClass<UPlayerProgressionComponent>();

	if (attributes)
		attributes->LoadFromRecord(statRecord);

	if (progression)
		progression->RefreshInventorySizeForLevel();

	if (inventory)
		inventory->LoadFromRecord(inventoryRecord);

	if (progression)
		progression->ApplyLevelToAllStats();

	if (bodyParts)
		bodyParts->HealAllParts();

	if (bTeleportToSavePointOnLoad)
		TeleportToLoadedSavePoint();

	onSaveApplied.Broadcast();
}

bool UPlayerSaveComponent::RequestSave(FName savePointId, FVector respawnLocation, FRotator respawnRotation)
{
	UWindlessSaveSubsystem* saveSubsystem = GetSaveSubsystem();
	return saveSubsystem && saveSubsystem->SaveGameData(GetOwner(), savePointId, respawnLocation, respawnRotation);
}

void UPlayerSaveComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoLoadOnStart)
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UPlayerSaveComponent::HandleAutoLoad);
}

void UPlayerSaveComponent::HandleAutoLoad()
{
	if (UWindlessSaveSubsystem* saveSubsystem = GetSaveSubsystem())
		saveSubsystem->TryContinue(GetOwner());
}

void UPlayerSaveComponent::TeleportToLoadedSavePoint()
{
	UWindlessSaveSubsystem* saveSubsystem = GetSaveSubsystem();
	AActor* owner = GetOwner();
	if (!saveSubsystem || !owner)
		return;

	FVector location = FVector::ZeroVector;
	FRotator rotation = FRotator::ZeroRotator;
	if (saveSubsystem->GetLoadedRespawnTransform(location, rotation))
		owner->SetActorLocationAndRotation(location, rotation, false, nullptr, ETeleportType::TeleportPhysics);
}

UWindlessSaveSubsystem* UPlayerSaveComponent::GetSaveSubsystem() const
{
	const UGameInstance* gameInstance = GetOwner() ? GetOwner()->GetGameInstance() : nullptr;
	return gameInstance ? gameInstance->GetSubsystem<UWindlessSaveSubsystem>() : nullptr;
}