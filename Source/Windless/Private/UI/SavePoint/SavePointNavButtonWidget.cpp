#include "UI/SavePoint/SavePointNavButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void USavePointNavButtonWidget::SetSelected(bool bInSelected)
{
	bSelected = bInSelected;
	RefreshVisual();
}

void USavePointNavButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (labelText)
		labelText->SetText(label);

	if (iconImage)
	{
		iconImage->SetBrush(iconBrush);
		iconImage->SetRenderTransformAngle(iconAngle);
	}
	RefreshVisual();
}

void USavePointNavButtonWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (navButton)
	{
		navButton->OnClicked.AddDynamic(this, &USavePointNavButtonWidget::HandleClicked);
		navButton->OnHovered.AddDynamic(this, &USavePointNavButtonWidget::HandleHovered);
	}
}

void USavePointNavButtonWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	bFocused = true;
	RefreshVisual();
}

void USavePointNavButtonWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	bFocused = false;
	RefreshVisual();
}

void USavePointNavButtonWidget::HandleClicked()
{
	onNavClicked.Broadcast(page);
}

void USavePointNavButtonWidget::HandleHovered()
{
	if (navButton)
		navButton->SetKeyboardFocus();
}

void USavePointNavButtonWidget::RefreshVisual()
{
	const bool bHighlighted = bSelected || bFocused;

	if (highlightImage)
	{
		highlightImage->SetVisibility(bHighlighted ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		highlightImage->SetRenderOpacity(bSelected ? 1.f : focusOnlyOpacity);
	}

	if (labelText)
		labelText->SetColorAndOpacity(bHighlighted ? highlightTextColor : normalTextColor);
}
