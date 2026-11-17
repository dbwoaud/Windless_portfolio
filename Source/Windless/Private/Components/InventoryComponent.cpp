#include "Components/InventoryComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/RepairComponent.h"
#include "Level/ChaosZoneVolume.h"
#include "Items/Database/ItemDatabaseSubsystem.h"
#include "Windless/DebugMacro.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddItemById(FName itemId, int32 quantity)
{
    FItemData data;
    if (!GetItemData(itemId, data) || quantity <= 0)
        return false;

    FItemInstance src;
    src.itemId = itemId;
    src.currentStackCount = quantity;
    src.currentDurability = -1.f; 

    int32 added = 0;
    return AddItemInstance(src, added);
}

bool UInventoryComponent::AddItem(const FItemData& data, int32 quantity)
{
    if (!data.IsValid() || quantity <= 0)
        return false;

    if (itemDatabase)
        itemDatabase->RegisterItem(data);

    FItemInstance src;
    src.itemId = data.itemId;
    src.currentStackCount = quantity;
    src.currentDurability = -1.f;

    int32 added = 0;
    return AddItemInstance(src, added);
}

bool UInventoryComponent::AddItemInstance(const FItemInstance& srcInstance, int32& outAddedCount)
{
    outAddedCount = 0;

    FItemData data;
    if (!GetItemData(srcInstance.itemId, data))
        return false;

    /* 소지 한도 연산 */
    const int32 requested = FMath::Max(1, srcInstance.currentStackCount);
    const int32 carrySpace = GetRemainingCarrySpace(data.itemId);
    if (carrySpace <= 0) // 소지 아이템 한도 도달
    {
        NotifyCarryLimitBlocked(data, 0);
        return false;
    }

    int32 remaining = FMath::Min(requested, carrySpace);
    const bool bClampedByCarryLimit = (remaining < requested);

    const float srcDurability = (srcInstance.currentDurability < 0.f) ? data.maxDurability : srcInstance.currentDurability;

    /* 중첩 처리 연산 */
    if (data.bStackable)
    {
        int32 stackIdx = FindStackableIndex(data.itemId, data.maxStack);
        while (remaining > 0 && stackIdx != INDEX_NONE)
        {
            const int32 space = data.maxStack - items[stackIdx].currentStackCount;
            const int32 moved = FMath::Min(space, remaining);

            items[stackIdx].currentStackCount += moved;
            remaining -= moved;
            outAddedCount += moved;

            stackIdx = (remaining > 0) ? FindStackableIndex(data.itemId, data.maxStack) : INDEX_NONE;
        }
    }

    /* 배치 연산 */
    while (remaining > 0)
    {
        FItemInstance inst;
        inst.instanceUID = GenerateInstanceUID();
        inst.itemId = data.itemId;
        inst.currentDurability = srcDurability;
        inst.currentStackCount = data.bStackable ? FMath::Min(data.maxStack, remaining) : 1;

        if (!TryAutoPlace(inst)) // 인벤토리 공간 부족 시 부분 획득
        {
            if (outAddedCount > 0)
                OnInventoryChanged.Broadcast();
            return false;
        }

        items.Add(inst);
        remaining -= inst.currentStackCount;
        outAddedCount += inst.currentStackCount;
    }

    OnInventoryChanged.Broadcast();

    if (IsRecoveryData(data))
        TryAutoEquipRecoveryItem(data.itemId);

    if (bClampedByCarryLimit)
        NotifyCarryLimitBlocked(data, outAddedCount);

    return !bClampedByCarryLimit;
}

bool UInventoryComponent::AddStackToInstance(int64 instanceUID, int32 amount)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE || amount <= 0)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data) || !data.bStackable)
        return false;

    if (items[idx].currentStackCount + amount > data.maxStack)
        return false;

    items[idx].currentStackCount += amount;
    OnInventoryChanged.Broadcast();

    if (items[idx].quickSlot != EQuickSlot::EQS_None)
        OnQuickSlotChanged.Broadcast(items[idx].quickSlot);

    return true;
}

bool UInventoryComponent::RemoveInstance(int64 instanceUID, int32 quantity)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE || quantity <= 0)
        return false;

 

    items[idx].currentStackCount -= quantity;

    if (items[idx].currentStackCount <= 0) 
    {
        RemoveInstanceAt(idx);
        return true;
    }

    OnInventoryChanged.Broadcast();

    const EQuickSlot affectedSlot = items[idx].quickSlot;
    if (affectedSlot != EQuickSlot::EQS_None)
        OnQuickSlotChanged.Broadcast(affectedSlot);

    return true;
}

void UInventoryComponent::RemoveInstanceAt(int32 index)
{
    if (!items.IsValidIndex(index))
        return;

    const bool bWasEquipped = items[index].bIsEquipped;
    const EQuickSlot affectedSlot = items[index].quickSlot;

    FreeForInstance(items[index]); // 점유 셀 해제
    items.RemoveAt(index);

    if (bWasEquipped) // 장착 중이던 아이템이면 보정치 재계산 및 장착 현황 갱신
    {
        RecalculateEquipmentStats();
        OnEquipmentChanged.Broadcast();
    }

    if (affectedSlot != EQuickSlot::EQS_None)
        OnQuickSlotChanged.Broadcast(affectedSlot);

    OnInventoryChanged.Broadcast();
}

int32 UInventoryComponent::RemoveItemById(FName itemId, int32 quantity)
{
    if (itemId.IsNone() || quantity <= 0)
        return 0;

    TArray<int64> candidates;
    for (const FItemInstance& inst : items)
    {
        if (inst.itemId == itemId && !inst.bIsEquipped)
            candidates.Add(inst.instanceUID);
    }

    candidates.Sort([this](const int64& a, const int64& b)
        {
            return FindInstance(a).currentStackCount < FindInstance(b).currentStackCount;
        });

    int32 removed = 0;
    for (const int64 uid : candidates)
    {
        if (removed >= quantity)
            break;

        const int32 stack = FMath::Max(1, FindInstance(uid).currentStackCount);
        const int32 take = FMath::Min(stack, quantity - removed);

        if (RemoveInstance(uid, take))
            removed += take;
    }

    return removed;
}

bool UInventoryComponent::UseItem(int64 instanceUID)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return false;

    if (data.category != EItemCategory::EIC_Consumable)
        return false;

    if (data.effectType == EItemEffect::EIE_Ammo)
        return false;

    if (!ApplyConsumableEffect(data, EBodyPart::EBP_None, instanceUID))
        return false;

    if (data.bConsumedOnUse)
        return RemoveInstance(instanceUID, 1);

    OnInventoryChanged.Broadcast();
    return true;
}

float UInventoryComponent::GetItemCastTime(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return 0.f;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return 0.f;

    // 부서진 부위 치유(소드락)는 회복 아이템과 같은 시전 시간을 쓴다 (사용자 2026-11-17)
    FItemData recovery;
    if (data.effectType == EItemEffect::EIE_RevivePart && GetItemData(recoveryItemId, recovery))
        data.useTime = recovery.useTime;

    const float multiplier = ownerAttributes ? ownerAttributes->GetCastSpeedMultiplier() : 1.f;
    return data.useTime * multiplier;
}

float UInventoryComponent::GetItemMoveSpeedRate(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return 1.f;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return 1.f;

    return (data.moveSpeedRateOnUse > 0.f) ? data.moveSpeedRateOnUse : 1.f;
}

bool UInventoryComponent::UseItemOnPart(int64 instanceUID, EBodyPart targetPart)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE || targetPart == EBodyPart::EBP_None)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data) || data.category != EItemCategory::EIC_Consumable)
        return false;

    const bool bPartEffect =
        data.effectType == EItemEffect::EIE_HealHP ||
        data.effectType == EItemEffect::EIE_CureInjury ||
        data.effectType == EItemEffect::EIE_RevivePart;

    if (!bPartEffect)
        return UseItem(instanceUID);

    if (!ApplyConsumableEffect(data, targetPart, instanceUID))
        return false;

    if (data.bConsumedOnUse)
        return RemoveInstance(instanceUID, 1);

    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::CanUseItemOnPart(int64 instanceUID, EBodyPart targetPart) const
{
    if (targetPart == EBodyPart::EBP_None)
        return false;

    TArray<EBodyPart> usableParts;
    GetUsableBodyParts(instanceUID, usableParts);
    return usableParts.Contains(targetPart);
}

void UInventoryComponent::GetUsableBodyParts(int64 instanceUID, TArray<EBodyPart>& outParts) const
{
    outParts.Reset();

    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE || !ownerBodyParts)
        return;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data) || data.category != EItemCategory::EIC_Consumable)
        return;

    static const EBodyPart allParts[] = 
    {
        EBodyPart::EBP_Head,    EBodyPart::EBP_Chest,
        EBodyPart::EBP_LeftArm, EBodyPart::EBP_RightArm,
        EBodyPart::EBP_LeftLeg, EBodyPart::EBP_RightLeg 
    };

    for (const EBodyPart part : allParts)
    {
        switch (data.effectType)
        {
            case EItemEffect::EIE_CureInjury:
                if (ownerBodyParts->IsPartInjured(part, data.targetInjury))
                    outParts.Add(part);
                break;

            case EItemEffect::EIE_RevivePart:
                if (ownerBodyParts->IsPartDisabled(part))
                    outParts.Add(part);
                break;

            case EItemEffect::EIE_HealHP:
                if (!ownerBodyParts->IsPartDisabled(part) &&
                    ownerBodyParts->GetPartHealth(part) < ownerBodyParts->GetPartMaxHealth(part))
                    outParts.Add(part);
                break;

            default:
                break;
        }
    }
}

bool UInventoryComponent::RequiresBodyPartSelection(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return false;

    return data.category == EItemCategory::EIC_Consumable && data.bRequiresBodyPart;
}

bool UInventoryComponent::DropItem(int64 instanceUID, int32 quantity)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return false;

    if (!data.bDroppable) // 버리기 불가 아이템은 불가 안내
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("버릴 수 없는 아이템입니다."));
        return false;
    }

    if (items[idx].bIsEquipped) // 장착 중인 장비는 먼저 해제해야 함
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("[버리기 실패] 장착 중인 아이템입니다"));
        return false;
    }

    const int32 dropCount = (quantity < 0) ? items[idx].currentStackCount : FMath::Min(quantity, items[idx].currentStackCount);
    if (dropCount <= 0)
        return false;

    FItemInstance snapshot = items[idx];
    snapshot.currentStackCount = dropCount;
    snapshot.gridPosX = 0;
    snapshot.gridPosY = 0;
    snapshot.bIsRotated = false;
    snapshot.bIsEquipped = false;
    snapshot.equipSlot = EEquipSlot::EES_None;

    const bool bRemoved = RemoveInstance(instanceUID, dropCount);
    if (bRemoved)
        OnItemDropped.Broadcast(snapshot);

    return bRemoved;
}

bool UInventoryComponent::CanDropItem(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return false;

    return data.bDroppable;
}

bool UInventoryComponent::EquipItem(int64 instanceUID, EEquipSlot targetSlot)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return false;

    // 양손무기는 오른손 슬롯 기준으로 장착 
    const bool bTwoHanded = IsTwoHandedWeapon(data);
    EEquipSlot desiredSlot;

    if (bTwoHanded)
        desiredSlot = EEquipSlot::EES_RightHand;
    else if (targetSlot == EEquipSlot::EES_None)
        desiredSlot = ResolveAutoEquipSlot(data.equipSlot);
    else
        desiredSlot = targetSlot;

    if (!CanEquipToSlot(data.itemId, desiredSlot))
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange,
                FString::Printf(TEXT("[장착 불가] %s"), *data.itemName.ToString()));
        return false;
    }

    if (items[idx].bIsEquipped)
        return false;

    // 양손무기 충돌 슬롯 확인 및 캐릭터 전투 중 상태 확인 
    const EEquipSlot conflictSlot = FindTwoHandedConflictSlot(desiredSlot, bTwoHanded);
    if (!IsEquipSlotChangeAllowed(desiredSlot))
        return false;

    if (conflictSlot != EEquipSlot::EES_None && !IsEquipSlotChangeAllowed(conflictSlot))
        return false;

    if (conflictSlot != EEquipSlot::EES_None && !ReleaseEquippedToGrid(conflictSlot)) // 반대 손 비우기
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
                TEXT("[장착 실패] 반대 손 장비를 되돌릴 공간이 부족합니다"));
        return false;
    }

    const int32 prevIdx = FindEquippedIndex(desiredSlot);

    FreeForInstance(items[idx]);
    if (prevIdx != INDEX_NONE)
    {
        items[prevIdx].bIsEquipped = false;
        items[prevIdx].equipSlot = EEquipSlot::EES_None;

        if (!TryAutoPlace(items[prevIdx]))
        {
            items[prevIdx].bIsEquipped = true;
            items[prevIdx].equipSlot = desiredSlot;
            OccupyForInstance(items[idx]);

            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
                    TEXT("[장착 실패] 기존 장비를 되돌릴 공간이 부족합니다"));

            if (conflictSlot != EEquipSlot::EES_None)
            {
                RecalculateEquipmentStats();
                OnEquipmentChanged.Broadcast();
                OnInventoryChanged.Broadcast();
            }
            return false;
        }
    }

    items[idx].bIsEquipped = true;
    items[idx].equipSlot = desiredSlot;
    RecalculateEquipmentStats();

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
            FString::Printf(TEXT("[장착] %s -> 슬롯 %d (ATK+%.0f LimbHP+%.0f)"),
                *data.itemName.ToString(), (int32)desiredSlot, data.attackPowerBonus, data.maxLimbHPBonus));

    OnEquipmentChanged.Broadcast();
    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::CanEquipToSlot(FName itemId, EEquipSlot targetSlot) const
{
    FItemData data;
    if (!GetItemData(itemId, data) || data.category != EItemCategory::EIC_Equipment)
        return false;

    if (data.equipSlot == EEquipSlot::EES_None || targetSlot == EEquipSlot::EES_None)
        return false;

    return GetSlotGroup(data.equipSlot) == GetSlotGroup(targetSlot);
}

bool UInventoryComponent::UnequipSlot(EEquipSlot slot)
{
    const int32 idx = FindEquippedIndex(slot);
    if (idx == INDEX_NONE)
        return false;

    if (!IsEquipSlotChangeAllowed(slot))
        return false;

    const FName itemId = items[idx].itemId;
    if (!ReleaseEquippedToGrid(slot))
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red,
                TEXT("[해제 실패] 인벤토리 여유 공간 부족"));
        return false;
    }

    RecalculateEquipmentStats();
    if (GEngine)
    {
        FItemData data;
        GetItemData(itemId, data);
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::White,
            FString::Printf(TEXT("[해제] %s"), *data.itemName.ToString()));
    }

    OnEquipmentChanged.Broadcast();
    OnInventoryChanged.Broadcast();
    return true;
}

int64 UInventoryComponent::GetEquippedInstanceUID(EEquipSlot slot) const
{
    const int32 idx = FindEquippedIndex(slot);
    return items.IsValidIndex(idx) ? items[idx].instanceUID : 0;
}

EBodyPart UInventoryComponent::GetBodyPartForSlot(EEquipSlot slot) const
{
    return EquipSlotToBodyPart(slot);
}

EEquipSlot UInventoryComponent::GetOccupyingSlot(EEquipSlot slot) const
{
    if (!IsHandSlot(slot) || FindEquippedIndex(slot) != INDEX_NONE)
        return slot;

    const EEquipSlot pairedSlot = GetPairedSlot(slot);
    const int32 pairedIdx = FindEquippedIndex(pairedSlot);
    if (pairedIdx == INDEX_NONE)
        return slot;

    FItemData pairedData;
    if (GetItemData(items[pairedIdx].itemId, pairedData) && IsTwoHandedWeapon(pairedData))
        return pairedSlot;

    return slot;
}

float UInventoryComponent::GetHandAttackBonus(EEquipSlot handSlot) const
{
    if (!IsHandSlot(handSlot))
        return 0.f;

    const int32 idx = FindEquippedIndex(GetOccupyingSlot(handSlot));
    if (idx == INDEX_NONE)
        return 0.f;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return 0.f;

    if (data.maxDurability > 0.f && items[idx].currentDurability <= 0.f)
        return 0.f;

    return data.attackPowerBonus;
}

int32 UInventoryComponent::GetCarryLimit(FName itemId) const
{
    FItemData data;
    if (!GetItemData(itemId, data))
        return 0;

    return GetCarryLimitFromData(data);
}

int32 UInventoryComponent::GetItemTotalCount(FName itemId) const
{
    int32 total = 0;
    for (const FItemInstance& inst : items)
    {
        if (inst.itemId == itemId)
            total += inst.currentStackCount;
    }

    return total;
}

int32 UInventoryComponent::GetFreeCellCount() const
{
    int32 free = 0;
    for (const int32 cell : gridMap)
    {
        if (cell == 0)
            ++free;
    }
    return free;
}

int32 UInventoryComponent::GetRemainingCarrySpace(FName itemId) const
{
    const int32 limit = GetCarryLimit(itemId);
    if (limit <= 0)
        return MAX_int32;

    return FMath::Max(0, limit - GetItemTotalCount(itemId));
}

bool UInventoryComponent::IsCarryLimited(FName itemId) const
{
    return GetCarryLimit(itemId) > 0;
}

bool UInventoryComponent::IsCarryLimitReached(FName itemId) const
{
    return IsCarryLimited(itemId) && GetRemainingCarrySpace(itemId) <= 0;
}

void UInventoryComponent::DamageEquippedDurability(EEquipSlot slot, float amount)
{
    const int32 idx = FindEquippedIndex(slot);
    if (idx == INDEX_NONE)
        return;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data) || data.maxDurability <= 0.f)
        return;

    const bool bWasBroken = (items[idx].currentDurability <= 0.f);

    items[idx].currentDurability = FMath::Clamp(items[idx].currentDurability - amount, 0.f, data.maxDurability);
    if (!bWasBroken && items[idx].currentDurability <= 0.f)
    {
        RecalculateEquipmentStats();
        OnItemDestroyed.Broadcast(items[idx].itemId);
        OnEquipmentChanged.Broadcast();
        OnInventoryChanged.Broadcast();
        return;
    }

    OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::IsInstanceBroken(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data) || data.maxDurability <= 0.f)
        return false;

    return items[idx].currentDurability <= 0.f;
}

bool UInventoryComponent::RepairItem(int64 instanceUID, float amount)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data) || data.maxDurability <= 0.f)
        return false;

    if (items[idx].currentDurability >= data.maxDurability)
        return false;

    const bool bWasBroken = (items[idx].currentDurability <= 0.f);
    const float repairAmount = (amount < 0.f) ? data.maxDurability : amount;

    items[idx].currentDurability = FMath::Min(data.maxDurability, items[idx].currentDurability + repairAmount);
    if (bWasBroken && items[idx].currentDurability > 0.f)
    {
        RecalculateEquipmentStats();
        OnEquipmentChanged.Broadcast();
    }

    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::SetInstanceDurability(int64 instanceUID, float newValue)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data) || data.maxDurability <= 0.f)
        return false;

    const bool bWasBroken = (items[idx].currentDurability <= 0.f);
    items[idx].currentDurability = FMath::Clamp(newValue, 0.f, data.maxDurability);

    if (items[idx].equipSlot != EEquipSlot::EES_None &&
        bWasBroken != (items[idx].currentDurability <= 0.f))
    {
        RecalculateEquipmentStats();
        OnEquipmentChanged.Broadcast();
    }

    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::ResizeGrid(int32 newWidth, int32 newHeight)
{
    if (newWidth < 1 || newHeight < 1)
        return false;

    if (newWidth == gridWidth && newHeight == gridHeight)
        return true;

    // 축소 시 기존 아이템이 새 범위를 벗어나면 거부
    for (const FItemInstance& inst : items)
    {
        int32 w = 0, h = 0;
        GetInstanceFootprint(inst.instanceUID, inst.bIsRotated, w, h);
        if (inst.gridPosX + w > newWidth || inst.gridPosY + h > newHeight)
        {
            UE_LOG(LogWindless, Error, TEXT("[인벤토리] 그리드 축소 실패 - 아이템이 범위를 벗어남 (UID %lld)"), inst.instanceUID);
            return false;
        }
    }

    gridWidth = newWidth;
    gridHeight = newHeight;

    // 점유 맵을 새 크기로 다시 구성
    ResetGridMap();
    for (const FItemInstance& inst : items)
    {
        int32 w = 0, h = 0;
        GetInstanceFootprint(inst.instanceUID, inst.bIsRotated, w, h);
        SetCells(inst.gridPosX, inst.gridPosY, w, h, 1);
    }

    UE_LOG(LogWindless, Log, TEXT("[인벤토리] 그리드 크기 변경 -> %dx%d"), gridWidth, gridHeight);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::MoveItem(int64 instanceUID, int32 newX, int32 newY, bool bRotated)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    if (items[idx].bIsEquipped)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return false;

    FreeForInstance(items[idx]);

    int32 w, h;
    GetFootprint(data, bRotated, w, h);
    if (CanPlaceAt(newX, newY, w, h))
    {
        items[idx].gridPosX = newX;
        items[idx].gridPosY = newY;
        items[idx].bIsRotated = bRotated;
        SetCells(newX, newY, w, h, 1);
        OnInventoryChanged.Broadcast();
        return true;
    }
    OccupyForInstance(items[idx]);
    return false;
}

bool UInventoryComponent::SwapItems(int64 uidA, int64 uidB)
{
    const int32 idxA = FindInstanceIndex(uidA);
    const int32 idxB = FindInstanceIndex(uidB);
    if (idxA == INDEX_NONE || idxB == INDEX_NONE || idxA == idxB)
        return false;

    if (items[idxA].bIsEquipped || items[idxB].bIsEquipped)
        return false;

    FItemData dataA, dataB;
    if (!GetItemData(items[idxA].itemId, dataA) || !GetItemData(items[idxB].itemId, dataB))
        return false;

    const int32 ax = items[idxA].gridPosX, ay = items[idxA].gridPosY;
    const int32 bx = items[idxB].gridPosX, by = items[idxB].gridPosY;

    int32 aw, ah, bw, bh;
    GetFootprint(dataA, items[idxA].bIsRotated, aw, ah);
    GetFootprint(dataB, items[idxB].bIsRotated, bw, bh);

    FreeForInstance(items[idxA]);
    FreeForInstance(items[idxB]);

    if (CanPlaceAt(bx, by, aw, ah))
    {
        SetCells(bx, by, aw, ah, 1);
        if (CanPlaceAt(ax, ay, bw, bh))
        {
            SetCells(ax, ay, bw, bh, 1);
            items[idxA].gridPosX = bx;
            items[idxA].gridPosY = by;
            items[idxB].gridPosX = ax;
            items[idxB].gridPosY = ay;

            OnInventoryChanged.Broadcast();
            return true;
        }

        SetCells(bx, by, aw, ah, 0);
    }

    OccupyForInstance(items[idxA]);
    OccupyForInstance(items[idxB]);
    return false;
}

bool UInventoryComponent::MergeStacks(int64 sourceUID, int64 targetUID)
{
    const int32 srcIdx = FindInstanceIndex(sourceUID);
    const int32 tgtIdx = FindInstanceIndex(targetUID);
    if (srcIdx == INDEX_NONE || tgtIdx == INDEX_NONE || srcIdx == tgtIdx)
        return false;

    if (items[srcIdx].bIsEquipped || items[tgtIdx].bIsEquipped)
        return false;

    if (items[srcIdx].itemId != items[tgtIdx].itemId)
        return false;

    FItemData data;
    if (!GetItemData(items[srcIdx].itemId, data) || !data.bStackable)
        return false;

    const int32 space = data.maxStack - items[tgtIdx].currentStackCount;
    if (space <= 0) // 대상 아이템의 최대 보관 가능 개수가 이미 가득차면 무시
        return false;

    const int32 moved = FMath::Min(space, items[srcIdx].currentStackCount);
    items[tgtIdx].currentStackCount += moved;
    items[srcIdx].currentStackCount -= moved;

    if (items[srcIdx].currentStackCount <= 0)
        RemoveInstanceAt(srcIdx);
    else
        OnInventoryChanged.Broadcast();

    return true;
}

bool UInventoryComponent::TransferMergeTo(UInventoryComponent* target, int64 sourceUID, int64 targetUID)
{
    if (!target || target == this)
        return false;

    const int32 srcIdx = FindInstanceIndex(sourceUID);
    if (srcIdx == INDEX_NONE || items[srcIdx].bIsEquipped)
        return false;

    const FItemInstance targetInst = target->FindInstance(targetUID);
    if (!targetInst.IsValid() || targetInst.itemId != items[srcIdx].itemId)
        return false;

    FItemData data;
    if (!GetItemData(items[srcIdx].itemId, data) || !data.bStackable)
        return false;

    /* 대상 여유 공간 계산 */
    const int32 space = data.maxStack - targetInst.currentStackCount;
    if (space <= 0)
        return false;

    /* 대상 소지 한도 확인 */
    const int32 carrySpace = target->GetRemainingCarrySpace(data.itemId);
    const int32 moved = FMath::Min3(space, items[srcIdx].currentStackCount, carrySpace);
    if (moved <= 0)
        return false;

    if (!target->AddStackToInstance(targetUID, moved))
        return false;

    RemoveInstance(sourceUID, moved);
    return true;
}

bool UInventoryComponent::SplitStack(int64 instanceUID, int32 splitAmount, int32 newX, int32 newY, bool bRotated)
{
    if (!CanSplitStack(instanceUID, splitAmount))
        return false;

    const int32 idx = FindInstanceIndex(instanceUID);
    FItemData data;
    GetItemData(items[idx].itemId, data);

    int32 w, h;
    GetFootprint(data, bRotated, w, h);
    if (!CanPlaceAt(newX, newY, w, h))
        return false;

    items[idx].currentStackCount -= splitAmount;

    FItemInstance newInst;
    newInst.instanceUID = GenerateInstanceUID();
    newInst.itemId = items[idx].itemId;
    newInst.currentStackCount = splitAmount;
    newInst.currentDurability = items[idx].currentDurability;
    newInst.gridPosX = newX;
    newInst.gridPosY = newY;
    newInst.bIsRotated = bRotated;

    OccupyForInstance(newInst);
    items.Add(newInst);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::SplitStackAuto(int64 instanceUID, int32 splitAmount)
{
    if (!CanSplitStack(instanceUID, splitAmount))
        return false;

    const int32 idx = FindInstanceIndex(instanceUID);

    FItemData data;
    GetItemData(items[idx].itemId, data);

    FItemInstance newInst;
    newInst.instanceUID = GenerateInstanceUID();
    newInst.itemId = items[idx].itemId;
    newInst.currentStackCount = splitAmount;
    newInst.currentDurability = items[idx].currentDurability;

    if (!TryAutoPlace(newInst))
        return false;
    
    items[idx].currentStackCount -= splitAmount;
    items.Add(newInst);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::CanSplitStack(int64 instanceUID, int32 splitAmount) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    if (items[idx].bIsEquipped)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data) || !data.bStackable)
        return false;

    return splitAmount >= 1 && splitAmount < items[idx].currentStackCount;
}

bool UInventoryComponent::CanSplitStackAt(int64 instanceUID, int32 splitAmount, int32 x, int32 y, bool bRotated) const
{
    if (!IsSplittable(instanceUID))
        return false;

    const int32 idx = FindInstanceIndex(instanceUID);
    if (splitAmount <= 0 || splitAmount >= items[idx].currentStackCount)
        return false;

    return CanPlaceNewInstanceAt(items[idx].itemId, x, y, bRotated);
}

bool UInventoryComponent::CanPlaceNewInstanceAt(FName itemId, int32 x, int32 y, bool bRotated) const
{
    FItemData data;
    if (!GetItemData(itemId, data))
        return false;

    int32 w, h;
    GetFootprint(data, bRotated, w, h);
    return CanPlaceAt(x, y, w, h);
}

bool UInventoryComponent::IsSplittable(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE || items[idx].bIsEquipped)
        return false;

    if (items[idx].currentStackCount < 2)
        return false;

    FItemData data;
    return GetItemData(items[idx].itemId, data) && data.bStackable;
}

bool UInventoryComponent::RotateItem(int64 instanceUID)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    return MoveItem(instanceUID, items[idx].gridPosX, items[idx].gridPosY, !items[idx].bIsRotated);
}

bool UInventoryComponent::CanPlaceInstanceAt(int64 instanceUID, int32 x, int32 y, bool bRotated) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return false;

    int32 w, h;
    GetFootprint(data, bRotated, w, h);

    if (x < 0 || y < 0 || x + w > gridWidth || y + h > gridHeight)
        return false;

    for (int32 cy = y; cy < y + h; ++cy)
    {
        for (int32 cx = x; cx < x + w; ++cx)
        {
            if (gridMap[CellIndex(cx, cy)] == 0)
                continue;

            if (FindInstanceAtCell(cx, cy) != idx)
                return false;
        }
    }
    return true;
}

void UInventoryComponent::GetInstanceFootprint(int64 instanceUID, bool bRotated, int32& outW, int32& outH) const
{
    outW = 1;
    outH = 1;

    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return;

    FItemData data;
    if (GetItemData(items[idx].itemId, data))
        GetFootprint(data, bRotated, outW, outH);
}

bool UInventoryComponent::IsCellOccupied(int32 x, int32 y) const
{
    if (x < 0 || y < 0 || x >= gridWidth || y >= gridHeight)
        return true;

    return gridMap.IsValidIndex(CellIndex(x, y)) && gridMap[CellIndex(x, y)] != 0;
}

FItemInstance UInventoryComponent::FindInstance(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    return (idx != INDEX_NONE) ? items[idx] : FItemInstance();
}

int64 UInventoryComponent::FindInstanceUIDAtCell(int32 x, int32 y) const
{
    const int32 idx = FindInstanceAtCell(x, y);
    return items.IsValidIndex(idx) ? items[idx].instanceUID : 0;
}

FColor UInventoryComponent::GetGradeColor(EItemGrade grade) const
{
    return GetItemGradeColor(grade);
}

bool UInventoryComponent::IsItemRegistered(FName itemId) const
{
    const UItemDatabaseSubsystem* database = itemDatabase ? itemDatabase.Get() : UItemDatabaseSubsystem::Get(this);
    if (!database)
        return false;

    return database->HasItem(itemId);
}

bool UInventoryComponent::GetItemData(FName itemId, FItemData& outData) const
{
    const UItemDatabaseSubsystem* database = itemDatabase ? itemDatabase.Get() : UItemDatabaseSubsystem::Get(this);
    if (!database)
        return false;

    return database->GetItemData(itemId, outData);
}

bool UInventoryComponent::IsAmmoItem(FName itemId) const
{
    FItemData data;
    return GetItemData(itemId, data) && data.category == EItemCategory::EIC_Consumable && data.effectType == EItemEffect::EIE_Ammo;
}

void UInventoryComponent::SaveToRecord(FInventorySaveRecord& outRecord) const
{
    outRecord.items = items;
    outRecord.nextInstanceUID = nextInstanceUID;
    outRecord.gridWidth = gridWidth;
    outRecord.gridHeight = gridHeight;
}

void UInventoryComponent::LoadFromRecord(const FInventorySaveRecord& record)
{
    items.Empty();
    ResetGridMap();

    int64 maxUID = record.nextInstanceUID;
    for (const FItemInstance& inst : record.items)
        maxUID = FMath::Max(maxUID, inst.instanceUID + 1);
    
    nextInstanceUID = FMath::Max<int64>(1, maxUID);

    const bool bGridChanged = (record.gridWidth != gridWidth) || (record.gridHeight != gridHeight);
    if (bGridChanged && record.gridWidth > 0)
    {
        UE_LOG(LogWindless, Warning, TEXT("[세이브] 그리드 크기 변경 감지 (%dx%d -> %dx%d) - 자동 재배치 수행"),
           record.gridWidth, record.gridHeight, gridWidth, gridHeight);
    }

    int32 lostCount = 0;
    for (const FItemInstance& src : record.items)
    {
        FItemData data;
        if (!GetItemData(src.itemId, data))
        {
            ++lostCount;
            continue;
        }

        FItemInstance inst = src;

        if (inst.bIsEquipped || inst.quickSlot != EQuickSlot::EQS_None)
        {
            items.Add(inst);
            continue;
        }

        if (bGridChanged)
        {
            if (!TryAutoPlace(inst))
            {
                ++lostCount;
                continue;
            }
        }
        else
        {
            int32 w, h;
            GetFootprint(data, inst.bIsRotated, w, h);
            if (CanPlaceAt(inst.gridPosX, inst.gridPosY, w, h))
                SetCells(inst.gridPosX, inst.gridPosY, w, h, 1);

            else if (!TryAutoPlace(inst))
            {
                ++lostCount;
                continue;
            }
        }

        items.Add(inst);
    }

    RecalculateEquipmentStats();
    OnEquipmentChanged.Broadcast();
    BroadcastAllQuickSlots();
    OnInventoryChanged.Broadcast();
}

bool UInventoryComponent::TransferInstanceTo(UInventoryComponent* target, int64 instanceUID, int32 targetX, int32 targetY, bool bRotated)
{
    if (!target || target == this)
        return false;

    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    if (items[idx].bIsEquipped) // 장착 중인 아이템은 먼저 해제해야 함
    {
        if (GEngine)
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("[이동 실패] 장착 중인 아이템입니다"));
        return false;
    }

    const FItemInstance snapshot = items[idx];

    if (!target->CanAcceptInstanceAt(snapshot, targetX, targetY, bRotated))
        return false;

    if (!RemoveInstance(instanceUID, snapshot.currentStackCount))
        return false;

    if (!target->PlaceInstanceAt(snapshot, targetX, targetY, bRotated))
    {
        int32 restored = 0;
        AddItemInstance(snapshot, restored);
        return false;
    }

    return true;
}

bool UInventoryComponent::CanAcceptInstanceAt(const FItemInstance& inst, int32 x, int32 y, bool bRotated) const
{
    FItemData data;
    if (!GetItemData(inst.itemId, data))
        return false;

    /* 소지 한도 확인 */
    const int32 space = GetRemainingCarrySpace(inst.itemId);
    if (space < inst.currentStackCount)
        return false;

    int32 w, h;
    GetFootprint(data, bRotated, w, h);
    return CanPlaceAt(x, y, w, h);
}

bool UInventoryComponent::PlaceInstanceAt(const FItemInstance& inst, int32 x, int32 y, bool bRotated)
{
    if (!CanAcceptInstanceAt(inst, x, y, bRotated))
        return false;

    FItemInstance placed = inst;
    placed.instanceUID = GenerateInstanceUID();
    placed.gridPosX = x;
    placed.gridPosY = y;
    placed.bIsRotated = bRotated;
    placed.bIsEquipped = false;
    placed.equipSlot = EEquipSlot::EES_None;

    items.Add(placed);
    OccupyForInstance(placed);

    OnInventoryChanged.Broadcast();
    return true;
}

void UInventoryComponent::GetNewInstanceFootprint(FName itemId, bool bRotated, int32& outW, int32& outH) const
{
    outW = 1;
    outH = 1;

    FItemData data;
    if (GetItemData(itemId, data))
        GetFootprint(data, bRotated, outW, outH);
}

bool UInventoryComponent::QuickTransferTo(UInventoryComponent* target, int64 instanceUID)
{
    if (!target || target == this)
        return false;

    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    if (items[idx].bIsEquipped || items[idx].quickSlot != EQuickSlot::EQS_None) // 장착 중인 아이템은 이동 불가
        return false;

    const FItemInstance snapshot = items[idx];
    int32 movedCount = 0;
    target->AddItemInstance(snapshot, movedCount);

    if (movedCount <= 0)
        return false;
   
    RemoveInstance(instanceUID, movedCount);
    return true;
}

bool UInventoryComponent::EquipToQuickSlot(int64 instanceUID, EQuickSlot quickSlot)
{
    int32 srcIdx = FindInstanceIndex(instanceUID);
    if (srcIdx == INDEX_NONE || items[srcIdx].bIsEquipped)
        return false;

    if (items[srcIdx].quickSlot == quickSlot) // 같은 슬롯이면 무시
        return false;

    FItemData data;
    if (!GetItemData(items[srcIdx].itemId, data))
        return false;

    if (!CanAssignToQuickSlot(data.itemId, quickSlot))
        return false;

    const int32 maxEquip = GetMaxEquipCount(data.itemId);
    if (maxEquip <= 0)
        return false;

    const EQuickSlot fromSlot = items[srcIdx].quickSlot; // 슬롯 간 이동 여부
    const FItemInstance srcSnapshot = items[srcIdx];
    const int32 equipCount = FMath::Min(srcSnapshot.currentStackCount, maxEquip);
    const int32 remainCount = srcSnapshot.currentStackCount - equipCount;

    const int32 dstIdx = FindQuickSlotIndex(quickSlot);

    /* ID 일치 - 중첩 연산 */
    if (dstIdx != INDEX_NONE && items[dstIdx].itemId == srcSnapshot.itemId)
    {
        const int32 space = maxEquip - items[dstIdx].currentStackCount;
        if (space <= 0)
            return false;

        const int32 moved = FMath::Min(space, srcSnapshot.currentStackCount);
        items[dstIdx].currentStackCount += moved;
        RemoveInstance(instanceUID, moved); 

        OnQuickSlotChanged.Broadcast(quickSlot);

        if (fromSlot != EQuickSlot::EQS_None)
            OnQuickSlotChanged.Broadcast(fromSlot);

        return true;
    }

    /* 교체 대상 확보 */
    FItemInstance replaced;
    const bool bHasReplaced = (dstIdx != INDEX_NONE);
    if (bHasReplaced)
        replaced = items[dstIdx];

    /* 원본을 그리드/슬롯에서 분리 */
    if (remainCount > 0)
        items[srcIdx].currentStackCount = remainCount;
    else
        RemoveInstanceAt(srcIdx);

    srcIdx = INDEX_NONE;

    /* 교체 대상을 인벤토리로 반환 */
    if (bHasReplaced)
    {
        const int32 replacedIdx = FindQuickSlotIndex(quickSlot);
        if (items.IsValidIndex(replacedIdx))
            items.RemoveAt(replacedIdx);

        bool bReturned = false;

        /* 퀵슬롯 간 이동이면 출발 슬롯으로 맞교체 */
        if (fromSlot != EQuickSlot::EQS_None && CanAssignToQuickSlot(replaced.itemId, fromSlot))
        {
            FItemInstance swapped = replaced;
            swapped.quickSlot = fromSlot;
            swapped.gridPosX = 0;
            swapped.gridPosY = 0;
            swapped.bIsRotated = false;
            items.Add(swapped);
            bReturned = true;
        }
        else
        {
            const bool bSwapInPlace = (remainCount == 0 && fromSlot == EQuickSlot::EQS_None);
            bReturned = bSwapInPlace
                ? ReturnInstanceToGridAt(replaced, srcSnapshot.gridPosX, srcSnapshot.gridPosY, srcSnapshot.bIsRotated)
                : ReturnInstanceToGrid(replaced);
        }

        if (!bReturned) 
        {
            items.Add(replaced);

            if (remainCount > 0)
            {
                const int32 restoreIdx = FindInstanceIndex(instanceUID);
                if (items.IsValidIndex(restoreIdx))
                    items[restoreIdx].currentStackCount = srcSnapshot.currentStackCount;
            }
            else
                ReturnInstanceToGridAt(srcSnapshot, srcSnapshot.gridPosX, srcSnapshot.gridPosY, srcSnapshot.bIsRotated);

            OnInventoryChanged.Broadcast();
            return false;
        }
    }

    /* 장착 인스턴스 생성 */
    FItemInstance equipped = srcSnapshot;
    equipped.instanceUID = (remainCount > 0) ? GenerateInstanceUID() : srcSnapshot.instanceUID;
    equipped.currentStackCount = equipCount;
    equipped.quickSlot = quickSlot;
    equipped.bIsEquipped = false;
    equipped.equipSlot = EEquipSlot::EES_None;
    equipped.gridPosX = 0;
    equipped.gridPosY = 0;
    equipped.bIsRotated = false;

    items.Add(equipped);

    OnQuickSlotChanged.Broadcast(quickSlot);
    if (fromSlot != EQuickSlot::EQS_None)
        OnQuickSlotChanged.Broadcast(fromSlot);

    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::QuickEquipToQuickSlot(int64 instanceUID)
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data) || data.category != EItemCategory::EIC_Consumable)
        return false;

    if (items[idx].quickSlot != EQuickSlot::EQS_None) // 이미 등록된 아이템은 해제
        return UnequipQuickSlot(items[idx].quickSlot);

    if (IsRecoveryData(data)) // 체력 회복 아이템은 회복 슬롯 고정
        return EquipToQuickSlot(instanceUID, EQuickSlot::EQS_Recovery);

    /* 빈 소모품 슬롯을 1~4 순서로 우선 탐색 */
    for (uint8 i = (uint8)EQuickSlot::EQS_Consume1; i <= (uint8)EQuickSlot::EQS_Consume4; ++i)
    {
        const EQuickSlot candidate = (EQuickSlot)i;
        if (FindQuickSlotIndex(candidate) == INDEX_NONE)
            return EquipToQuickSlot(instanceUID, candidate);
    }

    /* 모두 채워져 있으면 1번 슬롯과 교체 */
    return EquipToQuickSlot(instanceUID, EQuickSlot::EQS_Consume1);
}

bool UInventoryComponent::UnequipQuickSlot(EQuickSlot quickSlot)
{
    if (quickSlot == EQuickSlot::EQS_Recovery) // 회복 슬롯은 고정 슬롯
        return false;

    const int32 idx = FindQuickSlotIndex(quickSlot);
    if (idx == INDEX_NONE)
        return false;

    const FItemInstance snapshot = items[idx];
    items.RemoveAt(idx);

    if (!ReturnInstanceToGrid(snapshot))
    {
        items.Add(snapshot);
        return false;
    }

    OnQuickSlotChanged.Broadcast(quickSlot);
    OnInventoryChanged.Broadcast();
    return true;
}

bool UInventoryComponent::UseQuickSlot(EQuickSlot quickSlot, EBodyPart targetPart)
{
    const int32 idx = FindQuickSlotIndex(quickSlot);
    if (idx == INDEX_NONE)
        return false;

    const int64 uid = items[idx].instanceUID;
    const bool bResult = (targetPart == EBodyPart::EBP_None)
        ? UseItem(uid)
        : UseItemOnPart(uid, targetPart);

    if (bResult)
        OnQuickSlotChanged.Broadcast(quickSlot);

    return bResult;
}

int64 UInventoryComponent::GetQuickSlotInstanceUID(EQuickSlot quickSlot) const
{
    const int32 idx = FindQuickSlotIndex(quickSlot);
    return items.IsValidIndex(idx) ? items[idx].instanceUID : 0;
}

bool UInventoryComponent::CanAssignToQuickSlot(FName itemId, EQuickSlot quickSlot) const
{
    if (quickSlot == EQuickSlot::EQS_None || quickSlot == EQuickSlot::EQS_MAX)
        return false;

    FItemData data;
    if (!GetItemData(itemId, data) || data.category != EItemCategory::EIC_Consumable)
        return false;

    if (data.effectType == EItemEffect::EIE_Ammo)
        return false;

    return (quickSlot == EQuickSlot::EQS_Recovery) ? IsRecoveryData(data) : !IsRecoveryData(data);
}

int32 UInventoryComponent::GetMaxEquipCount(FName itemId) const
{
    FItemData data;
    if (!GetItemData(itemId, data) || data.category != EItemCategory::EIC_Consumable)
        return 0;

    if (data.maxEquipCount > 0)
        return data.maxEquipCount;

    return data.bStackable ? FMath::Max(1, data.maxStack) : 1;
}

EBodyPart UInventoryComponent::GetAutoTargetPart(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return EBodyPart::EBP_None;

    FItemData data;
    if (!GetItemData(items[idx].itemId, data))
        return EBodyPart::EBP_None;

    if (data.effectType == EItemEffect::EIE_HealHP)
        return ownerBodyParts ? ownerBodyParts->GetLowestHealthPart() : EBodyPart::EBP_None;

    if (!data.bRequiresBodyPart)
        return EBodyPart::EBP_None;

    TArray<EBodyPart> usableParts;
    GetUsableBodyParts(instanceUID, usableParts);
    return (usableParts.Num() > 0) ? usableParts[0] : EBodyPart::EBP_None;
}

float UInventoryComponent::GetItemCooldown(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return 0.f;

    FItemData data;
    return GetItemData(items[idx].itemId, data) ? data.cooldown : 0.f;
}

int32 UInventoryComponent::GetItemConditionCode(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return 0;

    FItemData data;
    return GetItemData(items[idx].itemId, data) ? data.conditionCode : 0;
}

bool UInventoryComponent::IsRecoveryInstance(int64 instanceUID) const
{
    const int32 idx = FindInstanceIndex(instanceUID);
    if (idx == INDEX_NONE)
        return false;

    FItemData data;
    return GetItemData(items[idx].itemId, data) && IsRecoveryData(data);
}

void UInventoryComponent::BroadcastAllQuickSlots()
{
    for (uint8 i = (uint8)EQuickSlot::EQS_Recovery; i < (uint8)EQuickSlot::EQS_MAX; ++i)
        OnQuickSlotChanged.Broadcast((EQuickSlot)i);
}

int64 UInventoryComponent::FindFirstUnequippedUID(FName itemId) const
{
    for (const FItemInstance& inst : items)
    {
        if (!inst.bIsEquipped && inst.itemId == itemId)
            return inst.instanceUID;
    }
    return 0;
}

void UInventoryComponent::PrintInventoryDebug() const
{
    if (!GEngine)
        return;

    GEngine->AddOnScreenDebugMessage(200, 6.f, FColor::White,
        FString::Printf(TEXT("===== 인벤토리 (%dx%d) ====="), gridWidth, gridHeight));

    int32 line = 0;
    for (const FItemInstance& inst : items)
    {
        FItemData data;
        if (!GetItemData(inst.itemId, data))
            continue;

        const FString equipText = inst.bIsEquipped
            ? FString::Printf(TEXT(" [장착 슬롯 %d]"), (int32)inst.equipSlot)
            : FString::Printf(TEXT(" (%d,%d)%s"), inst.gridPosX, inst.gridPosY, inst.bIsRotated ? TEXT(" R") : TEXT(""));

        const FString durText = (data.maxDurability > 0.f)
            ? FString::Printf(TEXT(" [내구도 %.0f/%.0f]"), inst.currentDurability, data.maxDurability)
            : TEXT("");

        GEngine->AddOnScreenDebugMessage(201 + line, 6.f, GetGradeColor(data.grade),
            FString::Printf(TEXT("#%lld %s x%d%s%s"),
                inst.instanceUID, *data.itemName.ToString(), inst.currentStackCount, *equipText, *durText));
        ++line;
    }

    if (line == 0)
        GEngine->AddOnScreenDebugMessage(201, 6.f, FColor::Silver, TEXT("(비어 있음)"));
}

void UInventoryComponent::PrintEquippedDebug() const
{
    if (!GEngine)
        return;

    static const TCHAR* slotNames[] = {
        TEXT("None"), TEXT("머리"), TEXT("가슴"), TEXT("왼팔"), TEXT("오른팔"),
        TEXT("왼다리"), TEXT("오른다리"), TEXT("왼손"), TEXT("오른손") };

    GEngine->AddOnScreenDebugMessage(300, 6.f, FColor::Cyan,
        FString::Printf(TEXT("===== 장착 현황 (ATK+%.0f / LimbHP+%.0f) ====="), totalAttackBonus, totalLimbHPBonus));

    int32 line = 0;
    for (const FItemInstance& inst : items)
    {
        if (!inst.bIsEquipped)
            continue;

        FItemData data;
        if (!GetItemData(inst.itemId, data))
            continue;

        const int32 s = (int32)inst.equipSlot;
        const FString name = (s >= 0 && s <= 8) ? slotNames[s] : TEXT("?");
        GEngine->AddOnScreenDebugMessage(301 + line, 6.f, GetGradeColor(data.grade),
            FString::Printf(TEXT("%s : %s"), *name, *data.itemName.ToString()));
        ++line;
    }

    if (line == 0)
        GEngine->AddOnScreenDebugMessage(301, 6.f, FColor::Silver, TEXT("(장착 없음)"));
}

void UInventoryComponent::PrintGridDebug() const
{
    UE_LOG(LogWindless, Warning, TEXT("===== 그리드 점유 맵 (%dx%d) ====="), gridWidth, gridHeight);

    FString header = TEXT("    ");
    for (int32 x = 0; x < gridWidth; ++x)
        header += FString::Printf(TEXT("%2d"), x);
    UE_LOG(LogWindless, Warning, TEXT("%s"), *header);

    int32 used = 0;
    for (int32 y = 0; y < gridHeight; ++y)
    {
        FString row = FString::Printf(TEXT("Y%d |"), y);
        for (int32 x = 0; x < gridWidth; ++x)
        {
            const bool bOccupied = (gridMap[CellIndex(x, y)] != 0);
            row += bOccupied ? TEXT(" #") : TEXT(" .");
            used += bOccupied ? 1 : 0;
        }
        UE_LOG(LogWindless, Warning, TEXT("%s"), *row);
    }

    UE_LOG(LogWindless, Warning, TEXT("점유 %d / 전체 %d 칸"), used, gridMap.Num());
}

void UInventoryComponent::PrintCarryLimitDebug() const
{
    UE_LOG(LogWindless, Warning, TEXT("===== 소지 개수 제한 현황 (기본 한도: %d) ====="), defaultInjuryCureCarryLimit);

    TSet<FName> printed;
    for (const FItemInstance& inst : items)
    {
        if (printed.Contains(inst.itemId))
            continue;

        printed.Add(inst.itemId);
        const int32 limit = GetCarryLimit(inst.itemId);
        if (limit <= 0)
            continue;

        FItemData data;
        if (!GetItemData(inst.itemId, data))
            continue;

        UE_LOG(LogWindless, Warning, TEXT("%s (%s) : %d / %d"), *data.itemName.ToString(), *inst.itemId.ToString(), GetItemTotalCount(inst.itemId), limit);
    }
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
    if (AActor* owner = GetOwner())
    {
        ownerAttributes = owner->FindComponentByClass<UAttributeComponent>();
        ownerBodyParts = owner->FindComponentByClass<UBodyPartComponent>();
    }

    itemDatabase = UItemDatabaseSubsystem::Get(this);
    ResetGridMap();
}

int64 UInventoryComponent::GenerateInstanceUID()
{
    return nextInstanceUID++;
}

void UInventoryComponent::ResetGridMap()
{
    gridMap.Init(0, gridWidth * gridHeight);
}

void UInventoryComponent::GetFootprint(const FItemData& data, bool bRotated, int32& outW, int32& outH) const
{
    if (bRotated)
    {
        outW = data.height;
        outH = data.width;
    }
    else
    {
        outW = data.width;
        outH = data.height;
    }
}

bool UInventoryComponent::CanPlaceAt(int32 startX, int32 startY, int32 w, int32 h) const
{
    if (startX < 0 || startY < 0 || startX + w > gridWidth || startY + h > gridHeight)
        return false;

    for (int32 y = startY; y < startY + h; ++y)
    {
        for (int32 x = startX; x < startX + w; ++x)
        {
            if (gridMap[CellIndex(x, y)] != 0)
                return false;
        }
    }
    return true;
}

void UInventoryComponent::SetCells(int32 startX, int32 startY, int32 w, int32 h, int32 value)
{
    for (int32 y = startY; y < startY + h; ++y)
    {
        for (int32 x = startX; x < startX + w; ++x)
        {
            if (gridMap.IsValidIndex(CellIndex(x, y)))
                gridMap[CellIndex(x, y)] = value;
        }
    }
}

void UInventoryComponent::OccupyForInstance(const FItemInstance& inst)
{
    if (inst.bIsEquipped || inst.quickSlot != EQuickSlot::EQS_None) // 장착 슬롯 및 퀵슬롯 등록 아이템 무시
        return;

    FItemData data;
    if (!GetItemData(inst.itemId, data))
        return;

    int32 w, h;
    GetFootprint(data, inst.bIsRotated, w, h);
    SetCells(inst.gridPosX, inst.gridPosY, w, h, 1);
}

void UInventoryComponent::FreeForInstance(const FItemInstance& inst)
{
    if (inst.bIsEquipped || inst.quickSlot != EQuickSlot::EQS_None) // 장착 슬롯 및 퀵슬롯 등록 아이템 무시
        return;

    FItemData data;
    if (!GetItemData(inst.itemId, data))
        return;

    int32 w, h;
    GetFootprint(data, inst.bIsRotated, w, h);
    SetCells(inst.gridPosX, inst.gridPosY, w, h, 0);
}

bool UInventoryComponent::TryAutoPlace(FItemInstance& inst)
{
    FItemData data;
    if (!GetItemData(inst.itemId, data))
        return false;

    // 회전하지 않은 상태로 탐색
    auto ScanAndPlace = [&](bool bRotated) -> bool
        {
            int32 w, h;
            GetFootprint(data, bRotated, w, h);
            for (int32 y = 0; y < gridHeight; ++y)      // Y축 우선
            {
                for (int32 x = 0; x < gridWidth; ++x)   // X축 차순
                {
                    if (CanPlaceAt(x, y, w, h))
                    {
                        inst.gridPosX = x;
                        inst.gridPosY = y;
                        inst.bIsRotated = bRotated;
                        SetCells(x, y, w, h, 1);
                        return true;
                    }
                }
            }
            return false;
        };

    if (ScanAndPlace(false))
        return true;

    // 회전 배치
    if (bAllowAutoRotate && data.width != data.height)
        return ScanAndPlace(true);

    return false;
}

int32 UInventoryComponent::FindInstanceIndex(int64 instanceUID) const
{
    for (int32 i = 0; i < items.Num(); ++i)
    {
        if (items[i].instanceUID == instanceUID)
            return i;
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::FindEquippedIndex(EEquipSlot slot) const
{
    for (int32 i = 0; i < items.Num(); ++i)
    {
        if (items[i].bIsEquipped && items[i].equipSlot == slot)
            return i;
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::FindStackableIndex(FName itemId, int32 maxStack) const
{
    for (int32 i = 0; i < items.Num(); ++i)
    {
        if (!items[i].bIsEquipped &&
            items[i].quickSlot == EQuickSlot::EQS_None &&
            items[i].itemId == itemId &&
            items[i].currentStackCount < maxStack)
                return i;
    }
    return INDEX_NONE;
}

int32 UInventoryComponent::FindInstanceAtCell(int32 x, int32 y) const
{
    if (x < 0 || y < 0 || x >= gridWidth || y >= gridHeight)
        return INDEX_NONE;

    for (int32 i = 0; i < items.Num(); ++i)
    {
        if (items[i].bIsEquipped || items[i].quickSlot != EQuickSlot::EQS_None)
            continue;

        FItemData data;
        if (!GetItemData(items[i].itemId, data))
            continue;

        int32 w, h;
        GetFootprint(data, items[i].bIsRotated, w, h);

        const int32 sx = items[i].gridPosX;
        const int32 sy = items[i].gridPosY;

        if (x >= sx && x < sx + w && y >= sy && y < sy + h)
            return i;
    }
    return INDEX_NONE;
}

EEquipSlot UInventoryComponent::GetSlotGroup(EEquipSlot slot)
{
    switch (slot)
    {
        case EEquipSlot::EES_LeftArm:
        case EEquipSlot::EES_RightArm:
            return EEquipSlot::EES_RightArm;

        case EEquipSlot::EES_LeftLeg:
        case EEquipSlot::EES_RightLeg:
            return EEquipSlot::EES_RightLeg;

        case EEquipSlot::EES_LeftHand:
        case EEquipSlot::EES_RightHand:
            return EEquipSlot::EES_RightHand;

        default:
            return slot;
    }
}

EEquipSlot UInventoryComponent::GetPairedSlot(EEquipSlot slot)
{
    switch (slot)
    {
        case EEquipSlot::EES_LeftArm:   return EEquipSlot::EES_RightArm;
        case EEquipSlot::EES_RightArm:  return EEquipSlot::EES_LeftArm;

        case EEquipSlot::EES_LeftLeg:   return EEquipSlot::EES_RightLeg;
        case EEquipSlot::EES_RightLeg:  return EEquipSlot::EES_LeftLeg;

        case EEquipSlot::EES_LeftHand:  return EEquipSlot::EES_RightHand;
        case EEquipSlot::EES_RightHand: return EEquipSlot::EES_LeftHand;

        default: return EEquipSlot::EES_None;
    }
}

EEquipSlot UInventoryComponent::GetLeftSlotOfPair(EEquipSlot slot)
{
    switch (slot)
    {
        case EEquipSlot::EES_LeftArm:
        case EEquipSlot::EES_RightArm:
            return EEquipSlot::EES_LeftArm;

        case EEquipSlot::EES_LeftLeg:
        case EEquipSlot::EES_RightLeg:
            return EEquipSlot::EES_LeftLeg;

        case EEquipSlot::EES_LeftHand:
        case EEquipSlot::EES_RightHand:
            return EEquipSlot::EES_LeftHand;

        default:
            return slot;
    }
}

EEquipSlot UInventoryComponent::ResolveAutoEquipSlot(EEquipSlot desiredSlot) const
{
    if (GetPairedSlot(desiredSlot) == EEquipSlot::EES_None)
        return desiredSlot;

    const EEquipSlot leftSlot = GetLeftSlotOfPair(desiredSlot);
    const EEquipSlot rightSlot = GetPairedSlot(leftSlot);

    if (FindEquippedIndex(leftSlot) == INDEX_NONE)
        return leftSlot;

    if (FindEquippedIndex(rightSlot) == INDEX_NONE)
        return rightSlot;

    return leftSlot;
}

bool UInventoryComponent::IsEquipSlotChangeAllowed(EEquipSlot slot) const
{
    return !canChangeEquipSlot.IsBound() || canChangeEquipSlot.Execute(slot);
}

bool UInventoryComponent::ReleaseEquippedToGrid(EEquipSlot slot)
{
    const int32 idx = FindEquippedIndex(slot);
    if (idx == INDEX_NONE)
        return true;

    items[idx].bIsEquipped = false;
    items[idx].equipSlot = EEquipSlot::EES_None;

    if (!TryAutoPlace(items[idx])) // 공간이 없으면 원상 복구
    {
        items[idx].bIsEquipped = true;
        items[idx].equipSlot = slot;
        return false;
    }
    return true;
}

EEquipSlot UInventoryComponent::FindTwoHandedConflictSlot(EEquipSlot desiredSlot, bool bTwoHanded) const
{
    if (desiredSlot != EEquipSlot::EES_LeftHand && desiredSlot != EEquipSlot::EES_RightHand)
        return EEquipSlot::EES_None;

    const EEquipSlot pairedSlot = GetPairedSlot(desiredSlot);
    const int32 pairedIdx = FindEquippedIndex(pairedSlot);
    if (pairedIdx == INDEX_NONE)
        return EEquipSlot::EES_None;

    if (bTwoHanded)
        return pairedSlot;

    FItemData pairedData;
    if (GetItemData(items[pairedIdx].itemId, pairedData) && IsTwoHandedWeapon(pairedData))
        return pairedSlot;

    return EEquipSlot::EES_None;
}

bool UInventoryComponent::IsHandSlot(EEquipSlot slot)
{
    return slot == EEquipSlot::EES_LeftHand || slot == EEquipSlot::EES_RightHand;
}

bool UInventoryComponent::IsTwoHandedWeapon(const FItemData& data)
{
    const bool bHandItem = (data.equipSlot == EEquipSlot::EES_LeftHand || data.equipSlot == EEquipSlot::EES_RightHand);
    return bHandItem && (data.weaponType == EWeaponType::EWT_TwoHanded || data.weaponType == EWeaponType::EWT_Bow);
}

bool UInventoryComponent::ApplyConsumableEffect(const FItemData& data, EBodyPart targetPart, int64 instanceUID)
{
    bool bApplied = false;

    switch (data.effectType)
    {
        case EItemEffect::EIE_HealHP:
        {
            if (ownerBodyParts)
            {
                const EBodyPart part = (targetPart != EBodyPart::EBP_None) ? targetPart : ownerBodyParts->GetLowestHealthPart();
                if (part != EBodyPart::EBP_None)
                    bApplied = ownerBodyParts->HealPart(part, data.effectAmount);
            }
            else if (ownerAttributes)
            {
                ownerAttributes->Heal(data.effectAmount);
                bApplied = true;
            }
            break;
        }

        case EItemEffect::EIE_CureInjury:
        {
            if (ownerBodyParts && targetPart != EBodyPart::EBP_None)
                bApplied = ownerBodyParts->CurePartInjury(targetPart, data.targetInjury);
            break;
        }

        case EItemEffect::EIE_RevivePart:
        {
            if (ownerBodyParts && targetPart != EBodyPart::EBP_None)
            {
                if (ownerBodyParts->IsInSaveZone())
                    bApplied = ownerBodyParts->RevivePart(targetPart, 0.5f);
            }
            break;
        }

        case EItemEffect::EIE_RecoverChaos:
            if (ownerAttributes)
            {
                ownerAttributes->RecoverChaos(data.effectAmount);
                bApplied = true;
            }
            break;

        case EItemEffect::EIE_BodyHeat:
            if (ownerAttributes) 
            {
                ownerAttributes->ApplyTimedBodyHeat(data.effectAmount, data.effectDuration);
                bApplied = true;
            }
            break;

        case EItemEffect::EIE_GiveMoney:
            // AddMoney((int64)data.effectAmount);
            bApplied = true;
            break;
        
        case EItemEffect::EIE_RepairTool:
        {
            URepairComponent* repairComp = GetOwner() ? GetOwner()->FindComponentByClass<URepairComponent>() : nullptr;
            if (!repairComp || instanceUID == 0)
                break;

            int64 worstUID = 0;
            float worstRatio = 1.f;
            for (const FRepairTargetEntry& target : repairComp->GetRepairTargets())
            {
                if (target.requiredCharge <= 0.f || target.maxDurability <= 0.f)
                    continue;

                const float ratio = target.currentDurability / target.maxDurability;
                if (ratio < worstRatio)
                {
                    worstRatio = ratio;
                    worstUID = target.instanceUID;
                }
            }

            if (worstUID != 0)
                bApplied = repairComp->RepairWithTool(instanceUID, worstUID);
            break;
        }

        case EItemEffect::EIE_SpawnObject: // 빛나는 돌: 발밑에 빛 구역을 놓음 (D-37, D-52)
        {
            AActor* owner = GetOwner();
            AChaosZoneVolume* stone = owner ? GetWorld()->SpawnActor<AChaosZoneVolume>(owner->GetActorLocation(), FRotator::ZeroRotator) : nullptr;
            if (!stone)
                break;

            if (shiningStone.IsValid()) // 한 번에 하나
                shiningStone->Destroy();

            // ponytail: 반경 3 m를 상자(한 변 6 m)로 근사. 구가 필요하면 구 컴포넌트로 바꾼다
            stone->InitAsLight(0.f, 300.f);
            stone->SetLifeSpan(data.effectDuration > 0.f ? data.effectDuration : 60.f);
            shiningStone = stone;
            bApplied = true;
            break;
        }

        default:
            // TODO(후속): 투척/인챈트/탐지는 각 시스템 완성 후 effectPayload 로 라우팅 (DEV-14)
            break;
    }
    return bApplied;
}

void UInventoryComponent::RecalculateEquipmentStats()
{
    totalAttackBonus = 0.f;
    totalLimbHPBonus = 0.f;

    TMap<EBodyPart, float> partBonus;

    for (const FItemInstance& item : items)
    {
        if (!item.bIsEquipped)
            continue;

        FItemData data;
        if (!GetItemData(item.itemId, data))
            continue;

        if (data.maxDurability > 0.f && item.currentDurability <= 0.f)
            continue;

        if (!IsHandSlot(item.equipSlot))
            totalAttackBonus += data.attackPowerBonus;
        totalLimbHPBonus += data.maxLimbHPBonus;

        const EBodyPart part = EquipSlotToBodyPart(item.equipSlot);
        if (part != EBodyPart::EBP_None)
            partBonus.FindOrAdd(part) += data.maxLimbHPBonus;
    }

    if (ownerAttributes)
        ownerAttributes->SetEquipmentAttackBonus(totalAttackBonus);

    if (ownerBodyParts)
    {
        for (int32 i = (int32)EBodyPart::EBP_Head; i <= (int32)EBodyPart::EBP_RightLeg; ++i)
        {
            const EBodyPart part = (EBodyPart)i;
            ownerBodyParts->SetEquipmentMaxHealthBonus(part, partBonus.FindRef(part));
        }
    }
}

EBodyPart UInventoryComponent::EquipSlotToBodyPart(EEquipSlot slot)
{
    switch (slot)
    {
        case EEquipSlot::EES_Head:
            return EBodyPart::EBP_Head;
        case EEquipSlot::EES_Chest:
            return EBodyPart::EBP_Chest;
        case EEquipSlot::EES_LeftArm:
            return EBodyPart::EBP_LeftArm;
        case EEquipSlot::EES_RightArm:
            return EBodyPart::EBP_RightArm;
        case EEquipSlot::EES_LeftLeg:
            return EBodyPart::EBP_LeftLeg;
        case EEquipSlot::EES_RightLeg:
            return EBodyPart::EBP_RightLeg;
        default:
            return EBodyPart::EBP_None;
    }
}

bool UInventoryComponent::IsInjuryCureData(const FItemData& data)
{
    return data.category == EItemCategory::EIC_Consumable
        && data.effectType == EItemEffect::EIE_CureInjury
        && data.targetInjury != EInjuryType::EIT_None;
}

int32 UInventoryComponent::GetCarryLimitFromData(const FItemData& data) const
{
    if (data.carryLimit > 0)
        return data.carryLimit;

    if (IsInjuryCureData(data))
        return FMath::Max(0, defaultInjuryCureCarryLimit);

    return 0;
}

void UInventoryComponent::NotifyCarryLimitBlocked(const FItemData& data, int32 addedCount) const
{
    const int32 limit = GetCarryLimitFromData(data);
    const int32 current = GetItemTotalCount(data.itemId); // 획득 반영 후 호출되어야 정확한 수치

    const FString message = (addedCount > 0)
        ? FString::Printf(TEXT("[소지 한도] %s %d개만 획득했습니다 (%d/%d)"), *data.itemName.ToString(), addedCount, current, limit)
        : FString::Printf(TEXT("[소지 한도] %s 은(는) 최대 %d개까지만 소지할 수 있습니다 (%d/%d)"), *data.itemName.ToString(), limit, current, limit);

    if (GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, message);

    UE_LOG(LogWindless, Log, TEXT("[소지 한도] %s : %d개 획득 (%d/%d)"), *data.itemId.ToString(), addedCount, current, limit);
}

int32 UInventoryComponent::FindQuickSlotIndex(EQuickSlot quickSlot) const
{
    if (quickSlot == EQuickSlot::EQS_None)
        return INDEX_NONE;

    for (int32 i = 0; i < items.Num(); ++i)
    {
        if (items[i].quickSlot == quickSlot)
            return i;
    }
    return INDEX_NONE;
}

bool UInventoryComponent::IsRecoveryData(const FItemData& data)
{
    return data.effectType == EItemEffect::EIE_HealHP;
}

bool UInventoryComponent::ReturnInstanceToGrid(FItemInstance inst)
{
    inst.quickSlot = EQuickSlot::EQS_None;
    inst.bIsEquipped = false;
    inst.equipSlot = EEquipSlot::EES_None;

    if (!TryAutoPlace(inst))
        return false;

    items.Add(inst);
    return true;
}

bool UInventoryComponent::ReturnInstanceToGridAt(FItemInstance inst, int32 x, int32 y, bool bRotated)
{
    FItemData data;
    if (!GetItemData(inst.itemId, data))
        return false;

    int32 w, h;
    GetFootprint(data, bRotated, w, h);

    if (!CanPlaceAt(x, y, w, h))
        return ReturnInstanceToGrid(inst);

    inst.quickSlot = EQuickSlot::EQS_None;
    inst.bIsEquipped = false;
    inst.equipSlot = EEquipSlot::EES_None;
    inst.gridPosX = x;
    inst.gridPosY = y;
    inst.bIsRotated = bRotated;

    SetCells(x, y, w, h, 1);
    items.Add(inst);
    return true;
}

void UInventoryComponent::RefillRecoveryItem(int32 charges)
{
    int32 idx = FindQuickSlotIndex(EQuickSlot::EQS_Recovery);
    if (idx == INDEX_NONE && AddItemById(recoveryItemId, 1))
        idx = FindQuickSlotIndex(EQuickSlot::EQS_Recovery);
    if (idx == INDEX_NONE)
        return;

    items[idx].currentStackCount = FMath::Max(1, charges);
    OnQuickSlotChanged.Broadcast(EQuickSlot::EQS_Recovery);
}

void UInventoryComponent::TryAutoEquipRecoveryItem(FName itemId)
{
    const int32 slotIdx = FindQuickSlotIndex(EQuickSlot::EQS_Recovery);

    /* 다른 회복 아이템이 이미 고정되어 있으면 유지 */
    if (slotIdx != INDEX_NONE && items[slotIdx].itemId != itemId)
        return;

    const int32 maxEquip = GetMaxEquipCount(itemId);
    if (slotIdx != INDEX_NONE && items[slotIdx].currentStackCount >= maxEquip)
        return;

    /* 그리드에 남아 있는 동일 아이템의 UID를 먼저 확보 */
    int64 targetUID = 0;
    for (const FItemInstance& inst : items)
    {
        if (inst.quickSlot == EQuickSlot::EQS_None && !inst.bIsEquipped && inst.itemId == itemId)
        {
            targetUID = inst.instanceUID;
            break;
        }
    }

    if (targetUID != 0)
        EquipToQuickSlot(targetUID, EQuickSlot::EQS_Recovery); // 배열이 변경되므로 순회 밖에서 호출
}
