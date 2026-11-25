#include "UI/SavePoint/RefineConfirmWidget.h"
#include "Components/RefineComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

void URefineConfirmWidget::InitializeConfirm(UInventoryComponent* inventory, int64 instanceUID)
{
	targetInventory = inventory;
	targetUID = instanceUID;

	URefineComponent* refine = GetOwningPlayerPawn() ? GetOwningPlayerPawn()->FindComponentByClass<URefineComponent>() : nullptr;
	if (!refine || !inventory)
		return;

	const FItemInstance inst = inventory->FindInstance(instanceUID);
	FItemData data;
	inventory->GetItemData(inst.itemId, data);

	const int32 value = refine->GetRefineValue(inventory, instanceUID);
	const int32 count = FMath::Max(1, inst.currentStackCount);

	if (messageText)
	{
		messageText->SetText(FText::FromString(FString::Printf(
			TEXT("%s x%d 을(를) 정제합니다.\n탈피석 %d 을(를) 얻습니다.\n되돌릴 수 없습니다."),
			*data.itemName.ToString(), count, value)));
	}

	if (cancelButton)
		cancelButton->SetKeyboardFocus();
}

void URefineConfirmWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);

	if (confirmButton)
		confirmButton->OnClicked.AddDynamic(this, &URefineConfirmWidget::HandleConfirm);
	if (cancelButton)
		cancelButton->OnClicked.AddDynamic(this, &URefineConfirmWidget::HandleCancel);
}

FReply URefineConfirmWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey key = InKeyEvent.GetKey();
	if (key == EKeys::Escape || key == EKeys::BackSpace || key == EKeys::Gamepad_FaceButton_Right)
	{
		HandleCancel();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void URefineConfirmWidget::HandleConfirm()
{
	URefineComponent* refine = GetOwningPlayerPawn() ? GetOwningPlayerPawn()->FindComponentByClass<URefineComponent>() : nullptr;
	if (refine)
		refine->Refine(targetInventory, targetUID);

	RemoveFromParent();
}

void URefineConfirmWidget::HandleCancel()
{
	RemoveFromParent();
}
