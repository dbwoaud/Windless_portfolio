#include "UI/ItemInfoPopupWidget.h"
#include "UI/WindlessUIStyle.h"
#include "UI/InventoryWidget.h"
#include "Components/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Button.h"

void UItemInfoPopupWidget::SetupInfo(UInventoryWidget* inOwnerWidget, UInventoryComponent* inInventory, int64 uid)
{
	Super::SetupInfo(inOwnerWidget, inInventory, uid);
	SetKeyboardFocus();
}

void UItemInfoPopupWidget::ClosePopup()
{
	if (owningInventoryWidget)
		owningInventoryWidget->CloseInfoPopup();
	else
		RemoveFromParent();
}

void UItemInfoPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WindlessUI::StyleTree(this);
	SetIsFocusable(true);

	if (closeButton)
		closeButton->OnClicked.AddDynamic(this, &UItemInfoPopupWidget::HandleCloseClicked);
}

FReply UItemInfoPopupWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		ClosePopup();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UItemInfoPopupWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (popupBorder && popupBorder->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		return FReply::Handled();

	ClosePopup();
	return FReply::Handled();
}

void UItemInfoPopupWidget::ApplyItemInfo(const FItemInstance& inst, const FItemData& data)
{
	Super::ApplyItemInfo(inst, data); // 이름 / 분류 / 스테이터스

	/* 아이템 아이콘 */
	if (iconImage)
	{
		if (data.icon)
		{
			iconImage->SetBrushFromTexture(data.icon);
			iconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
			iconImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	/* 아이콘 등급 테두리 */
	if (iconFrame)
	{
		iconFrame->SetColorAndOpacity(FLinearColor(owningInventory->GetGradeColor(data.grade)));
		iconFrame->SetVisibility(data.icon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	/* 설명문 */
	if (descriptionText)
	{
		descriptionText->SetText(data.itemDescription);
		descriptionText->SetVisibility(data.itemDescription.IsEmpty()
			? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
}

void UItemInfoPopupWidget::HandleInvalidTarget()
{
	ClosePopup();
}

void UItemInfoPopupWidget::HandleCloseClicked()
{
	ClosePopup();
}
