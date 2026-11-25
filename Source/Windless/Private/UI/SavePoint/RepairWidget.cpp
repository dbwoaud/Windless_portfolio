#include "UI/SavePoint/RepairWidget.h"
#include "UI/SavePoint/RepairToolRowWidget.h"
#include "UI/SavePoint/RepairTargetRowWidget.h"
#include "Components/RepairComponent.h"
#include "Components/PanelWidget.h"
#include "Components/PanelSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "Windless/DebugMacro.h"

void URepairWidget::SelectTool(int64 toolUID)
{
	selectedToolUID = toolUID;
	RefreshToolSelection();
}

bool URepairWidget::RepairTarget(int64 targetUID)
{
	URepairComponent* repair = GetRepairComponent();
	if (!repair || selectedToolUID == 0)
		return false;

	const bool bSuccess = repair->RepairWithTool(selectedToolUID, targetUID);
	if (bSuccess)
		OnRepairSucceededBP();

	RefreshDisplay();
	return bSuccess;
}

void URepairWidget::RefreshDisplay()
{
	URepairComponent* repair = GetRepairComponent();
	if (!repair)
		return;

	tools = repair->GetRepairTools();

	const bool bSelectedAlive = tools.ContainsByPredicate(
		[this](const FRepairToolEntry& entry) { return entry.instanceUID == selectedToolUID; });

	if (!bSelectedAlive)
		selectedToolUID = tools.IsEmpty() ? 0 : tools[0].instanceUID;

	RebuildToolList();
	RebuildTargetList();
	RefreshToolSelection();
}

void URepairWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDisplay();
}

void URepairWidget::RebuildToolList()
{
	if (!toolList)
		return;

	toolList->ClearChildren();
	toolRows.Reset();

	if (!toolRowClass)
		return;

	for (const FRepairToolEntry& entry : tools)
	{
		URepairToolRowWidget* row = CreateWidget<URepairToolRowWidget>(GetOwningPlayer(), toolRowClass);
		if (!row)
			continue;

		row->Setup(entry, this);

		if (UPanelSlot* slot = toolList->AddChild(row))
		{
			if (UVerticalBoxSlot* boxSlot = Cast<UVerticalBoxSlot>(slot))
				boxSlot->SetPadding(toolRowPadding);
		}
		toolRows.Add(row);
	}

	if (emptyToolText)
		emptyToolText->SetVisibility(tools.IsEmpty() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void URepairWidget::RebuildTargetList()
{
	if (!targetList)
		return;

	targetList->ClearChildren();

	URepairComponent* repair = GetRepairComponent();
	if (!repair || !targetRowClass)
		return;

	const TArray<FRepairTargetEntry> targets = repair->GetRepairTargets();

	for (const FRepairTargetEntry& entry : targets)
	{
		URepairTargetRowWidget* row = CreateWidget<URepairTargetRowWidget>(GetOwningPlayer(), targetRowClass);
		if (!row)
			continue;

		row->Setup(entry, this);

		if (UPanelSlot* slot = targetList->AddChild(row))
		{
			if (UVerticalBoxSlot* boxSlot = Cast<UVerticalBoxSlot>(slot))
				boxSlot->SetPadding(targetRowPadding);
		}
	}

	if (emptyTargetText)
		emptyTargetText->SetVisibility(targets.IsEmpty() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void URepairWidget::RefreshToolSelection()
{
	for (URepairToolRowWidget* row : toolRows)
	{
		if (row)
			row->SetSelected(row->GetToolUID() == selectedToolUID);
	}

	if (selectedToolText)
	{
		const FRepairToolEntry* selected = tools.FindByPredicate(
			[this](const FRepairToolEntry& entry) { return entry.instanceUID == selectedToolUID; });

		selectedToolText->SetText(selected
			? FText::FromString(FString::Printf(TEXT("%s   %.0f / %.0f"),
				*selected->toolName.ToString(), selected->currentCharge, selected->maxCharge))
			: FText::GetEmpty());
	}
}

URepairComponent* URepairWidget::GetRepairComponent() const
{
	APawn* pawn = GetOwningPlayerPawn();
	return pawn ? pawn->FindComponentByClass<URepairComponent>() : nullptr;
}
