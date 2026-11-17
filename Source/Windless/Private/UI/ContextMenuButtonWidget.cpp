#include "UI/ContextMenuButtonWidget.h"
#include "UI/WindlessUIStyle.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UContextMenuButtonWidget::SetupButton(EItemContextAction inAction, const FText& label, bool bEnabled, EBodyPart inTargetPart)
{
	action = inAction;
	targetPart = inTargetPart;
	bActionEnabled = bEnabled;

	if (actionText)
	{
		actionText->SetText(label);
		actionText->SetColorAndOpacity(FSlateColor(bEnabled ? normalTextColor : disabledTextColor));
	}

	if (actionButton)
		actionButton->SetIsEnabled(bEnabled);
}

void UContextMenuButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WindlessUI::StyleTree(this);

	if (!actionButton)
		return;

	actionButton->OnClicked.AddDynamic(this, &UContextMenuButtonWidget::HandleClicked);
	actionButton->OnHovered.AddDynamic(this, &UContextMenuButtonWidget::HandleHovered);
	actionButton->OnUnhovered.AddDynamic(this, &UContextMenuButtonWidget::HandleUnhovered);
}

void UContextMenuButtonWidget::HandleClicked()
{
	if (!bActionEnabled)
		return;

	OnContextActionClicked.ExecuteIfBound(action, targetPart);
}

void UContextMenuButtonWidget::HandleHovered()
{
	if (actionText && bActionEnabled)
		actionText->SetColorAndOpacity(FSlateColor(hoveredTextColor));
}

void UContextMenuButtonWidget::HandleUnhovered()
{
	if (actionText && bActionEnabled)
		actionText->SetColorAndOpacity(FSlateColor(normalTextColor));
}