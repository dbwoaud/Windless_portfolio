#include "UI/SavePoint/CraftWidget.h"
#include "UI/SavePoint/CraftRecipeRowWidget.h"
#include "Components/CraftingComponent.h"
#include "Components/PanelWidget.h"
#include "Components/PanelSlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "GameFramework/Pawn.h"
#include "Windless/DebugMacro.h"
#include "UI/SavePoint/CraftWidget.h"

void UCraftWidget::RefreshDisplay()
{
	UCraftingComponent* crafting = GetCrafting();
	if (!crafting)
		return;

	entries = crafting->GetRecipeEntries(currentCategory);

	const bool bSelectedAlive = entries.ContainsByPredicate(
		[this](const FCraftRecipeEntry& e) { return e.recipeId == selectedRecipeId; });

	if (!bSelectedAlive)
	{
		selectedRecipeId = entries.IsEmpty() ? NAME_None : entries[0].recipeId;
		craftCount = 1;
	}

	RebuildList();
	RefreshDetail();
}

void UCraftWidget::SetCategory(ECraftCategory newCategory)
{
	currentCategory = newCategory;
	selectedRecipeId = NAME_None;
	craftCount = 1;
	RefreshDisplay();
}

void UCraftWidget::SelectRecipe(FName recipeId)
{
	selectedRecipeId = recipeId;
	craftCount = 1;
	RefreshDetail();
}

void UCraftWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (craftButton)
		craftButton->OnClicked.AddDynamic(this, &UCraftWidget::HandleCraftClicked);

	if (countSlider)
		countSlider->OnValueChanged.AddDynamic(this, &UCraftWidget::HandleSliderChanged);

	if (tabConsumableButton)
		tabConsumableButton->OnClicked.AddDynamic(this, &UCraftWidget::HandleTabConsumable);

	if (tabEquipmentButton)
		tabEquipmentButton->OnClicked.AddDynamic(this, &UCraftWidget::HandleTabEquipment);

	if (tabToolButton) 
		tabToolButton->OnClicked.AddDynamic(this, &UCraftWidget::HandleTabTool);

	if (tabFacilityButton)
		tabFacilityButton->OnClicked.AddDynamic(this, &UCraftWidget::HandleTabFacility);
}

void UCraftWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDisplay();
}

void UCraftWidget::HandleCraftClicked()
{
	UCraftingComponent* crafting = GetCrafting();
	if (!crafting || selectedRecipeId.IsNone())
		return;

	if (crafting->Craft(selectedRecipeId, craftCount))
		OnCraftSucceededBP();

	craftCount = 1;
	RefreshDisplay();
}

void UCraftWidget::HandleSliderChanged(float value)
{
	craftCount = FMath::RoundToInt(value);
	RefreshDetail();
}

void UCraftWidget::HandleTabConsumable()
{
	SetCategory(ECraftCategory::ECC_Consumable);
}

void UCraftWidget::HandleTabEquipment()
{
	SetCategory(ECraftCategory::ECC_Equipment);
}

void UCraftWidget::HandleTabTool()
{
	SetCategory(ECraftCategory::ECC_Tool);
}

void UCraftWidget::HandleTabFacility()
{
	SetCategory(ECraftCategory::ECC_Facility);
}

void UCraftWidget::RebuildList()
{
	if (!recipeList)
		return;

	recipeList->ClearChildren();
	rows.Reset();

	if (!recipeRowClass)
		return;

	for (const FCraftRecipeEntry& entry : entries)
	{
		UCraftRecipeRowWidget* row = CreateWidget<UCraftRecipeRowWidget>(GetOwningPlayer(), recipeRowClass);
		if (!row)
			continue;

		row->Setup(entry, this);
		row->SetSelected(entry.recipeId == selectedRecipeId);

		if (UPanelSlot* slot = recipeList->AddChild(row))
		{
			if (UVerticalBoxSlot* boxSlot = Cast<UVerticalBoxSlot>(slot))
				boxSlot->SetPadding(recipeRowPadding);
		}
		rows.Add(row);
	}

	if (emptyRecipeText)
		emptyRecipeText->SetVisibility(entries.IsEmpty() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UCraftWidget::RefreshDetail()
{
	const FCraftRecipeEntry* selected = entries.FindByPredicate(
		[this](const FCraftRecipeEntry& e) { return e.recipeId == selectedRecipeId; });

	const int32 maxCount = selected ? FMath::Max(1, selected->maxCraftableCount) : 1;
	craftCount = FMath::Clamp(craftCount, 1, maxCount);

	if (detailNameText)
		detailNameText->SetText(selected ? selected->displayName : FText::GetEmpty());

	if (detailResultText)
		detailResultText->SetText(selected ? selected->resultSummary : FText::GetEmpty());

	if (detailIngredientText)
		detailIngredientText->SetText(selected ? selected->ingredientSummary : FText::GetEmpty());

	if (countText)
		countText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), craftCount, maxCount)));

	if (countSlider)
	{
		countSlider->SetMinValue(1.f);
		countSlider->SetMaxValue(static_cast<float>(maxCount));
		countSlider->SetValue(static_cast<float>(craftCount));
		countSlider->SetIsEnabled(maxCount > 1);
	}

	const bool bCanCraft = selected && selected->maxCraftableCount >= craftCount;

	if (craftButton)
		craftButton->SetIsEnabled(bCanCraft);

	if (blockReasonText)
	{
		FText reason = FText::GetEmpty();
		if (selected && !bCanCraft)
		{
			switch (selected->blockReason)
			{
				case ECraftBlockReason::ECBR_NoMaterial:    reason = FText::FromString(TEXT("재료가 부족합니다")); break;
				case ECraftBlockReason::ECBR_NoSpace:       reason = FText::FromString(TEXT("소지품 공간이 부족합니다")); break;
				case ECraftBlockReason::ECBR_GradeMismatch: reason = FText::FromString(TEXT("이전 등급 창고가 필요합니다")); break;
				default:                                    reason = FText::FromString(TEXT("제작할 수 없습니다")); break;
			}
		}
		blockReasonText->SetText(reason);
	}

	for (UCraftRecipeRowWidget* row : rows)
	{
		if (row)
			row->SetSelected(row->GetRecipeId() == selectedRecipeId);
	}
}

UCraftingComponent* UCraftWidget::GetCrafting() const
{
	APawn* pawn = GetOwningPlayerPawn();
	return pawn ? pawn->FindComponentByClass<UCraftingComponent>() : nullptr;
}
