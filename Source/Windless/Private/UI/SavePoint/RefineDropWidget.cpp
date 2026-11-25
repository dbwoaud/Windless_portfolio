#include "UI/SavePoint/RefineDropWidget.h"
#include "UI/SavePoint/RefineConfirmWidget.h"
#include "UI/ItemDragDrop.h"
#include "Components/RefineComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "GameFramework/Pawn.h"

bool URefineDropWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDrop* op = Cast<UItemDragDrop>(InOperation);
	if (!op)
		return false;

	URefineComponent* refine = GetOwningPlayerPawn() ? GetOwningPlayerPawn()->FindComponentByClass<URefineComponent>() : nullptr;
	const bool bAllowed = refine && refine->CanRefine(op->sourceInventory, op->instanceUID);

	SetHighlight(true, bAllowed);
	return true;
}

void URefineDropWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	SetHighlight(false, false);
}

bool URefineDropWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	SetHighlight(false, false);

	UItemDragDrop* op = Cast<UItemDragDrop>(InOperation);
	if (!op || !confirmWidgetClass)
		return false;

	URefineComponent* refine = GetOwningPlayerPawn() ? GetOwningPlayerPawn()->FindComponentByClass<URefineComponent>() : nullptr;
	if (!refine || !refine->CanRefine(op->sourceInventory, op->instanceUID))
	{
		if (hintText)
			hintText->SetText(refusalText);
		return true;
	}

	URefineConfirmWidget* confirm = CreateWidget<URefineConfirmWidget>(GetOwningPlayer(), confirmWidgetClass);
	if (!confirm)
		return true;

	confirm->InitializeConfirm(op->sourceInventory, op->instanceUID);
	confirm->AddToViewport(confirmZOrder);
	return true;
}

void URefineDropWidget::SetHighlight(bool bActive, bool bAllowed)
{
	if (!glowImage)
		return;

	glowImage->SetVisibility(bActive ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	glowImage->SetColorAndOpacity(bAllowed ? FLinearColor(1.f, 0.72f, 0.42f, 1.f) : FLinearColor(0.75f, 0.32f, 0.29f, 1.f));
}
