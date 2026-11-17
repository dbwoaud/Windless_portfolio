#include "UI/ItemGridWidget.h"
#include "UI/WindlessUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "UI/InventoryWidget.h"
#include "UI/ItemSlotWidget.h"
#include "UI/ItemDragDrop.h"
#include "Components/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

void UItemGridWidget::InitializeGrid(UInventoryComponent* inventory, UInventoryWidget* screenWidget)
{
	if (owningInventory != inventory)
	{
		builtGridWidth = 0;
		builtGridHeight = 0;
	}


	owningInventory = inventory;
	owningScreen = screenWidget;

	if (!owningInventory)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::Visible);
	CreatePlacementHighlight();
	RefreshGrid();
}

void UItemGridWidget::RefreshGrid()
{
	if (!owningInventory)
		return;

	BuildBackgroundGrid();
	RebuildItems();
}

void UItemGridWidget::HidePlacementHighlight()
{
	if (placementHighlight)
		placementHighlight->SetVisibility(ESlateVisibility::Collapsed);
}

void UItemGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CreatePlacementHighlight();
}

bool UItemGridWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UItemDragDrop* op = Cast<UItemDragDrop>(InOperation))
	{
		if (op->bUseOnly)
		{
			HidePlacementHighlight();
			return true;
		}

		UpdatePlacementHighlight(InDragDropEvent.GetScreenSpacePosition(), op);
		return true;
	}

	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}

void UItemGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	HidePlacementHighlight();
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

bool UItemGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	HidePlacementHighlight();

	UItemDragDrop* op = Cast<UItemDragDrop>(InOperation);
	if (!op || !owningInventory || op->bUseOnly)
		return false;

	int32 cellX, cellY;
	const bool bInside = ResolveDropCell(InGeometry, InDragDropEvent.GetScreenSpacePosition(), op, cellX, cellY);

	UInventoryComponent* source = op->sourceInventory;
	if (!source)
		return false;

	/* 컨테이너 간 이동 */
	if (source != owningInventory)
	{
		if (!bInside)
			return false;

		if (op->sourceType == EDragSource::EDS_EquipSlot || op->sourceType == EDragSource::EDS_QuickSlot)
			return false;

		if (source->TransferInstanceTo(owningInventory, op->instanceUID, cellX, cellY, op->bRotated))
			return true;

		/* 배치 실패 시 대상 칸의 아이템과 스택 합치기 시도 */
		const FGeometry canvasGeo = itemCanvas->GetCachedGeometry();
		const FVector2D local = canvasGeo.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
		const int32 targetX = FMath::FloorToInt(local.X / cellSize);
		const int32 targetY = FMath::FloorToInt(local.Y / cellSize);
		const int64 targetUID = owningInventory->FindInstanceUIDAtCell(targetX, targetY);

		if (targetUID == 0)
			return false;

		return source->TransferMergeTo(owningInventory, op->instanceUID, targetUID);
	}

	/* 같은 컨테이너 내부 */
	switch (op->sourceType)
	{
		case EDragSource::EDS_Grid:
		{
			if (!bInside)
				return false;

			/* 나누기 드래그 */
			if (op->bSplitDrag)
			{
				const FItemInstance srcInst = owningInventory->FindInstance(op->instanceUID);
				if (!owningInventory->CanPlaceNewInstanceAt(srcInst.itemId, cellX, cellY, op->bRotated))
					return false;

				if (owningScreen)
					owningScreen->OpenSplitPopup(owningInventory, op->instanceUID, cellX, cellY, op->bRotated);

				return true;
			}

			/* 이동 */
			if (owningInventory->MoveItem(op->instanceUID, cellX, cellY, op->bRotated))
				return true;

			const FGeometry canvasGeo = itemCanvas->GetCachedGeometry();
			const FVector2D local = canvasGeo.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
			const int32 targetX = FMath::FloorToInt(local.X / cellSize);
			const int32 targetY = FMath::FloorToInt(local.Y / cellSize);
			const int64 targetUID = owningInventory->FindInstanceUIDAtCell(targetX, targetY);

			if (targetUID == 0 || targetUID == op->instanceUID)
				return false;

			if (owningInventory->MergeStacks(op->instanceUID, targetUID))
				return true;

			return owningInventory->SwapItems(op->instanceUID, targetUID);
		}

		case EDragSource::EDS_EquipSlot:
			return owningInventory->UnequipSlot(op->sourceSlot);

		case EDragSource::EDS_QuickSlot:
			return owningInventory->UnequipQuickSlot(op->sourceQuickSlot);

		default:
			return false;
	}
}

void UItemGridWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	HidePlacementHighlight();
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
}

void UItemGridWidget::BuildBackgroundGrid()
{
	if (!backgroundGrid || !backgroundCellClass || !owningInventory)
		return;

	const int32 gridWidth = owningInventory->GetGridWidth();
	const int32 gridHeight = owningInventory->GetGridHeight();

	if (gridWidth == builtGridWidth && gridHeight == builtGridHeight)
		return;

	backgroundGrid->ClearChildren();

	for (int32 y = 0; y < gridHeight; ++y)
	{
		for (int32 x = 0; x < gridWidth; ++x)
		{
			UUserWidget* cell = CreateWidget<UUserWidget>(GetOwningPlayer(), backgroundCellClass);
			if (!cell)
				continue;

			// 칸: 옅은 금색 선과 어두운 속 (세이브포인트 UI 색)
			if (cell->WidgetTree)
			{
				if (USizeBox* box = Cast<USizeBox>(cell->WidgetTree->FindWidget(TEXT("CellBox")))) // 칸 위젯(56)을 칸 크기에 맞춰 끝 줄이 잘리지 않게
				{
					box->SetWidthOverride(cellSize);
					box->SetHeightOverride(cellSize);
				}
				if (UImage* line = Cast<UImage>(cell->WidgetTree->FindWidget(TEXT("BorderImage"))))
					line->SetColorAndOpacity(WindlessUI::Alpha(WindlessUI::GoldDim, 0.22f));
				if (UImage* inside = Cast<UImage>(cell->WidgetTree->FindWidget(TEXT("CellImage"))))
					inside->SetColorAndOpacity(FLinearColor(0.012f, 0.011f, 0.008f, 0.85f));
			}

			if (UUniformGridSlot* slot = backgroundGrid->AddChildToUniformGrid(cell, y, x))
			{
				slot->SetHorizontalAlignment(HAlign_Fill);
				slot->SetVerticalAlignment(VAlign_Fill);
			}
		}
	}

	builtGridWidth = gridWidth;
	builtGridHeight = gridHeight;

	if (gridSizeBox)
	{
		gridSizeBox->SetWidthOverride(gridWidth * cellSize);
		gridSizeBox->SetHeightOverride(gridHeight * cellSize);
	}
}

void UItemGridWidget::RebuildItems()
{
	if (!itemCanvas || !owningInventory || !slotWidgetClass)
		return;

	for (UItemSlotWidget* widget : spawnedItemWidgets)
	{
		if (widget)
			widget->RemoveFromParent();
	}
	spawnedItemWidgets.Empty();

	for (const FItemInstance& inst : owningInventory->GetItems())
	{
		if (inst.bIsEquipped || inst.quickSlot != EQuickSlot::EQS_None)
			continue;

		UItemSlotWidget* slotWidget = CreateWidget<UItemSlotWidget>(GetOwningPlayer(), slotWidgetClass);
		if (!slotWidget)
			continue;

		slotWidget->SetOwningInventoryWidget(owningScreen);
		slotWidget->InitializeItem(owningInventory, inst.instanceUID, cellSize);

		int32 w, h;
		slotWidget->GetFootprint(w, h);

		if (UCanvasPanelSlot* canvasSlot = itemCanvas->AddChildToCanvas(slotWidget))
		{
			canvasSlot->SetAutoSize(false);
			canvasSlot->SetPosition(FVector2D(inst.gridPosX * cellSize, inst.gridPosY * cellSize));
			canvasSlot->SetSize(FVector2D(w * cellSize, h * cellSize));
		}

		spawnedItemWidgets.Add(slotWidget);
	}
}

void UItemGridWidget::CreatePlacementHighlight()
{
	if (!itemCanvas || placementHighlight)
		return;

	placementHighlight = NewObject<UImage>(this);
	if (!placementHighlight)
		return;

	placementHighlight->SetVisibility(ESlateVisibility::Collapsed);
	if (UCanvasPanelSlot* slot = itemCanvas->AddChildToCanvas(placementHighlight))
	{
		slot->SetAutoSize(false);
		slot->SetZOrder(-1);
	}
}

void UItemGridWidget::UpdatePlacementHighlight(const FVector2D& screenPos, UItemDragDrop* op)
{
	if (!placementHighlight || !owningInventory || !op)
		return;

	int32 cellX, cellY;
	if (!ResolveDropCell(GetCachedGeometry(), screenPos, op, cellX, cellY))
	{
		HidePlacementHighlight();
		return;
	}

	UInventoryComponent* source = op->sourceInventory;
	const bool bCrossContainer = (source && source != owningInventory);

	int32 w = 1, h = 1;
	bool bValid = false;

	/* 다른 컨테이너에서 넘어온 경우 */
	if (bCrossContainer)
	{
		const FItemInstance srcInst = source->FindInstance(op->instanceUID);
		owningInventory->GetNewInstanceFootprint(srcInst.itemId, op->bRotated, w, h);
		bValid = owningInventory->CanAcceptInstanceAt(srcInst, cellX, cellY, op->bRotated);
	}
	/* 한 컨테이너의 이동인 경우 */
	else
	{
		owningInventory->GetInstanceFootprint(op->instanceUID, op->bRotated, w, h);

		/* 나누기 드래그 */
		if (op->bSplitDrag)
		{
			const FItemInstance srcInst = owningInventory->FindInstance(op->instanceUID);
			bValid = owningInventory->IsSplittable(op->instanceUID)
				&& owningInventory->CanPlaceNewInstanceAt(srcInst.itemId, cellX, cellY, op->bRotated);
		}
		/* 이동 */
		else
			bValid = owningInventory->CanPlaceInstanceAt(op->instanceUID, cellX, cellY, op->bRotated);
	}

	placementHighlight->SetColorAndOpacity(bValid ? validColor : invalidColor);

	if (UCanvasPanelSlot* slot = Cast<UCanvasPanelSlot>(placementHighlight->Slot))
	{
		slot->SetPosition(FVector2D(cellX * cellSize, cellY * cellSize));
		slot->SetSize(FVector2D(w * cellSize, h * cellSize));
	}

	placementHighlight->SetVisibility(ESlateVisibility::HitTestInvisible);
}

bool UItemGridWidget::ResolveDropCell(const FGeometry& InGeometry, const FVector2D& screenPos, UItemDragDrop* op, int32& outX, int32& outY) const
{
	if (!itemCanvas || !op)
		return false;

	const FGeometry canvasGeo = itemCanvas->GetCachedGeometry();
	const FVector2D local = canvasGeo.AbsoluteToLocal(screenPos);
	const FVector2D canvasSize = canvasGeo.GetLocalSize();

	if (local.X < 0.f || local.Y < 0.f || local.X >= canvasSize.X || local.Y >= canvasSize.Y)
		return false;

	outX = FMath::FloorToInt(local.X / cellSize) - op->grabOffsetX;
	outY = FMath::FloorToInt(local.Y / cellSize) - op->grabOffsetY;
	return true;
}
