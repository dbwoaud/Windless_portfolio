#include "UI/SavePoint/FastTravelWidget.h"
#include "UI/SavePoint/MapMarkerWidget.h"
#include "Components/FastTravelComponent.h"
#include "Components/MaintenanceComponent.h"
#include "Level/SavePointSubsystem.h"
#include "Level/Landmarks/LandmarkSubsystem.h"
#include "Level/SoulMarkerSubsystem.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameFramework/Pawn.h"
#include "Windless/DebugMacro.h"

void UFastTravelWidget::SelectDestination(FName savePointId)
{
	selectedDestinationId = savePointId;

	UWorld* world = GetWorld();
	const USavePointSubsystem* savePoints = world ? world->GetSubsystem<USavePointSubsystem>() : nullptr;

	FSavePointRecord record;
	selectedDestinationName = (savePoints && savePoints->FindActivated(savePointId, record)) ? record.displayName : FText::GetEmpty();

	RefreshSelection();
}

void UFastTravelWidget::RefreshDisplay()
{
	if (markerCanvas)
		markerCanvas->ClearChildren();

	markers.Reset();

	if (!markerClass)
		return;

	UWorld* world = GetWorld();
	if (!world)
		return;

	// 발견한 랜드마크에 마커 추가
	if (const ULandmarkSubsystem* landmarks = world->GetSubsystem<ULandmarkSubsystem>())
	{
		for (const FLandmarkRecord& record : landmarks->GetDiscoveredLandmarks())
			AddMarker(record.landmarkId, record.displayName, record.location, EMapMarkerType::EMMT_Landmark);
	}

	// 손실 회수 지점 설정
	if (const USoulMarkerSubsystem* soulMarkers = world->GetSubsystem<USoulMarkerSubsystem>())
	{
		if (soulMarkers->HasMarker())
		{
			const FSoulMarkerRecord record = soulMarkers->GetRecord();
			AddMarker(FName(TEXT("SoulLoss")),
				FText::FromString(FString::Printf(TEXT("탈피석 %d"), record.moltStones)),
				record.location, EMapMarkerType::EMMT_SoulLoss);
		}
	}

	// 활성 세이브포인트에 마커 추가
	if (const USavePointSubsystem* savePoints = world->GetSubsystem<USavePointSubsystem>())
	{
		for (const FSavePointRecord& record : savePoints->GetActivatedSavePoints())
			AddMarker(record.savePointId, record.displayName, record.respawnLocation, EMapMarkerType::EMMT_SavePoint);
	}

	const bool bSelectedAlive = markers.ContainsByPredicate(
		[this](const UMapMarkerWidget* m) { return m && m->GetMarkerId() == selectedDestinationId; });

	if (!bSelectedAlive)
	{
		selectedDestinationId = NAME_None;
		selectedDestinationName = FText::GetEmpty();
	}
}

void UFastTravelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (travelButton)
		travelButton->OnClicked.AddDynamic(this, &UFastTravelWidget::HandleTravelClicked);
}

void UFastTravelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDisplay();
}

UFastTravelComponent* UFastTravelWidget::GetFastTravel() const
{
	APawn* pawn = GetOwningPlayerPawn();
	return pawn ? pawn->FindComponentByClass<UFastTravelComponent>() : nullptr;
}

FVector2D UFastTravelWidget::WorldToMap(const FVector& worldLocation) const
{
	const float normalizedX = (worldSize.X != 0.f) ? (worldLocation.X - worldOrigin.X) / worldSize.X : 0.f;
	const float normalizedY = (worldSize.Y != 0.f) ? (worldLocation.Y - worldOrigin.Y) / worldSize.Y : 0.f;

	return FVector2D(
		FMath::Clamp(normalizedX, 0.f, 1.f) * mapSize.X,
		FMath::Clamp(normalizedY, 0.f, 1.f) * mapSize.Y);
}

void UFastTravelWidget::HandleTravelClicked()
{
	UFastTravelComponent* fastTravel = GetFastTravel();
	if (!fastTravel)
		return;

	if (fastTravel->TravelTo(selectedDestinationId))
		OnTravelStartedBP();

	RefreshSelection();
}

void UFastTravelWidget::AddMarker(FName id, const FText& label, const FVector& worldLocation, EMapMarkerType markerType)
{
	if (!markerCanvas || !markerClass)
		return;

	UMapMarkerWidget* marker = CreateWidget<UMapMarkerWidget>(GetOwningPlayer(), markerClass);
	if (!marker)
		return;

	marker->Setup(id, label, markerType, this);

	if (UCanvasPanelSlot* slot = markerCanvas->AddChildToCanvas(marker))
	{
		slot->SetAnchors(FAnchors(0.f, 0.f));
		slot->SetAlignment(FVector2D(0.5f, 0.5f));
		slot->SetAutoSize(true);
		slot->SetPosition(WorldToMap(worldLocation));
		slot->SetZOrder(markerType == EMapMarkerType::EMMT_SavePoint ? 2 : 1);
	}

	markers.Add(marker);
}

void UFastTravelWidget::RefreshSelection()
{
	for (UMapMarkerWidget* marker : markers)
	{
		if (marker)
			marker->SetSelected(marker->GetMarkerId() == selectedDestinationId);
	}

	if (destinationText)
		destinationText->SetText(selectedDestinationName);

	const UFastTravelComponent* fastTravel = GetFastTravel();
	const EFastTravelBlockReason reason = fastTravel
		? fastTravel->GetBlockReason(selectedDestinationId)
		: EFastTravelBlockReason::EFTB_NotAtPoint;

	if (travelButton)
		travelButton->SetIsEnabled(reason == EFastTravelBlockReason::EFTB_None);

	if (blockReasonText)
	{
		FText text = FText::GetEmpty();
		switch (reason)
		{
			case EFastTravelBlockReason::EFTB_InCombat:
				text = FText::FromString(TEXT("전투 중에는 이동할 수 없습니다"));
				break;
			case EFastTravelBlockReason::EFTB_SamePoint:
				text = FText::FromString(TEXT("이미 이 세이브포인트에 있습니다"));
				break;
			case EFastTravelBlockReason::EFTB_NotActive:
				text = FText::FromString(TEXT("목적지를 선택하세요"));
				break;
			case EFastTravelBlockReason::EFTB_NotAtPoint:
				text = FText::FromString(TEXT("세이브포인트에서만 이동할 수 있습니다"));
				break;
			default:
				break;
		}
		blockReasonText->SetText(text);
	}
}
