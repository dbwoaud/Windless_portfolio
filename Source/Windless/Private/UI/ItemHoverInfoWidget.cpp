#include "UI/ItemHoverInfoWidget.h"
#include "UI/WindlessUIStyle.h"
#include "UI/InventoryWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Brushes/SlateNoResource.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UItemHoverInfoWidget::UpdatePosition()
{
	if (!popupBorder)
		return;

	UCanvasPanelSlot* borderSlot = Cast<UCanvasPanelSlot>(popupBorder->Slot);
	if (!borderSlot)
		return;

	if (bDocked)
	{
		popupBorder->SetBrush(FSlateNoResource()); // 설명 칸에 붙을 때는 팝업 틀을 그리지 않는다
		popupBorder->SetPadding(FMargin(0.f));
		borderSlot->SetAutoSize(true);
		borderSlot->SetAlignment(FVector2D::ZeroVector);
		borderSlot->SetPosition(FVector2D::ZeroVector);
		return;
	}

	popupBorder->ForceLayoutPrepass();

	const FVector2D popupSize = popupBorder->GetDesiredSize();
	const FVector2D mousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	const FVector2D viewportSize = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).GetLocalSize();

	FVector2D finalPos = mousePos + cursorOffset;
	finalPos.X = FMath::Clamp(finalPos.X, screenEdgePadding.X, FMath::Max(screenEdgePadding.X, viewportSize.X - popupSize.X - screenEdgePadding.X));
	finalPos.Y = FMath::Clamp(finalPos.Y, screenEdgePadding.Y, FMath::Max(screenEdgePadding.Y, viewportSize.Y - popupSize.Y - screenEdgePadding.Y));

	borderSlot->SetAutoSize(true);
	borderSlot->SetAlignment(FVector2D::ZeroVector);
	borderSlot->SetPosition(finalPos);
}

void UItemHoverInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WindlessUI::StyleTree(this);
	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetIsFocusable(false);
	UpdatePosition();
}

void UItemHoverInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bFollowCursor)
		UpdatePosition();
}

void UItemHoverInfoWidget::HandleInvalidTarget()
{
	if (owningInventoryWidget)
		owningInventoryWidget->CancelHoverInfo();
	else
		RemoveFromParent();
}