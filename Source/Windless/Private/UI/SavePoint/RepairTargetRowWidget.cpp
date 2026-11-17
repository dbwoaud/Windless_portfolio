#include "UI/SavePoint/RepairTargetRowWidget.h"
#include "UI/SavePoint/RepairWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void URepairTargetRowWidget::Setup(const FRepairTargetEntry& entry, URepairWidget* owner)
{
	ownerWidget = owner;
	targetUID = entry.instanceUID;

	if (nameText)
		nameText->SetText(entry.equipmentName);

	if (durabilityText)
	{
		FString text = FString::Printf(TEXT("%.0f / %.0f"), entry.currentDurability, entry.maxDurability);
		if (entry.bBroken)
			text += brokenSuffix.ToString();
		durabilityText->SetText(FText::FromString(text));
	}

	if (durabilityBar)
	{
		const float percent = (entry.maxDurability > 0.f) ? (entry.currentDurability / entry.maxDurability) : 0.f;
		durabilityBar->SetPercent(percent);
		durabilityBar->SetFillColorAndOpacity(entry.bBroken ? brokenBarColor : normalBarColor);
	}

	if (repairButton)
		repairButton->SetIsEnabled(entry.requiredCharge > 0.f);
}

void URepairTargetRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (repairButton)
		repairButton->OnClicked.AddDynamic(this, &URepairTargetRowWidget::HandleRepairClicked);
}

void URepairTargetRowWidget::HandleRepairClicked()
{
	if (ownerWidget)
		ownerWidget->RepairTarget(targetUID);
}
