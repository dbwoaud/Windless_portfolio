#include "UI/ItemInfoWidgetBase.h"
#include "UI/InventoryWidget.h"
#include "Components/InventoryComponent.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

void UItemInfoWidgetBase::SetupInfo(UInventoryWidget* inOwnerWidget, UInventoryComponent* inInventory, int64 uid)
{
	owningInventoryWidget = inOwnerWidget;
	owningInventory = inInventory;
	targetUID = uid;

	RefreshInfo();
}

void UItemInfoWidgetBase::RefreshInfo()
{
	FItemInstance inst;
	FItemData data;
	if (!FindTargetItem(inst, data))
	{
		HandleInvalidTarget();
		return;
	}

	ApplyItemInfo(inst, data);
}

bool UItemInfoWidgetBase::FindTargetItem(FItemInstance& outInstance, FItemData& outData) const
{
	if (!owningInventory)
		return false;

	outInstance = owningInventory->FindInstance(targetUID);
	if (!outInstance.IsValid())
		return false;

	return owningInventory->GetItemData(outInstance.itemId, outData);
}

void UItemInfoWidgetBase::ApplyItemInfo(const FItemInstance& inst, const FItemData& data)
{
	if (itemNameText)
	{
		itemNameText->SetText(data.itemName);
		itemNameText->SetColorAndOpacity(FSlateColor(FLinearColor(owningInventory->GetGradeColor(data.grade))));
	}

	if (categoryText)
		categoryText->SetText(SetCategoryText(data));

	if (statText)
		statText->SetText(SetStatText(inst, data));
}

void UItemInfoWidgetBase::HandleInvalidTarget()
{
	RemoveFromParent();
}

FText UItemInfoWidgetBase::SetCategoryText(const FItemData& data) const
{
	FString category;
	switch (data.category)
	{
	case EItemCategory::EIC_Equipment:  category = TEXT("장비"); break;
	case EItemCategory::EIC_Material:   category = TEXT("재료"); break;
	case EItemCategory::EIC_Consumable: category = TEXT("소비"); break;
	default:                            category = TEXT("기타"); break;
	}

	static const TCHAR* gradeNames[] = { TEXT("일반"), TEXT("고급"), TEXT("희귀"), TEXT("유일"), TEXT("전설") };
	const int32 gradeIdx = static_cast<int32>(data.grade);
	const FString gradeName = (gradeIdx >= 0 && gradeIdx < UE_ARRAY_COUNT(gradeNames)) ? gradeNames[gradeIdx] : TEXT("-");

	return FText::FromString(FString::Printf(TEXT("%s  |  %s  |  %d x %d칸"), *category, *gradeName, data.width, data.height));
}

FText UItemInfoWidgetBase::SetStatText(const FItemInstance& inst, const FItemData& data) const
{
	TArray<FString> lines;

	/* 내구도 */
	if (data.maxDurability > 0.f)
	{
		lines.Add(FString::Printf(TEXT("내구도  %.0f / %.0f%s"), inst.currentDurability, data.maxDurability,
			(inst.currentDurability <= 0.f) ? TEXT("  [파괴됨]") : TEXT("")));
	}

	/* 공격력 */
	if (!FMath::IsNearlyZero(data.attackPowerBonus))
		lines.Add(FString::Printf(TEXT("공격력  +%.0f"), data.attackPowerBonus));

	/* 생명력 */
	if (!FMath::IsNearlyZero(data.maxLimbHPBonus))
		lines.Add(FString::Printf(TEXT("부위 생명력  +%.0f"), data.maxLimbHPBonus));

	/* 소모성 아이템 */
	if (data.category == EItemCategory::EIC_Consumable)
	{
		if (!FMath::IsNearlyZero(data.effectAmount))
			lines.Add(FString::Printf(TEXT("효과량  %.0f"), data.effectAmount));

		// 실제로 쓰이는 시전 시간을 보여 준다(소드락은 회복 아이템 값을 따르고, 혼돈 배율이 붙는다)
		const float castTime = owningInventory ? owningInventory->GetItemCastTime(inst.instanceUID) : data.useTime;
		if (castTime > 0.f)
			lines.Add(FString::Printf(TEXT("시전 시간  %.1f초"), castTime));

		if (data.cooldown > 0.f)
			lines.Add(FString::Printf(TEXT("재사용 대기  %.1f초"), data.cooldown));
	}

	/* 스택 표시 */
	if (data.bStackable)
		lines.Add(FString::Printf(TEXT("수량  %d / %d"), inst.currentStackCount, data.maxStack));

	/* 소지 개수 제한 표시 */
	if (owningInventory && owningInventory->IsCarryLimited(data.itemId))
	{
		lines.Add(FString::Printf(TEXT("소지 한도  %d / %d%s"),
			owningInventory->GetItemTotalCount(data.itemId),
			owningInventory->GetCarryLimit(data.itemId),
			owningInventory->IsCarryLimitReached(data.itemId) ? TEXT("  [한도 도달]") : TEXT("")));
	}

	/* 버리기 여부 */
	if (!data.bDroppable)
		lines.Add(TEXT("버릴 수 없는 아이템"));

	return FText::FromString(FString::Join(lines, TEXT("\n")));
}