#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/FastTravelComponent.h"
#include "UI/SavePoint/MapMarkerWidget.h"
#include "FastTravelWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UButton;
class UMapMarkerWidget;

UCLASS()
class WINDLESS_API UFastTravelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 빠른 이동 */
	void SelectDestination(FName savePointId); // 빠른 이동 목적지를 선택하는 함수


	/* UI 설정 */
	UFUNCTION(BlueprintCallable, Category = "FastTravel")
	void RefreshDisplay(); // 지도 핀을 다시 배치하는 함수

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> markerCanvas; // 마커 캔버스

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> travelButton; // 빠른 이동 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> destinationText; // 목적지 이름 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> blockReasonText; // 차단 사유 텍스트


	/* 지도 좌표 변환 설정 */
	UPROPERTY(EditAnywhere, Category = "FastTravel|Map")
	FVector2D worldOrigin = FVector2D(-100000.f, -100000.f); // 지도 좌상단에 해당하는 월드 좌표 (X, Y)

	UPROPERTY(EditAnywhere, Category = "FastTravel|Map")
	FVector2D worldSize = FVector2D(200000.f, 200000.f); // 지도가 덮는 월드 범위 (X, Y)

	UPROPERTY(EditAnywhere, Category = "FastTravel|Map")
	FVector2D mapSize = FVector2D(900.f, 620.f); // 지도 이미지 픽셀 크기

	UPROPERTY(EditAnywhere, Category = "FastTravel|Map")
	TSubclassOf<UMapMarkerWidget> markerClass; // 마커 위젯 클래스


	/* 블루프린트용 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category = "FastTravel")
	void OnTravelStartedBP(); // 빠른 이동 시작 시 실행되는 블루프린트용 함수

private:
	/* 캐시 변수 */
	UFastTravelComponent* GetFastTravel() const; // 빠른 이동 컴포넌트를 반환하는 함수 


	/* 지도 좌표 변환 설정 */
	FVector2D WorldToMap(const FVector& worldLocation) const; // 월드 좌표를 지도 픽셀 좌표로 변환하는 함수


	/* UI 설정 */
	UFUNCTION()
	void HandleTravelClicked(); // 빠른 이동 버튼 클릭 시 실행되는 함수
	void AddMarker(FName id, const FText& label, const FVector& worldLocation, EMapMarkerType markerType);  // 마커를 배치하는 함수
	void RefreshSelection(); // 선택 표시와 버튼 상태를 갱신하는 함수

	UPROPERTY()
	TArray<TObjectPtr<UMapMarkerWidget>> markers; // 생성된 마커

	FName selectedDestinationId = NAME_None; // 선택한 목적지 ID
	FText selectedDestinationName; // 선택한 목적지 이름
};