#include "UI/QuickSlot/BodyPartSectorWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UBodyPartSectorWidget::SetSelectable(bool bInSelectable)
{
	bSelectable = bInSelectable;
	RefreshColor();
}

void UBodyPartSectorWidget::SetHighlighted(bool bInHighlighted)
{
	bHighlighted = bInHighlighted;
	RefreshColor();
}

void UBodyPartSectorWidget::SetPartStatus(float health, float maxHealth, bool bInjured)
{
	const float percent = (maxHealth > 0.f) ? FMath::Clamp(health / maxHealth, 0.f, 1.f) : 0.f;

	if (partHealthBar)
		partHealthBar->SetPercent(percent);

	if (partHealthText)
		partHealthText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), FMath::RoundToInt(health), FMath::RoundToInt(maxHealth))));

	if (injuryIcon)
		injuryIcon->SetVisibility(bInjured ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UBodyPartSectorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshColor();
}

void UBodyPartSectorWidget::RefreshColor()
{
	if (!sectorImage)
		return;

	if (bHighlighted)
		sectorImage->SetColorAndOpacity(bSelectable ? highlightColor : invalidColor);
	else
		sectorImage->SetColorAndOpacity(bSelectable ? normalColor : disabledColor);
}