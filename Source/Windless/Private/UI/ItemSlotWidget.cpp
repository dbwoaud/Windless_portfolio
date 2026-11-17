#include "UI/ItemSlotWidget.h"
#include "UI/WindlessUIStyle.h"
#include "UI/InventoryWidget.h"
#include "UI/ItemDragDrop.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"

void UItemSlotWidget::InitializeItem(UInventoryComponent* inventory, int64 uid, float inCellSize)
{
	owningInventory = inventory;
	instanceUID = uid;
	cellSize = inCellSize;

	SetVisibility(ESlateVisibility::Visible);
	if (rootSizeBox)
	{
		int32 w, h;
		GetFootprint(w, h);
		rootSizeBox->SetWidthOverride(w * cellSize);
		rootSizeBox->SetHeightOverride(h * cellSize);
	}
	RefreshSlot();
}

void UItemSlotWidget::RefreshSlot()
{
	if (!owningInventory)
		return;

	const FItemInstance inst = owningInventory->FindInstance(instanceUID);
	FItemData data;
	const bool bValid = inst.IsValid() && owningInventory->GetItemData(inst.itemId, data);

	if (icon)
	{
		if (bValid && data.icon)
		{
			icon->SetBrushFromTexture(data.icon);
			icon->SetVisibility(ESlateVisibility::HitTestInvisible);
			icon->SetRenderTransformAngle(inst.bIsRotated ? 90.f : 0.f);
		}
		else
		{
			icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (gradeBorder && bValid)
	{
		const bool bBroken = owningInventory->IsInstanceBroken(instanceUID);
		// 칸마다 얇은 금색 테두리: 붙어 있는 아이템이 한 덩어리로 보이지 않게 한다
		gradeBorder->SetBrush(WindlessUI::Box(bBroken
			? FLinearColor(0.18f, 0.02f, 0.02f, 0.95f)
			: FLinearColor(0.035f, 0.03f, 0.022f, 0.96f), WindlessUI::Alpha(WindlessUI::GoldDim, 0.55f)));
		gradeBorder->SetColorAndOpacity(FLinearColor::White);
	}

	// 아이콘이 없는 아이템은 이름을 적는다
	WindlessUI::NameLabel(this, icon, bValid && !data.icon ? data.itemName : FText::GetEmpty());

	if (quantityText)
	{
		const bool bLimited = bValid && owningInventory->IsCarryLimited(inst.itemId);
		if (bLimited)
		{
			const int32 total = owningInventory->GetItemTotalCount(inst.itemId);
			const int32 limit = owningInventory->GetCarryLimit(inst.itemId);

			const FString text = (inst.currentStackCount > 1)
				? FString::Printf(TEXT("%d  (%d/%d)"), inst.currentStackCount, total, limit)
				: FString::Printf(TEXT("(%d/%d)"), total, limit);

			quantityText->SetText(FText::FromString(text));
			quantityText->SetColorAndOpacity(FSlateColor(total >= limit ? FLinearColor(1.f, 0.45f, 0.1f, 1.f) : WindlessUI::Cream));
			quantityText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else if (bValid && inst.currentStackCount > 1)
		{
			quantityText->SetText(FText::AsNumber(inst.currentStackCount));
			quantityText->SetColorAndOpacity(FSlateColor(WindlessUI::Cream));
			quantityText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
			quantityText->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (durabilityBar)
	{
		if (bValid && data.maxDurability > 0.f)
		{
			durabilityBar->SetPercent(inst.currentDurability / data.maxDurability);
			durabilityBar->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			durabilityBar->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UItemSlotWidget::GetFootprint(int32& outW, int32& outH) const
{
	outW = 1;
	outH = 1;
	if (!owningInventory)
		return;

	const FItemInstance inst = owningInventory->FindInstance(instanceUID);
	FItemData data;
	if (!owningInventory->GetItemData(inst.itemId, data))
		return;

	if (inst.bIsRotated)
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

void UItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WindlessUI::Bar(durabilityBar);
	RefreshSlot();
}

FReply UItemSlotWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!owningInventory)
		return FReply::Unhandled();

	const FKey key = InKeyEvent.GetKey();

	if (key == EKeys::F) // 마우스가 올라간 아이템을 장착
	{
		const FItemInstance inst = owningInventory->FindInstance(instanceUID);
		FItemData data;
		if (owningInventory->GetItemData(inst.itemId, data))
		{
			if (data.category == EItemCategory::EIC_Equipment) // 장비 아이템 장착
			{
				owningInventory->EquipItem(instanceUID);
				return FReply::Handled();
			}
			if (data.category == EItemCategory::EIC_Consumable) // 소모품 아이템 장착
			{
				owningInventory->QuickEquipToQuickSlot(instanceUID);
				return FReply::Handled();
			}
		}
	}

	if (key == EKeys::Q) // 마우스가 올라간 아이템을 버리기
	{
		owningInventory->DropItem(instanceUID);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (owningInventoryWidget)
		owningInventoryWidget->CancelHoverInfo();

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton) // 우클릭 시 팝업 출력
	{
		if (!owningInventory)
			return FReply::Handled();

		if (InMouseEvent.IsShiftDown()) // 빠른 보관 수행
		{
			if (owningInventoryWidget)
				owningInventoryWidget->RequestQuickTransfer(owningInventory, instanceUID);
			return FReply::Handled();
		}

		if (owningInventoryWidget)
			owningInventoryWidget->OpenContextMenu(owningInventory, instanceUID, EEquipSlot::EES_None, InMouseEvent.GetScreenSpacePosition());

		return FReply::Handled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) // 좌클릭 시 드래그 시작
	{
		bPendingSplitDrag = InMouseEvent.IsShiftDown() && owningInventory && owningInventory->IsSplittable(instanceUID);
		const FVector2D local = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

		grabOffsetX = FMath::Max(0, FMath::FloorToInt(local.X / cellSize));
		grabOffsetY = FMath::Max(0, FMath::FloorToInt(local.Y / cellSize));

		if (bPendingSplitDrag)
		{
			grabOffsetX = 0;
			grabOffsetY = 0;
		}

		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return FReply::Unhandled();
}

void UItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (!owningInventoryWidget)
		return;

	owningInventoryWidget->SetHoveredItem(owningInventory, instanceUID);
	owningInventoryWidget->BeginHoverInfo(owningInventory, instanceUID);
}

void UItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (!owningInventoryWidget)
		return;

	owningInventoryWidget->SetHoveredItem(nullptr, 0);
	owningInventoryWidget->CancelHoverInfo();
}

void UItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (owningInventoryWidget)
		owningInventoryWidget->CancelHoverInfo();

	if (!owningInventory)
		return;

	UItemDragDrop* op = Cast<UItemDragDrop>(UWidgetBlueprintLibrary::CreateDragDropOperation(UItemDragDrop::StaticClass()));
	if (!op)
		return;

	const FItemInstance inst = owningInventory->FindInstance(instanceUID);

	op->instanceUID = instanceUID;
	op->sourceType = EDragSource::EDS_Grid;
	op->sourceInventory = owningInventory;
	op->grabOffsetX = grabOffsetX;
	op->grabOffsetY = grabOffsetY;
	op->bRotated = inst.bIsRotated;
	op->bSplitDrag = bPendingSplitDrag;
	op->Pivot = EDragPivot::MouseDown;

	bPendingSplitDrag = false;
	if (UItemSlotWidget* ghost = CreateWidget<UItemSlotWidget>(GetOwningPlayer(), GetClass()))
	{
		ghost->InitializeItem(owningInventory, instanceUID, cellSize);
		ghost->SetRenderOpacity(0.6f);

		int32 fw, fh;
		ghost->GetFootprint(fw, fh);

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

	if (owningInventoryWidget)
		owningInventoryWidget->BeginConsumableDrag(op);

	OutOperation = op;
}