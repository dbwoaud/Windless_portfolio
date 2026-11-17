#include "Components/RepairComponent.h"
#include "Components/InventoryComponent.h"
#include "Windless/DebugMacro.h"

URepairComponent::URepairComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<FRepairToolEntry> URepairComponent::GetRepairTools() const
{
	TArray<FRepairToolEntry> result;

	UInventoryComponent* inventory = GetOwnerInventory();
	if (!inventory)
		return result;

	for (const FItemInstance& inst : inventory->GetItems())
	{
		FItemData data;
		if (!inventory->GetItemData(inst.itemId, data))
			continue;

		if (data.effectType != EItemEffect::EIE_RepairTool)
			continue;

		FRepairToolEntry entry;
		entry.instanceUID = inst.instanceUID;
		entry.toolName = data.itemName;
		entry.maxCharge = (data.maxDurability > 0.f) ? data.maxDurability : GetRepairToolMaxCharge(data.grade);
		entry.currentCharge = (inst.currentDurability < 0.f) ? entry.maxCharge : inst.currentDurability;
		result.Add(entry);
	}

	return result;
}

TArray<FRepairTargetEntry> URepairComponent::GetRepairTargets() const
{
	TArray<FRepairTargetEntry> result;

	UInventoryComponent* inventory = GetOwnerInventory();
	if (!inventory)
		return result;

	// 장착 슬롯만 대상
	for (int32 i = static_cast<int32>(EEquipSlot::EES_Head); i < static_cast<int32>(EEquipSlot::EES_MAX); ++i)
	{
		const EEquipSlot slot = static_cast<EEquipSlot>(i);
		const int64 uid = inventory->GetEquippedInstanceUID(slot);
		if (uid == 0)
			continue;

		const FItemInstance inst = inventory->FindInstance(uid);
		FItemData data;
		if (!inventory->GetItemData(inst.itemId, data) || data.maxDurability <= 0.f)
			continue;

		FRepairTargetEntry entry;
		entry.instanceUID = uid;
		entry.slot = slot;
		entry.equipmentName = data.itemName;
		entry.maxDurability = data.maxDurability;
		entry.currentDurability = (inst.currentDurability < 0.f) ? data.maxDurability : inst.currentDurability;
		entry.requiredCharge = FMath::Max(0.f, entry.maxDurability - entry.currentDurability);
		entry.bBroken = (entry.currentDurability <= 0.f);

		result.Add(entry);
	}

	return result;
}

bool URepairComponent::IsRepairTool(int64 instanceUID) const
{
	UInventoryComponent* inventory = GetOwnerInventory();
	if (!inventory)
		return false;

	const FItemInstance inst = inventory->FindInstance(instanceUID);
	if (inst.instanceUID == 0)
		return false;

	FItemData data;
	if (!inventory->GetItemData(inst.itemId, data))
		return false;

	return data.effectType == EItemEffect::EIE_RepairTool;
}

float URepairComponent::GetRequiredCharge(int64 targetUID) const
{
	UInventoryComponent* inventory = GetOwnerInventory();
	if (!inventory)
		return 0.f;

	const FItemInstance inst = inventory->FindInstance(targetUID);
	if (inst.instanceUID == 0)
		return 0.f;

	FItemData data;
	if (!inventory->GetItemData(inst.itemId, data) || data.maxDurability <= 0.f)
		return 0.f;

	const float current = (inst.currentDurability < 0.f) ? data.maxDurability : inst.currentDurability;
	return FMath::Max(0.f, data.maxDurability - current);
}

bool URepairComponent::RepairWithTool(int64 toolUID, int64 targetUID)
{
	UInventoryComponent* inventory = GetOwnerInventory();
	if (!inventory || !IsRepairTool(toolUID))
		return false;

	const float required = GetRequiredCharge(targetUID);
	if (required <= 0.f) // 이미 최대 내구도
		return false; 

	const FItemInstance toolInst = inventory->FindInstance(toolUID);
	FItemData toolData;
	if (!inventory->GetItemData(toolInst.itemId, toolData))
		return false;

	const float maxCharge = (toolData.maxDurability > 0.f) ? toolData.maxDurability : GetRepairToolMaxCharge(toolData.grade);
	const float currentCharge = (toolInst.currentDurability < 0.f) ? maxCharge : toolInst.currentDurability;
	if (currentCharge <= 0.f)
		return false;

	// 충전량이 모자라면 가능한 만큼만 수리
	const float used = FMath::Min(required, currentCharge);
	if (!inventory->RepairItem(targetUID, used)) // 내구도 회복
		return false;

	// 수리도구 충전량 차감
	inventory->SetInstanceDurability(toolUID, currentCharge - used);

	UE_LOG(LogWindless, Log, TEXT("[수리] %s -> %s : 충전량 %.0f 소모 (잔여 %.0f)"),
		*toolData.itemName.ToString(), *inventory->FindInstance(targetUID).itemId.ToString(), used, currentCharge - used);

	onRepairPerformed.Broadcast(targetUID, used);

	// 충전량 소진 시 인벤토리에서 제거
	if (currentCharge - used <= 0.f)
	{
		inventory->RemoveInstance(toolUID, 1);
		onRepairToolDepleted.Broadcast(toolUID);
	}

	return true;
}

UInventoryComponent* URepairComponent::GetOwnerInventory() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UInventoryComponent>() : nullptr;
}
