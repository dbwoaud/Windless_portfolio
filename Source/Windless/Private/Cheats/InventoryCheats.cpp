#include "Cheats/InventoryCheats.h"
#include "Components/InventoryComponent.h"
#include "Components/RepairComponent.h"
#include "Components/CraftingComponent.h"
#include "Items/WorldItem.h"
#include "Items/Settings/WindlessItemSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Windless/DebugMacro.h"

void UInventoryCheats::InvAdd(const FString& itemId, int32 count)
{
	if (UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>())
		inventory->AddItemById(FName(*itemId), count);
}

void UInventoryCheats::InvUse(const FString& itemId)
{
	UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>();
	if (!inventory)
		return;

	const int64 uid = inventory->FindFirstUnequippedUID(FName(*itemId));
	if (uid != 0)
		inventory->UseItem(uid);
}

void UInventoryCheats::InvSplit(const FString& itemId, int32 amount)
{
	UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>();
	if (!inventory)
		return;

	const int64 uid = inventory->FindFirstUnequippedUID(FName(*itemId));
	if (uid != 0)
		inventory->SplitStackAuto(uid, amount);
}

void UInventoryCheats::InvDrop(const FString& itemId, int32 quantity)
{
	UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>();
	if (!inventory)
		return;

	const int64 uid = inventory->FindFirstUnequippedUID(FName(*itemId));
	if (uid != 0)
		inventory->DropItem(uid, quantity);
}

void UInventoryCheats::InvMove(int32 uid, int32 x, int32 y, bool bRot)
{
	if (UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>())
		inventory->MoveItem((int64)uid, x, y, bRot);
}

void UInventoryCheats::InvSpawnWorld(const FString& itemId, int32 count)
{
	APawn* pawn = GetPlayerPawn();
	UWorld* world = GetWorld();
	if (!pawn || !world)
		return;

	const TSoftClassPtr<AWorldItem>& classPath = UWindlessItemSettings::Get()->debugWorldItemClass;
	UClass* worldItemClass = classPath.LoadSynchronous();
	if (!worldItemClass)
		return;

	FVector spawnLocation = pawn->GetActorLocation() + pawn->GetActorForwardVector() * 200.f;
	FHitResult groundHit;
	FCollisionQueryParams traceParams(SCENE_QUERY_STAT(InvSpawnWorld), false, pawn);
	if (world->LineTraceSingleByChannel(groundHit, spawnLocation + FVector(0.f, 0.f, 100.f), spawnLocation - FVector(0.f, 0.f, 500.f), ECC_Visibility, traceParams))
		spawnLocation = groundHit.ImpactPoint;

	const FTransform spawnTransform(FRotator::ZeroRotator, spawnLocation);

	AWorldItem* spawned = world->SpawnActorDeferred<AWorldItem>(worldItemClass, spawnTransform, pawn, pawn, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!spawned)
		return;

	const int32 spawnCount = FMath::Max(1, count);
	spawned->InitializeItem(FName(*itemId), spawnCount, -1.f);
	UGameplayStatics::FinishSpawningActor(spawned, spawnTransform);

	UE_LOG(LogWindless, Log, TEXT("[치트] 필드 아이템 생성: %s x%d (%s) 위치 %s"),
		*itemId, spawnCount, *spawned->GetName(), *spawnLocation.ToCompactString());
}

void UInventoryCheats::InvEquip(const FString& itemId, int32 slot)
{
	UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>();
	if (!inventory)
		return;

	const int64 uid = inventory->FindFirstUnequippedUID(FName(*itemId));
	if (uid != 0)
		inventory->EquipItem(uid, (EEquipSlot)slot);
}

void UInventoryCheats::InvUnequip(int32 slot)
{
	if (UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>())
		inventory->UnequipSlot((EEquipSlot)slot);
}

void UInventoryCheats::InvDur(int32 slot, float amount)
{
	if (UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>())
		inventory->DamageEquippedDurability((EEquipSlot)slot, amount);
}

void UInventoryCheats::InvRepair(int32 slot, float amount)
{
	UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>();
	if (!inventory)
		return;

	const int64 uid = inventory->GetEquippedInstanceUID((EEquipSlot)slot);
	if (uid != 0)
		inventory->RepairItem(uid, amount);
}

void UInventoryCheats::InvPrint()
{
	UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>();
	if (!inventory)
		return;

	inventory->PrintInventoryDebug();
	inventory->PrintEquippedDebug();
	inventory->PrintGridDebug();
}

void UInventoryCheats::InvLimit()
{
	if (UInventoryComponent* inventory = GetPlayerComponent<UInventoryComponent>())
		inventory->PrintCarryLimitDebug();
}

void UInventoryCheats::WLRepairInfo()
{
	const URepairComponent* repair = GetPlayerComponent<URepairComponent>();
	if (!repair)
		return;

	UE_LOG(LogWindless, Warning, TEXT("===== 수리도구 ====="));
	for (const FRepairToolEntry& tool : repair->GetRepairTools())
		UE_LOG(LogWindless, Warning, TEXT("%-16s 충전량 %.0f / %.0f"),
			*tool.toolName.ToString(), tool.currentCharge, tool.maxCharge);

	UE_LOG(LogWindless, Warning, TEXT("===== 수리 대상 ====="));
	for (const FRepairTargetEntry& target : repair->GetRepairTargets())
		UE_LOG(LogWindless, Warning, TEXT("[슬롯%d] %-16s 내구도 %.0f / %.0f (필요 %.0f)%s"),
			(int32)target.slot, *target.equipmentName.ToString(),
			target.currentDurability, target.maxDurability, target.requiredCharge,
			target.bBroken ? TEXT(" [파괴]") : TEXT(""));
}

void UInventoryCheats::WLCraftInfo()
{
	if (const UCraftingComponent* crafting = GetPlayerComponent<UCraftingComponent>())
		crafting->PrintDebugInfo();
}