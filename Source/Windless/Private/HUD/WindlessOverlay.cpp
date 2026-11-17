#include "HUD/WindlessOverlay.h"
#include "HUD/BossHealthBarWidget.h"
#include "UI/QuickSlot/QuickSlotBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Components/AttributeComponent.h"

void UWindlessOverlay::SetHealthBarPercent(float percent)
{
	if (healthProgressBar)
		healthProgressBar->SetPercent(percent);
}

void UWindlessOverlay::SetStaminaBarPercent(float percent)
{
	if (staminaProgressBar)
		staminaProgressBar->SetPercent(percent);
}

void UWindlessOverlay::SetStaminaLockRatio(float ratio)
{
	if (staminaLockBar)
		staminaLockBar->SetPercent(ratio);
}

void UWindlessOverlay::SetMoltStones(int32 moltStone)
{
	if (moltStoneCountText)
	{
		const FString moltStoneString = FString::Printf(TEXT("%d"), moltStone);
		const FText moltStoneText = FText::FromString(moltStoneString);
		moltStoneCountText->SetText(moltStoneText);
	}
}

void UWindlessOverlay::SetChaosBarPercent(float percent)
{
	if (chaosProgressBar)
		chaosProgressBar->SetPercent(percent);
}

void UWindlessOverlay::SetChaosBarColor(EChaosState state)
{
	if (!chaosProgressBar)
		return;

	FLinearColor color;
	switch (state)
	{
		case EChaosState::ECS_Collapse1: 
			color = FLinearColor(0.9f, 0.5f, 0.1f, 1.f); // 주황
			break;
		case EChaosState::ECS_Collapse2: 
			color = FLinearColor(0.9f, 0.2f, 0.1f, 1.f); // 적주황
			break; 
		case EChaosState::ECS_Collapse3: 
			color = FLinearColor(0.7f, 0.0f, 0.0f, 1.f); // 진홍
			break;
		case EChaosState::ECS_Awakening1: 
			color = FLinearColor(0.2f, 0.6f, 0.9f, 1.f); // 하늘
			break;
		case EChaosState::ECS_Awakening2: 
			color = FLinearColor(0.1f, 0.4f, 0.9f, 1.f); // 청
			break;
		case EChaosState::ECS_Awakening3: 
			color = FLinearColor(0.2f, 0.1f, 0.8f, 1.f); // 남보라
			break;
		default:
			color = FLinearColor(0.6f, 0.6f, 0.6f, 1.f); // 회색
			break;
	}
	chaosProgressBar->SetFillColorAndOpacity(color);
}

void UWindlessOverlay::UpdateBodyPart(EBodyPart part, float percent, bool bInjured, bool bDisabled)
{
	UProgressBar* bar = nullptr;
	switch (part)
	{
		case EBodyPart::EBP_Head:
			bar = headBar;
			break;
		case EBodyPart::EBP_Chest:
			bar = chestBar;
			break;
		case EBodyPart::EBP_LeftArm:
			bar = leftArmBar;
			break;
		case EBodyPart::EBP_RightArm:
			bar = rightArmBar;
			break;
		case EBodyPart::EBP_LeftLeg:
			bar = leftLegBar;
			break;
		case EBodyPart::EBP_RightLeg:
			bar = rightLegBar;
			break;
		default: 
			return;
	}

	if (!bar) // 해당되는 부위 체력 바가 없으면 무시
		return;

	bar->SetPercent(percent);
	SetHealthBarColor(bDisabled, bInjured, bar);
}

void UWindlessOverlay::ToggleBodyStatus()
{
	SetBodyStatusVisible(!IsBodyStatusVisible());
}

void UWindlessOverlay::SetBodyStatusVisible(bool bVisible)
{
	if (bodyStatusPanel)
		bodyStatusPanel->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

bool UWindlessOverlay::IsBodyStatusVisible() const
{
	return bodyStatusPanel && bodyStatusPanel->GetVisibility() != ESlateVisibility::Collapsed;
}

void UWindlessOverlay::ShowBossHealthBar(const FText& bossName, float percent)
{
	if (bossHealthBarWidget)
		bossHealthBarWidget->ShowBar(bossName, percent);
}

void UWindlessOverlay::HideBossHealthBar()
{
	if (bossHealthBarWidget)
		bossHealthBarWidget->HideBar();
}

void UWindlessOverlay::SetBossHealthPercent(float percent)
{
	if (bossHealthBarWidget)
		bossHealthBarWidget->SetHealthPercent(percent);
}

void UWindlessOverlay::UpdateQuickSlot(EQuickSlot quickSlot, UTexture2D* icon, int32 count)
{
	if (quickSlotBar)
		quickSlotBar->SetSlotDisplay(quickSlot, icon, FText::GetEmpty(), count);
}

void UWindlessOverlay::SetQuickSlotCooldown(EQuickSlot quickSlot, float ratio)
{
	if (quickSlotBar)
		quickSlotBar->SetSlotCooldown(quickSlot, ratio);
}

void UWindlessOverlay::SetSelectedQuickSlot(EQuickSlot quickSlot)
{
	if (quickSlotBar)
		quickSlotBar->SetSelectedSlot(quickSlot);
}

void UWindlessOverlay::ClearQuickSlots()
{
	if (quickSlotBar)
		quickSlotBar->ClearAllSlots();
}

void UWindlessOverlay::SetHealthBarColor(bool bDisabled, bool bInjured, UProgressBar* bar)
{
	FLinearColor color;
	if (bDisabled)
		color = FLinearColor(0.25f, 0.f, 0.f, 1.f); // 파괴: 빨강
	else if (bInjured)
		color = FLinearColor(1.f, 0.85f, 0.1f, 1.f); // 부상: 노랑
	else
		color = FLinearColor(0.f, 1.f, 0.f, 1.f); // 정상 :초록

	bar->SetFillColorAndOpacity(color);
}