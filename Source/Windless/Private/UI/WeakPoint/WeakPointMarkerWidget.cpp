#include "UI/WeakPoint/WeakPointMarkerWidget.h"
#include "Components/TextBlock.h"

void UWeakPointMarkerWidget::SetupWarning(int32 weakPointCount)
{
	if (labelText)
		labelText->SetText(FText::FromString(FString::ChrN(FMath::Max(0, weakPointCount), TEXT('|'))));

	OnMarkerSetup(false);
}

void UWeakPointMarkerWidget::SetupDetail(const FText& regionLabel)
{
	if (labelText)
		labelText->SetText(regionLabel);

	OnMarkerSetup(true);
}
