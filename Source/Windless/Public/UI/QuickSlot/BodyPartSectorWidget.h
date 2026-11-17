#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/BodyPartTypes.h"
#include "BodyPartSectorWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;

UCLASS()
class WINDLESS_API UBodyPartSectorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 표시 제어 */
	void SetSelectable(bool bInSelectable); // 이 구역의 선택 가능 여부를 설정하는 함수
	void SetHighlighted(bool bInHighlighted); // 이 구역의 하이라이트 여부를 설정하는 함수
	void SetPartStatus(float health, float maxHealth, bool bInjured); // 부위 체력과 부상 표시를 갱신하는 함수

protected:
	virtual void NativeConstruct() override;

	void RefreshColor(); // 선택 가능/하이라이트 상태에 따라 색을 갱신하는 함수


	/* 부위 */
	UPROPERTY(EditAnywhere, Category = "QuickSlot|Sector")
	EBodyPart partType = EBodyPart::EBP_None; // 이 위젯이 담당하는 신체 부위


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> sectorImage; // 섹터 배경 이미지

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> partHealthBar; // 부위 체력 바

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> partHealthText; // 부위 체력 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> injuryIcon; // 부상 아이콘


	/* 설정값 */
	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor normalColor = FLinearColor(0.564712f, 0.391573f, 0.147027f, 0.6f); // 평시 색

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor highlightColor = FLinearColor(0.838799f, 0.693872f, 0.381326f, 1.f); // 하이라이트 색

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor disabledColor = FLinearColor(0.15f, 0.15f, 0.15f, 0.85f); // 사용 불가 색

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor invalidColor = FLinearColor(0.86f, 0.24f, 0.24f, 1.f); // 사용 불가 부위를 지목했을 때의 색


	/* 상태 */
	bool bSelectable = true; // 현재 선택 가능한지
	bool bHighlighted = false; // 현재 하이라이트 중인지

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "QuickSlot")
	FORCEINLINE EBodyPart GetPartType() const { return partType; }
};