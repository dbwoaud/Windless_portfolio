#include "Items/Storage/SharedStorageSubsystem.h"
#include "Level/StorageHolder.h"
#include "Components/InventoryComponent.h"
#include "Windless/DebugMacro.h"
#include "Engine/World.h"

UInventoryComponent* USharedStorageSubsystem::GetStorageInventory()
{
	EnsureHolder();
	return holder ? holder->GetInventory() : nullptr;
}

bool USharedStorageSubsystem::UpgradeTo(EStorageGrade newGrade)
{
	if (static_cast<int32>(newGrade) != static_cast<int32>(currentGrade) + 1)
		return false;

	currentGrade = newGrade;
	EnsureHolder();
	ApplyGradeToGrid();

	UE_LOG(LogWindless, Log, TEXT("[창고] 등급 상승 -> %s (%dx%d)"),
		*GetStorageGradeName(currentGrade).ToString(), StorageGridWidth, GetStorageGradeHeight(currentGrade));

	onStorageGradeChanged.Broadcast(currentGrade);
	return true;
}

void USharedStorageSubsystem::CollectRecord(FStorageSaveRecord& outRecord)
{
	outRecord.grade = currentGrade;

	if (const UInventoryComponent* inventory = GetStorageInventory())
		inventory->SaveToRecord(outRecord.inventory);
}

void USharedStorageSubsystem::ApplyRecord(const FStorageSaveRecord& record)
{
	currentGrade = record.grade;
	EnsureHolder();
	ApplyGradeToGrid();

	if (UInventoryComponent* inventory = GetStorageInventory())
		inventory->LoadFromRecord(record.inventory);

	onStorageGradeChanged.Broadcast(currentGrade);
}

void USharedStorageSubsystem::ResetStorage()
{
	currentGrade = EStorageGrade::ESG_Small;

	if (IsValid(holder))
	{
		holder->Destroy();
		holder = nullptr;
	}
	EnsureHolder();
}

void USharedStorageSubsystem::PrintDebugInfo()
{
	UInventoryComponent* inventory = GetStorageInventory();
	UE_LOG(LogWindless, Warning, TEXT("===== [창고] %s %dx%d ====="),
		*GetStorageGradeName(currentGrade).ToString(),
		inventory ? inventory->GetGridWidth() : 0,
		inventory ? inventory->GetGridHeight() : 0);

	if (inventory)
		inventory->PrintInventoryDebug();
}

bool USharedStorageSubsystem::DoesSupportWorldType(const EWorldType::Type worldType) const
{
	return worldType == EWorldType::Game || worldType == EWorldType::PIE;
}

void USharedStorageSubsystem::Deinitialize()
{
	if (IsValid(holder))
		holder->Destroy();

	holder = nullptr;
	Super::Deinitialize();
}

void USharedStorageSubsystem::EnsureHolder()
{
	if (IsValid(holder))
		return;

	UWorld* world = GetWorld();
	if (!world)
		return;

	FActorSpawnParameters params;
	params.Name = TEXT("SharedStorageHolder");
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.ObjectFlags |= RF_Transient;

	holder = world->SpawnActor<AStorageHolder>(AStorageHolder::StaticClass(), FTransform::Identity, params);
	if (!holder)
		return;

	ApplyGradeToGrid();
	UE_LOG(LogWindless, Log, TEXT("[창고] 통합 창고 생성 (%s %dx%d)"),
		*GetStorageGradeName(currentGrade).ToString(), StorageGridWidth, GetStorageGradeHeight(currentGrade));
}

void USharedStorageSubsystem::ApplyGradeToGrid()
{
	if (UInventoryComponent* inventory = holder ? holder->GetInventory() : nullptr)
		inventory->ResizeGrid(StorageGridWidth, GetStorageGradeHeight(currentGrade));
}
