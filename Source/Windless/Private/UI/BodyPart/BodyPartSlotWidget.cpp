#include "UI/BodyPart/BodyPartSlotWidget.h"
#include "UI/InventoryWidget.h"
#include "UI/ItemDragDrop.h"
#include "Characters/WindlessCharacter.h"
#include "Components/InventoryComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UBodyPartSlotWidget::InitializeSlot(UInventoryComponent* inventory, UBodyPartComponent* bodyParts, UInventoryWidget* ownerWidget)
{
	owningInventory = inventory;
	owningBodyParts = bodyParts;
	owningInventoryWidget = ownerWidget;

	ClearHighlight();
	RefreshSlot();
}

void UBodyPartSlotWidget::RefreshSlot()
{
	if (!owningBodyParts)
		return;

	const float percent = owningBodyParts->GetPartHealthPercent(partType);

	if (hpBar)
		hpBar->SetPercent(percent);

	if (hpText)
	{
		hpText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"),
			FMath::RoundToInt(owningBodyParts->GetPartHealth(partType)),
			FMath::RoundToInt(owningBodyParts->GetPartMaxHealth(partType)))));
	}
}

void UBodyPartSlotWidget::ClearHighlight()
{
	if (partImage)
		partImage->SetColorAndOpacity(normalColor);
}

bool UBodyPartSlotWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDrop* op = Cast<UItemDragDrop>(InOperation);
	if (!op || !owningInventory)
		return false;

	if (owningInventoryWidget)
		owningInventoryWidget->HideAllPlacementHighlights();

	SetHighlight(owningInventory->CanUseItemOnPart(op->instanceUID, partType));
	return true;
}

void UBodyPartSlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	ClearHighlight();
}

bool UBodyPartSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	ClearHighlight();

	UItemDragDrop* op = Cast<UItemDragDrop>(InOperation);
	if (!op || !owningInventory)
		return false;

	if (!owningInventory->CanUseItemOnPart(op->instanceUID, partType))
		return false;

	AWindlessCharacter* character = Cast<AWindlessCharacter>(owningInventory->GetOwner());
	if (!character)
		return false;

	const float castTime = owningInventory->GetItemCastTime(op->instanceUID);
	return character->BeginItemCast(op->instanceUID, partType, castTime);
}

void UBodyPartSlotWidget::SetHighlight(bool bUsable)
{
	if (partImage)
		partImage->SetColorAndOpacity(bUsable ? usableColor : unusableColor);
}
