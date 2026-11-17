#include "UI/SavePoint/RepairToolRowWidget.h"
#include "UI/SavePoint/RepairWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void URepairToolRowWidget::Setup(const FRepairToolEntry& entry, URepairWidget* owner)
{
	ownerWidget = owner;
	toolUID = entry.instanceUID;

	if (nameText)
		nameText->SetText(entry.toolName);

	if (chargeText)
		chargeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), entry.currentCharge, entry.maxCharge)));
}

void URepairToolRowWidget::SetSelected(bool bInSelected)
{
	if (selectedImage)
		selectedImage->SetVisibility(bInSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);

	if (nameText)
		nameText->SetColorAndOpacity(bInSelected ? selectedTextColor : normalTextColor);
}

void URepairToolRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (rowButton)
		rowButton->OnClicked.AddDynamic(this, &URepairToolRowWidget::HandleClicked);
}

void URepairToolRowWidget::HandleClicked()
{
	if (ownerWidget)
		ownerWidget->SelectTool(toolUID);
}
