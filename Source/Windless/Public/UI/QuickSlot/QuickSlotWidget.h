#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/QuickSlotTypes.h"
#include "QuickSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;


UCLASS()
class WINDLESS_API UQuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 표시 설정 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetSlotDisplay(UTexture2D* icon, const FText& label, int32 count); // 슬롯 표시 내용을 설정하는 함수

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetSelected(bool bSelected); // 현재 선택 상태를 설정하는 함수

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void ClearSlot(); // 슬롯을 빈 상태로 되돌리는 함수

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetCooldownRatio(float ratio); // 재사용 대기 진행 비율을 표시하는 함수

protected:
	virtual void NativeConstruct() override;


	/* 슬롯 */
	UPROPERTY(EditAnywhere, Category = "QuickSlot")
	EQuickSlot slotType = EQuickSlot::EQS_None; // 이 위젯이 담당하는 퀵슬롯 종류


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> slotBorder; // 선택 표시용 테두리

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> slotIcon; // 등록 아이템 아이콘

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> emptyIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> cooldownOverlay; // 쿨다운 오버레이

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> slotLabel; // 슬롯 라벨

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> countText; // 수량 텍스트


	/* 설정값 */
	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor selectedColor = FLinearColor(0.838799f, 0.693872f, 0.381326f, 1.f); // 선택 테두리 색

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor normalColor = FLinearColor(0.838799f, 0.693872f, 0.381326f, 0.55f); // 기본 테두리 색

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor selectedTextColor = FLinearColor(0.838799f, 0.693872f, 0.381326f, 1.f); // 하이라이트 텍스트 색

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor normalTextColor = FLinearColor(0.838799f, 0.760525f, 0.590619f, 1.f); // 기본 텍스트 색

public:
	/* 변수 getter 함수 */
	FORCEINLINE EQuickSlot GetSlotType() const { return slotType; }
};