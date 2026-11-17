#include "HUD/BossHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBossHealthBarWidget::ShowBar(const FText& bossName, float percent)
{
	if (bossNameText)
		bossNameText->SetText(bossName);

	SetHealthPercent(percent);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UBossHealthBarWidget::HideBar()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UBossHealthBarWidget::SetHealthPercent(float percent)
{
	if (bossHealthBar)
		bossHealthBar->SetPercent(FMath::Clamp(percent, 0.f, 1.f));
}

void UBossHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Collapsed);
}