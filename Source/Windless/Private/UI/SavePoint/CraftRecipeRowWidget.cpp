#include "UI/SavePoint/CraftRecipeRowWidget.h"
#include "UI/SavePoint/CraftWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UCraftRecipeRowWidget::Setup(const FCraftRecipeEntry& entry, UCraftWidget* owner)
{
	ownerWidget = owner;
	recipeId = entry.recipeId;

	if (nameText)
	{
		nameText->SetText(entry.displayName);
		nameText->SetColorAndOpacity(entry.bCraftable ? craftableColor : blockedColor);
	}

	if (resultText)
		resultText->SetText(entry.resultSummary);

	if (ingredientText)
		ingredientText->SetText(entry.ingredientSummary);
}

void UCraftRecipeRowWidget::SetSelected(bool bInSelected)
{
	if (selectedImage)
		selectedImage->SetVisibility(bInSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UCraftRecipeRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (rowButton)
		rowButton->OnClicked.AddDynamic(this, &UCraftRecipeRowWidget::HandleClicked);
}

void UCraftRecipeRowWidget::HandleClicked()
{
	if (ownerWidget)
		ownerWidget->SelectRecipe(recipeId);
}
