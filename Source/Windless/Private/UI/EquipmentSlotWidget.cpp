#include "UI/EquipmentSlotWidget.h"
#include "UI/WindlessUIStyle.h"
#include "UI/InventoryWidget.h"
#include "UI/ItemDragDrop.h"
#include "Characters/WindlessCharacter.h"
#include "Components/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Windless/DebugMacro.h"

void UEquipmentSlotWidget::InitializeSlot(UInventoryComponent* inventory, UInventoryWidget* ownerWidget)
{
	owningInventory = inventory;
	owningInventoryWidget = ownerWidget;
	ClearSlotHighlight(); // 기본 칸 모양(공용 스타일)을 입힌다
	RefreshSlot();
}

void UEquipmentSlotWidget::RefreshSlot()
{
	if (!owningInventory)
		return;

	const EEquipSlot sourceSlot = GetSourceSlot();
	const bool bMirrored = (sourceSlot != slotType);
	const int64 uid = owningInventory->GetEquippedInstanceUID(sourceSlot);
	const bool bHasItem = (uid != 0);
	const float displayOpacity = bMirrored ? 0.4f : 1.f;

	if (bHasItem)
	{
		const FItemInstance inst = owningInventory->FindInstance(uid);
		FItemData data;
		owningInventory->GetItemData(inst.itemId, data);

		if (icon)
		{
			icon->SetBrushFromTexture(data.icon);
			icon->SetRenderOpacity(displayOpacity);
			icon->SetVisibility(data.icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (gradeBorder)
		{
			gradeBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
			gradeBorder->SetColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.5f));
			gradeBorder->SetRenderOpacity(displayOpacity);
		}
		if (durabilityBar)
		{
			if (data.maxDurability > 0.f && !bMirrored)
			{
				durabilityBar->SetPercent(inst.currentDurability / data.maxDurability);
				durabilityBar->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
			else
				durabilityBar->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (emptyIcon)
			emptyIcon->SetVisibility(ESlateVisibility::Collapsed);
		WindlessUI::NameLabel(this, icon, data.icon ? FText::GetEmpty() : data.itemName);
	}
	else
	{
		WindlessUI::NameLabel(this, icon, FText::GetEmpty());
		if (icon)
			icon->SetVisibility(ESlateVisibility::Collapsed);
		if (gradeBorder)
			gradeBorder->SetVisibility(ESlateVisibility::Collapsed);
		if (durabilityBar)
			durabilityBar->SetVisibility(ESlateVisibility::Collapsed);
		if (emptyIcon)
			emptyIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

FReply UEquipmentSlotWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (owningInventory && InKeyEvent.GetKey() == EKeys::F)
	{
		const EEquipSlot sourceSlot = GetSourceSlot();
		if (owningInventory->GetEquippedInstanceUID(sourceSlot) != 0)
		{
			owningInventory->UnequipSlot(sourceSlot);
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!owningInventory) 
		return FReply::Handled();

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton) // 우클릭 시 팝업 출력
	{
		const EEquipSlot sourceSlot = GetSourceSlot();
		const int64 uid = owningInventory->GetEquippedInstanceUID(sourceSlot);
		if (uid == 0)
			return FReply::Handled();

		if (InMouseEvent.IsShiftDown())
			return FReply::Handled();

		if (owningInventoryWidget)
			owningInventoryWidget->OpenContextMenu(owningInventory, uid, sourceSlot, InMouseEvent.GetScreenSpacePosition());

		return FReply::Handled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) // 좌클릭 시 드래그
	{
		if (owningInventory->GetEquippedInstanceUID(GetSourceSlot()) == 0)
			return FReply::Unhandled();
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return FReply::Unhandled();
}

void UEquipmentSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!owningInventory || !owningInventoryWidget)
		return;

	owningInventoryWidget->BeginHoverInfo(owningInventory, owningInventory->GetEquippedInstanceUID(GetSourceSlot()));
}

void UEquipmentSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (owningInventoryWidget)
		owningInventoryWidget->CancelHoverInfo();
}

void UEquipmentSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!owningInventory)
		return;

	const EEquipSlot sourceSlot = GetSourceSlot();
	const int64 uid = owningInventory->GetEquippedInstanceUID(sourceSlot);
	if (uid == 0)
		return;

	UItemDragDrop* op = Cast<UItemDragDrop>(UWidgetBlueprintLibrary::CreateDragDropOperation(UItemDragDrop::StaticClass()));
	if (!op)
		return;

	op->instanceUID = uid;
	op->sourceType = EDragSource::EDS_EquipSlot;
	op->sourceSlot = sourceSlot;
	op->sourceInventory = owningInventory;
	op->grabOffsetX = 0;
	op->grabOffsetY = 0;
	op->bRotated = false;
	op->Pivot = EDragPivot::MouseDown;

	const float cellSize = owningInventoryWidget ? owningInventoryWidget->GetCellSize() : 100.f;

	int32 fw = 1, fh = 1;
	owningInventory->GetInstanceFootprint(uid, false, fw, fh);

	if (UImage* ghost = NewObject<UImage>(this))
	{
		const FItemInstance inst = owningInventory->FindInstance(uid);
		FItemData data;
		if (owningInventory->GetItemData(inst.itemId, data) && data.icon)
			ghost->SetBrushFromTexture(data.icon);

		ghost->SetRenderOpacity(0.6f);

		USizeBox* wrapper = NewObject<USizeBox>(this);
		if (wrapper)
		{
			wrapper->SetWidthOverride(fw * cellSize);
			wrapper->SetHeightOverride(fh * cellSize);
			wrapper->AddChild(ghost);
			op->DefaultDragVisual = wrapper;
		}
		else
		{
			op->DefaultDragVisual = ghost;
		}
	}

	OutOperation = op;
}

bool UEquipmentSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDrop* op = Cast<UItemDragDrop>(InOperation);
	if (!op || !owningInventory)
		return false;

	if (op->bUseOnly)
	{
		if (owningInventoryWidget)
			owningInventoryWidget->HideAllPlacementHighlights();

		const EBodyPart part = owningInventory->GetBodyPartForSlot(slotType);
		SetSlotHighlight(part != EBodyPart::EBP_None
			&& owningInventory->CanUseItemOnPart(op->instanceUID, part));
		return true;
	}

	if (op->sourceType == EDragSource::EDS_QuickSlot)
	{
		SetSlotHighlight(false);
		return true;
	}

	if (op->bSplitDrag)
	{
		SetSlotHighlight(false);
		return true;
	}

	if (owningInventoryWidget)
		owningInventoryWidget->HideAllPlacementHighlights();

	const FItemInstance inst = owningInventory->FindInstance(op->instanceUID);
	FItemData data;

	if (owningInventory->GetItemData(inst.itemId, data))
	{
		bool bCanAccept = false;
		if (data.category == EItemCategory::EIC_Equipment)
			bCanAccept = owningInventory->CanEquipToSlot(data.itemId, slotType);

		else if (data.category == EItemCategory::EIC_Consumable && data.effectType != EItemEffect::EIE_Ammo)
			bCanAccept = (owningInventory->GetBodyPartForSlot(slotType) != EBodyPart::EBP_None);

		SetSlotHighlight(bCanAccept);
	}

	return true;
}

void UEquipmentSlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	ClearSlotHighlight();
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDrop* op = Cast<UItemDragDrop>(InOperation);
	if (!op || !owningInventory)
		return false;

	if (op->bUseOnly)
	{
		ClearSlotHighlight();

		const EBodyPart part = owningInventory->GetBodyPartForSlot(slotType);
		if (part == EBodyPart::EBP_None)
			return false;

		if (!owningInventory->CanUseItemOnPart(op->instanceUID, part))
			return false;

		AWindlessCharacter* character = Cast<AWindlessCharacter>(owningInventory->GetOwner());
		if (!character)
			return false;

		const float castTime = owningInventory->GetItemCastTime(op->instanceUID);
		return character->BeginItemCast(op->instanceUID, part, castTime);
	}

	if (op->sourceType == EDragSource::EDS_QuickSlot)
	{
		ClearSlotHighlight();
		return false;
	}

	if (op->bSplitDrag)
	{
		ClearSlotHighlight();
		return false;
	}

	if (owningInventoryWidget)
		owningInventoryWidget->HideAllPlacementHighlights();

	ClearSlotHighlight();

	const FItemInstance inst = owningInventory->FindInstance(op->instanceUID);
	FItemData data;
	if (!owningInventory->GetItemData(inst.itemId, data))
		return false;

	if (data.category == EItemCategory::EIC_Equipment)
	{
		owningInventory->EquipItem(op->instanceUID, slotType);
		return true;
	}

	if (data.category == EItemCategory::EIC_Consumable && data.effectType != EItemEffect::EIE_Ammo)
	{
		const EBodyPart part = owningInventory->GetBodyPartForSlot(slotType);
		if (part != EBodyPart::EBP_None)
		{
			if (AWindlessCharacter* character = Cast<AWindlessCharacter>(owningInventory->GetOwner()))
			{
				const float castTime = owningInventory->GetItemCastTime(op->instanceUID);
				character->BeginItemCast(op->instanceUID, part, castTime);
			}
			return true;
		}
	}

	return true;
}

EEquipSlot UEquipmentSlotWidget::GetSourceSlot() const
{
	return owningInventory ? owningInventory->GetOccupyingSlot(slotType) : slotType;
}

void UEquipmentSlotWidget::SetSlotHighlight(bool bValid)
{
	if (!slotBackground)
		return;

	const FLinearColor color = bValid ? FLinearColor(0.10f, 0.45f, 0.12f) : FLinearColor(0.50f, 0.08f, 0.08f);
	slotBackground->SetBrush(WindlessUI::Box(WindlessUI::Alpha(color, 0.45f), WindlessUI::Alpha(color, 1.f)));
}

void UEquipmentSlotWidget::ClearSlotHighlight()
{
	if (slotBackground)
	{
		slotBackground->SetBrush(WindlessUI::Tile(0.06f, 0.45f));
		slotBackground->SetColorAndOpacity(FLinearColor::White);
	}
	WindlessUI::EmptyIcon(emptyIcon);
	WindlessUI::Bar(durabilityBar);
}
