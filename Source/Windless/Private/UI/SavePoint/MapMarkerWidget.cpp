#include "UI/SavePoint/MapMarkerWidget.h"
#include "UI/SavePoint/FastTravelWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UMapMarkerWidget::Setup(FName inId, const FText& inLabel, EMapMarkerType inType, UFastTravelWidget* owner)
{
	markerId = inId;
	markerType = inType;
	ownerWidget = owner;

	if (labelText)
		labelText->SetText(inLabel);

	const bool bSave = (markerType == EMapMarkerType::EMMT_SavePoint);
	const bool bLand = (markerType == EMapMarkerType::EMMT_Landmark);
	const bool bSoul = (markerType == EMapMarkerType::EMMT_SoulLoss);

	if (savePointIcon)
		savePointIcon->SetVisibility(bSave ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if (landmarkIcon)
		landmarkIcon->SetVisibility(bLand ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if (soulIcon)
		soulIcon->SetVisibility(bSoul ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	if (markerButton)
		markerButton->SetIsEnabled(bSave);

	SetSelected(false);
}

void UMapMarkerWidget::SetSelected(bool bInSelected)
{
	if (selectedRing)
		selectedRing->SetVisibility(bInSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UMapMarkerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (markerButton)
		markerButton->OnClicked.AddDynamic(this, &UMapMarkerWidget::HandleClicked);
}

void UMapMarkerWidget::HandleClicked()
{
	if (ownerWidget && IsSavePoint())
		ownerWidget->SelectDestination(markerId);
}
