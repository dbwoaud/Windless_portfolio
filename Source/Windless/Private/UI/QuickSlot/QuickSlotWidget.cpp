#include "UI/QuickSlot/QuickSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "UI/WindlessUIStyle.h"

void UQuickSlotWidget::SetSlotDisplay(UTexture2D* icon, const FText& label, int32 count)
{
	const bool bHasItem = (count > 0);

	if (bHasItem)
	{
		if (slotIcon)
		{
			if (icon)
			{
				slotIcon->SetBrushFromTexture(icon);
				slotIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
			else
				slotIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (countText)
		{
			countText->SetText(FText::AsNumber(count));
			countText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		if (emptyIcon)
			emptyIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		if (slotIcon)
			slotIcon->SetVisibility(ESlateVisibility::Collapsed);
		if (countText)
			countText->SetVisibility(ESlateVisibility::Collapsed);
		if (emptyIcon)
			emptyIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (slotLabel && !label.IsEmpty())
		slotLabel->SetText(label);
}

void UQuickSlotWidget::SetSelected(bool bSelected)
{
	if (slotBorder) // 세이브포인트 UI와 같은 칸: 어두운 바탕에 금색 얇은 테두리, 고른 칸은 밝은 금색 두 줄
	{
		slotBorder->SetBrush(WindlessUI::Box(FLinearColor(0.f, 0.f, 0.f, 0.6f),
			bSelected ? selectedColor : normalColor, bSelected ? 2.f : 1.f));
		slotBorder->SetBrushColor(FLinearColor::White);
	}

	if (slotLabel)
		slotLabel->SetColorAndOpacity(FSlateColor(bSelected ? selectedTextColor : normalTextColor));

	if (countText)
		countText->SetColorAndOpacity(FSlateColor(bSelected ? selectedTextColor : normalTextColor));
}

void UQuickSlotWidget::ClearSlot()
{
	SetSlotDisplay(nullptr, FText::GetEmpty(), 0);
	SetSelected(false);
}

void UQuickSlotWidget::SetCooldownRatio(float ratio)
{
	if (!cooldownOverlay)
		return;

	const float clamped = FMath::Clamp(ratio, 0.f, 1.f);
	cooldownOverlay->SetVisibility(clamped > 0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	cooldownOverlay->SetRenderOpacity(clamped * 0.7f);
}

void UQuickSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WindlessUI::EmptyIcon(emptyIcon);
	WindlessUI::Text(slotLabel, 10.f, 0, normalTextColor);
	WindlessUI::Text(countText, 11.f, 0, normalTextColor);
	SetSelected(false);
}
