#include "UI/InteractionWidget.h"
#include "Components/TextBlock.h"

void UInteractionWidget::SetActionText(const FText& inActionText)
{
	if (actionText)
		actionText->SetText(inActionText);
}