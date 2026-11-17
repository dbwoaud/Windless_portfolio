#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/BodyPartTypes.h"
#include "BodyPartRadialWidget.generated.h"

class UCanvasPanel;
class UImage;
class UBodyPartSectorWidget;

UCLASS()
class WINDLESS_API UBodyPartRadialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 */
	void InitializeRadial(const TArray<EBodyPart>& usableParts); // 사용 가능 부위 목록으로 구역 활성 상태를 초기화하는 함수

	/* 실시간 갱신 */
	void UpdateSelection(const FVector2D& selectionVector, EBodyPart hoveredPart); // 인디케이터 위치와 구역 하이라이트를 갱신하는 함수
	void UpdatePartStatus(EBodyPart part, float health, float maxHealth, bool bInjured); // 특정 부위의 체력/부상 표시를 갱신하는 함수

	/* 조회 */
	bool IsPartSelectable(EBodyPart part) const; // 해당 부위가 선택 가능한지 확인하는 함수

protected:
	virtual void NativeConstruct() override;


	/* 위젯 */
	UBodyPartSectorWidget* GetSectorWidget(EBodyPart part) const; // 부위에 대응하는 구역 위젯을 반환하는 함수


	/* 캐시 */
	TSet<EBodyPart> selectableParts; // 현재 선택 가능한 부위 집합
	EBodyPart lastHoveredPart = EBodyPart::EBP_None; // 직전 프레임의 대상 부위


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> radialCanvas; // 구역/인디케이터를 담는 캔버스

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> selectionIndicator; // 마우스 대체 선택 인디케이터

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBodyPartSectorWidget> headSector; // 머리 구역

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBodyPartSectorWidget> chestSector; // 몸통 구역

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBodyPartSectorWidget> leftArmSector; // 왼팔 구역

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBodyPartSectorWidget> rightArmSector; // 오른팔 구역

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBodyPartSectorWidget> leftLegSector; // 왼다리 구역

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBodyPartSectorWidget> rightLegSector; // 오른다리 구역

public:

};