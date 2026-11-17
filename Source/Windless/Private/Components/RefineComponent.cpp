#include "Components/RefineComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/AttributeComponent.h"
#include "Items/RefineTypes.h"
#include "Windless/DebugMacro.h"

URefineComponent::URefineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool URefineComponent::CanRefine(UInventoryComponent* inventory, int64 instanceUID) const
{
	if (!inventory)
		return false;

	const FItemInstance inst = inventory->FindInstance(instanceUID);
	if (inst.instanceUID == 0)
		return false;

	FItemData data;
	if (!inventory->GetItemData(inst.itemId, data))
		return false;

	return data.bDroppable;
}

int32 URefineComponent::GetRefineValue(UInventoryComponent* inventory, int64 instanceUID, int32 quantity) const
{
	if (!CanRefine(inventory, instanceUID))
		return 0;

	const FItemInstance inst = inventory->FindInstance(instanceUID);

	FItemData data;
	if (!inventory->GetItemData(inst.itemId, data))
		return 0;

	const int32 count = (quantity < 0) ? FMath::Max(1, inst.currentStackCount) : FMath::Clamp(quantity, 1, FMath::Max(1, inst.currentStackCount));
	return GetRefineValueForGrade(data.grade) * count;
}

bool URefineComponent::Refine(UInventoryComponent* inventory, int64 instanceUID, int32 quantity)
{
	const int32 gained = GetRefineValue(inventory, instanceUID, quantity);
	if (gained <= 0)
		return false;

	const FItemInstance inst = inventory->FindInstance(instanceUID);
	const int32 removeCount = (quantity < 0) ? FMath::Max(1, inst.currentStackCount) : quantity;

	if (!inventory->RemoveInstance(instanceUID, removeCount))
		return false;

	UAttributeComponent* attributes = GetOwnerAttributes();
	if (!attributes)
		return false;

	attributes->AddMoltStones(gained);

	UE_LOG(LogWindless, Log, TEXT("[정제] %s x%d -> 탈피석 +%d (총 %d)"),
		*inst.itemId.ToString(), removeCount, gained, attributes->GetMoltStones());

	onItemRefined.Broadcast(gained, attributes->GetMoltStones());
	return true;
}

UAttributeComponent* URefineComponent::GetOwnerAttributes() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UAttributeComponent>() : nullptr;
}
