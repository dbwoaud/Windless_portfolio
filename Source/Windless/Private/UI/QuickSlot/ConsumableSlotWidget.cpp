#include "UI/QuickSlot/ConsumableSlotWidget.h"
#include "UI/WindlessUIStyle.h"
#include "UI/InventoryWidget.h"
#include "UI/ItemDragDrop.h"
#include "Components/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Windless/DebugMacro.h"

void UConsumableSlotWidget::InitializeSlot(UInventoryComponent* inventory, UInventoryWidget* ownerWidget)
{
	owningInventory = inventory;
	owningInventoryWidget = ownerWidget;
	ClearSlotHighlight(); // 기본 칸 모양(공용 스타일)을 입힌다

	if (keyText)
	{
		FString keyLabel;
		switch (slotType)
		{
			case EQuickSlot::EQS_Recovery: keyLabel = TEXT("G"); break;
			case EQuickSlot::EQS_Consume1: keyLabel = TEXT("1"); break;
			case EQuickSlot::EQS_Consume2: keyLabel = TEXT("2"); break;
			case EQuickSlot::EQS_Consume3: keyLabel = TEXT("3"); break;
			case EQuickSlot::EQS_Consume4: keyLabel = TEXT("4"); break;
			default: break;
		}
		keyText->SetText(FText::FromString(keyLabel));
	}

	ClearSlotHighlight();
	RefreshSlot();
}

void UConsumableSlotWidget::RefreshSlot()
{
	if (!owningInventory)
		return;

	const int64 uid = owningInventory->GetQuickSlotInstanceUID(slotType);
	const bool bHasItem = (uid != 0);

	if (bHasItem)
	{
		const FItemInstance inst = owningInventory->FindInstance(uid);
		FItemData data;
		owningInventory->GetItemData(inst.itemId, data);

		if (icon)
		{
			icon->SetBrushFromTexture(data.icon);
			icon->SetVisibility(data.icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (countText)
		{
			countText->SetText(FText::AsNumber(inst.currentStackCount));
			countText->SetVisibility(ESlateVisibility::HitTestInvisible);
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
		if (countText)
			countText->SetVisibility(ESlateVisibility::Collapsed);
		if (emptyIcon)
			emptyIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

FReply UConsumableSlotWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	/* 장착 해제 */
	if (owningInventory && InKeyEvent.GetKey() == EKeys::F) 
	{
		if (slotType == EQuickSlot::EQS_Recovery)
			return FReply::Handled();

		if (owningInventory->GetQuickSlotInstanceUID(slotType) != 0)
		{
			owningInventory->UnequipQuickSlot(slotType);
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UConsumableSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!owningInventory)
		return FReply::Handled();

	const int64 uid = owningInventory->GetQuickSlotInstanceUID(slotType);
	if (uid == 0)
		return FReply::Unhandled();

	/* 우클릭 시 팝업 출력 */
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (InMouseEvent.IsShiftDown())
			return FReply::Handled();

		if (owningInventoryWidget)
			owningInventoryWidget->OpenContextMenu(owningInventory, uid, EEquipSlot::EES_None, InMouseEvent.GetScreenSpacePosition());

		return FReply::Handled();
	}

	/* 좌클릭 시 드래그 시작 */
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;

	return FReply::Unhandled();
}

void UConsumableSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!owningInventory || !owningInventoryWidget)
		return;

	const int64 uid = owningInventory->GetQuickSlotInstanceUID(slotType);
	owningInventoryWidget->SetHoveredItem(owningInventory, uid);
	owningInventoryWidget->BeginHoverInfo(owningInventory, uid);
}

void UConsumableSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (!owningInventoryWidget)
		return;

	owningInventoryWidget->SetHoveredItem(nullptr, 0);
	owningInventoryWidget->CancelHoverInfo();
}

void UConsumableSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!owningInventory)
		return;

	const int64 uid = owningInventory->GetQuickSlotInstanceUID(slotType);
	if (uid == 0)
		return;

	UItemDragDrop* op = Cast<UItemDragDrop>(UWidgetBlueprintLibrary::CreateDragDropOperation(UItemDragDrop::StaticClass()));
	if (!op)
		return;

	op->instanceUID = uid;
	op->sourceType = EDragSource::EDS_QuickSlot;
	op->sourceQuickSlot = slotType;
	op->sourceInventory = owningInventory;
	op->grabOffsetX = 0;
	op->grabOffsetY = 0;
	op->bRotated = false;
	op->bUseOnly = (slotType == EQuickSlot::EQS_Recovery);
	op->Pivot = EDragPivot::MouseDown;

	const float cellSize = owningInventoryWidget ? owningInventoryWidget->GetCellSize() : 48.f;

	int32 fw = 1, fh = 1;
	owningInventory->GetInstanceFootprint(uid, false, fw, fh);

	if (UImage* ghost = NewObject<UImage>(this))
	{
		const FItemInstance inst = owningInventory->FindInstance(uid);
		FItemData data;
		if (owningInventory->GetItemData(inst.itemId, data) && data.icon)
			ghost->SetBrushFromTexture(data.icon);

		ghost->SetRenderOpacity(0.6f);

		if (USizeBox* wrapper = NewObject<USizeBox>(this))
		{
			wrapper->SetWidthOverride(fw * cellSize);
			wrapper->SetHeightOverride(fh * cellSize);
			wrapper->AddChild(ghost);
			op->DefaultDragVisual = wrapper;
		}
		else
			op->DefaultDragVisual = ghost;
	}

	if (owningInventoryWidget)
		owningInventoryWidget->BeginConsumableDrag(op);

	OutOperation = op;
}

bool UConsumableSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
		UItemDragDrop* op = Cast<UItemDragDrop>(InOperation);
	if (!op || !owningInventory || op->bUseOnly)
		return false;

	if (owningInventoryWidget)
		owningInventoryWidget->HideAllPlacementHighlights();

	if (op->bSplitDrag || op->sourceInventory != owningInventory)
	{
		SetSlotHighlight(false);
		return true;
	}

	const FItemInstance inst = op->sourceInventory->FindInstance(op->instanceUID);
	SetSlotHighlight(owningInventory->CanAssignToQuickSlot(inst.itemId, slotType));
	return true;
}

void UConsumableSlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	ClearSlotHighlight();
}

bool UConsumableSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	ClearSlotHighlight();

	UItemDragDrop* op = Cast<UItemDragDrop>(InOperation);
	if (!op || !owningInventory || op->bUseOnly)
		return false;

	if (owningInventoryWidget)
		owningInventoryWidget->HideAllPlacementHighlights();

	if (op->bSplitDrag || op->sourceInventory != owningInventory)
		return false;

	return owningInventory->EquipToQuickSlot(op->instanceUID, slotType);
}

void UConsumableSlotWidget::SetSlotHighlight(bool bValid)
{
	if (slotBackground)
	{
		const FLinearColor color = bValid ? validColor : invalidColor;
		slotBackground->SetBrush(WindlessUI::Box(WindlessUI::Alpha(color, 0.45f), WindlessUI::Alpha(color, 1.f)));
	}
}

void UConsumableSlotWidget::ClearSlotHighlight()
{
	if (slotBackground)
	{
		slotBackground->SetBrush(WindlessUI::Tile(0.06f, 0.45f));
		slotBackground->SetColorAndOpacity(FLinearColor::White);
	}
	WindlessUI::EmptyIcon(emptyIcon);
	WindlessUI::Text(keyText, 10.f, 0, WindlessUI::Gold);
	WindlessUI::Text(countText, 10.f, 0, WindlessUI::Cream);
}
