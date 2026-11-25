#include "UI/QuickSlot/QuickSlotBarWidget.h"
#include "UI/QuickSlot/QuickSlotWidget.h"

void UQuickSlotBarWidget::SetSlotDisplay(EQuickSlot quickSlot, UTexture2D* icon, const FText& label, int32 count)
{
	if (UQuickSlotWidget* widget = GetSlotWidget(quickSlot))
		widget->SetSlotDisplay(icon, label, count);
}

void UQuickSlotBarWidget::SetSlotCooldown(EQuickSlot quickSlot, float ratio)
{
	if (UQuickSlotWidget* widget = GetSlotWidget(quickSlot))
		widget->SetCooldownRatio(ratio);
}

void UQuickSlotBarWidget::SetSelectedSlot(EQuickSlot quickSlot)
{
	selectedSlot = quickSlot;

	for (uint8 i = (uint8)EQuickSlot::EQS_Recovery; i < (uint8)EQuickSlot::EQS_MAX; ++i)
	{
		const EQuickSlot current = (EQuickSlot)i;
		if (UQuickSlotWidget* widget = GetSlotWidget(current))
			widget->SetSelected(current == quickSlot);
	}
}

void UQuickSlotBarWidget::ClearAllSlots()
{
	for (uint8 i = (uint8)EQuickSlot::EQS_Recovery; i < (uint8)EQuickSlot::EQS_MAX; ++i)
	{
		if (UQuickSlotWidget* widget = GetSlotWidget((EQuickSlot)i))
			widget->ClearSlot();
	}
}

void UQuickSlotBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetSelectedSlot(selectedSlot);
}

UQuickSlotWidget* UQuickSlotBarWidget::GetSlotWidget(EQuickSlot slot) const
{
	switch (slot)
	{
		case EQuickSlot::EQS_Recovery: return recoverySlot;
		case EQuickSlot::EQS_Consume1: return consumeSlot1;
		case EQuickSlot::EQS_Consume2: return consumeSlot2;
		case EQuickSlot::EQS_Consume3: return consumeSlot3;
		case EQuickSlot::EQS_Consume4: return consumeSlot4;
		default:                       return nullptr;
	}
}