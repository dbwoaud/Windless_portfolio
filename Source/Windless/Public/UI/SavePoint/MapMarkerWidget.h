#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapMarkerWidget.generated.h"

UENUM(BlueprintType)
enum class EMapMarkerType : uint8
{
	EMMT_SavePoint UMETA(DisplayName = "SavePoint"),
	EMMT_Landmark  UMETA(DisplayName = "Landmark"),
	EMMT_SoulLoss  UMETA(DisplayName = "SoulLoss")
};

class UButton;
class UTextBlock;
class UImage;
class UFastTravelWidget;

UCLASS()
class WINDLESS_API UMapMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void Setup(FName inId, const FText& inLabel, EMapMarkerType inType, UFastTravelWidget* owner); // 맵 마커 위젯을 설정하는 함수
	
	
	/* UI 설정 */
	void SetSelected(bool bInSelected); // 선택 표시를 갱신하는 함수

protected:
	virtual void NativeOnInitialized() override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> markerButton; // 마커 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> labelText; // 이름 텍스트 

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> savePointIcon; // 세이브포인트 아이콘 이미지

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> landmarkIcon; // 랜드마크 아이콘 이미지

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> soulIcon; // 탈피석 손실 지점 아이콘 이미지

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> selectedRing; // 선택 표시 이미지

private:
	/* 캐시 변수 */
	UPROPERTY()
	TObjectPtr<UFastTravelWidget> ownerWidget = nullptr; // 소유 지도 위젯 


	/* 마커 */
	FName markerId = NAME_None; // 마커 ID
	EMapMarkerType markerType = EMapMarkerType::EMMT_SavePoint; // 마커 종류


	/* UI 설정 */
	UFUNCTION()
	void HandleClicked(); // 마커 버튼 클릭 시 실행되는 함수

public:
	/* 변수 Getter 함수 */
	FORCEINLINE FName GetMarkerId() const { return markerId; }
	FORCEINLINE bool IsSavePoint() const { return markerType == EMapMarkerType::EMMT_SavePoint; }
};