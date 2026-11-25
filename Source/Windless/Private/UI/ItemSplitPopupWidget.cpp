#include "UI/ItemSplitPopupWidget.h"
#include "UI/WindlessUIStyle.h"
#include "UI/InventoryWidget.h"
#include "Components/InventoryComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "Components/Border.h"
#include "Engine/Engine.h"

void UItemSplitPopupWidget::SetupPopup(UInventoryWidget* inOwnerWidget, UInventoryComponent* inInventory, int64 uid, int32 inTargetX, int32 inTargetY, bool bInRotated)
{
	owningInventoryWidget = inOwnerWidget;
	owningInventory = inInventory;
	targetUID = uid;

	targetX = inTargetX;
	targetY = inTargetY;
	bTargetRotated = bInRotated;

	if (!owningInventory)
	{
		ClosePopup();
		return;
	}

	const FItemInstance inst = owningInventory->FindInstance(targetUID);
	FItemData data;
	if (!inst.IsValid() || !owningInventory->GetItemData(inst.itemId, data))
	{
		ClosePopup();
		return;
	}

	maxSplitAmount = FMath::Max(1, inst.currentStackCount - 1);
	splitAmount = FMath::Max(1, inst.currentStackCount / 2);

	if (headerText)
		headerText->SetText(data.itemName);

	if (amountSlider)
	{
		amountSlider->SetMinValue(1.f);
		amountSlider->SetMaxValue(static_cast<float>(maxSplitAmount));
		amountSlider->SetStepSize(1.f);
		amountSlider->SetValue(static_cast<float>(splitAmount));
	}

	RefreshAmountText();
	SetKeyboardFocus();
}

void UItemSplitPopupWidget::ClosePopup()
{
	if (owningInventoryWidget)
		owningInventoryWidget->CloseSplitPopup();
	else
		RemoveFromParent();
}

void UItemSplitPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WindlessUI::StyleTree(this);
	SetIsFocusable(true);

	if (amountSlider)
		amountSlider->OnValueChanged.AddDynamic(this, &UItemSplitPopupWidget::HandleSliderChanged);
	if (confirmButton)
		confirmButton->OnClicked.AddDynamic(this, &UItemSplitPopupWidget::HandleConfirmClicked);
	if (cancelButton)
		cancelButton->OnClicked.AddDynamic(this, &UItemSplitPopupWidget::HandleCancelClicked);
	if (minusButton)
		minusButton->OnClicked.AddDynamic(this, &UItemSplitPopupWidget::HandleMinusClicked);
	if (plusButton)
		plusButton->OnClicked.AddDynamic(this, &UItemSplitPopupWidget::HandlePlusClicked);
}

FReply UItemSplitPopupWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey key = InKeyEvent.GetKey();

	if (key == EKeys::Escape)
	{
		ClosePopup();
		return FReply::Handled();
	}

	if (key == EKeys::Enter)
	{
		HandleConfirmClicked();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UItemSplitPopupWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (popupBorder && popupBorder->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		return FReply::Handled();

	ClosePopup();
	return FReply::Handled();
}

void UItemSplitPopupWidget::HandleSliderChanged(float value)
{
	splitAmount = FMath::Clamp(FMath::RoundToInt(value), 1, maxSplitAmount);
	RefreshAmountText();
}

void UItemSplitPopupWidget::HandleMinusClicked()
{
	splitAmount = FMath::Clamp(splitAmount - 1, 1, maxSplitAmount);
	if (amountSlider)
		amountSlider->SetValue(static_cast<float>(splitAmount));

	RefreshAmountText();
}

void UItemSplitPopupWidget::HandlePlusClicked()
{
	splitAmount = FMath::Clamp(splitAmount + 1, 1, maxSplitAmount);
	if (amountSlider)
		amountSlider->SetValue(static_cast<float>(splitAmount));

	RefreshAmountText();
}

void UItemSplitPopupWidget::HandleConfirmClicked()
{
	if (!owningInventory)
	{
		ClosePopup();
		return;
	}

	const bool bPositioned = (targetX >= 0 && targetY >= 0);
	const bool bCanSplit = bPositioned
		? owningInventory->CanSplitStackAt(targetUID, splitAmount, targetX, targetY, bTargetRotated)
		: owningInventory->CanSplitStack(targetUID, splitAmount);

	if (!bCanSplit)
		return;
	
	const bool bSucceeded = bPositioned
		? owningInventory->SplitStack(targetUID, splitAmount, targetX, targetY, bTargetRotated)
		: owningInventory->SplitStackAuto(targetUID, splitAmount);

	if (!bSucceeded)
		return;

	ClosePopup();
}

void UItemSplitPopupWidget::HandleCancelClicked()
{
	ClosePopup();
}

void UItemSplitPopupWidget::RefreshAmountText()
{
	if (!amountText || !owningInventory)
		return;

	const FItemInstance inst = owningInventory->FindInstance(targetUID);
	amountText->SetText(FText::FromString(FString::Printf(TEXT("%d  /  %d"), splitAmount, inst.currentStackCount)));
}