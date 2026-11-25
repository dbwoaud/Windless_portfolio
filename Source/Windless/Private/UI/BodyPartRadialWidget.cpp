#include "UI/BodyPartRadialWidget.h"
#include "UI/QuickSlot/BodyPartSectorWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UBodyPartRadialWidget::InitializeRadial(const TArray<EBodyPart>& usableParts)
{
	selectableParts.Reset();
	for (const EBodyPart part : usableParts)
		selectableParts.Add(part);

	static const EBodyPart allParts[] =
	{
		EBodyPart::EBP_Head,    EBodyPart::EBP_Chest,
		EBodyPart::EBP_LeftArm, EBodyPart::EBP_RightArm,
		EBodyPart::EBP_LeftLeg, EBodyPart::EBP_RightLeg
	};

	for (const EBodyPart part : allParts)
	{
		if (UBodyPartSectorWidget* sector = GetSectorWidget(part))
		{
			sector->SetSelectable(selectableParts.Contains(part));
			sector->SetHighlighted(false);
		}
	}
}

void UBodyPartRadialWidget::UpdateSelection(const FVector2D& selectionVector, EBodyPart hoveredPart)
{
	if (selectionIndicator)
	{
		if (UCanvasPanelSlot* canvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(selectionIndicator))
			canvasSlot->SetPosition(FVector2D(selectionVector.X, -selectionVector.Y));
	}

	if (hoveredPart == lastHoveredPart)
		return;

	if (UBodyPartSectorWidget* prev = GetSectorWidget(lastHoveredPart))
		prev->SetHighlighted(false);

	if (UBodyPartSectorWidget* next = GetSectorWidget(hoveredPart))
		next->SetHighlighted(true);

	lastHoveredPart = hoveredPart;
}

void UBodyPartRadialWidget::UpdatePartStatus(EBodyPart part, float health, float maxHealth, bool bInjured)
{
	if (UBodyPartSectorWidget* sector = GetSectorWidget(part))
		sector->SetPartStatus(health, maxHealth, bInjured);
}

bool UBodyPartRadialWidget::IsPartSelectable(EBodyPart part) const
{
	return selectableParts.Contains(part);
}

void UBodyPartRadialWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(false);
}

UBodyPartSectorWidget* UBodyPartRadialWidget::GetSectorWidget(EBodyPart part) const
{
	switch (part)
	{
		case EBodyPart::EBP_Head:     return headSector;
		case EBodyPart::EBP_Chest:    return chestSector;
		case EBodyPart::EBP_LeftArm:  return leftArmSector;
		case EBodyPart::EBP_RightArm: return rightArmSector;
		case EBodyPart::EBP_LeftLeg:  return leftLegSector;
		case EBodyPart::EBP_RightLeg: return rightLegSector;
		default:                      return nullptr;
	}
}
