#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/QuickSlotTypes.h"
#include "QuickSlotBarWidget.generated.h"

class UQuickSlotWidget;

UCLASS()

class WINDLESS_API UQuickSlotBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 표시 설정 */
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetSlotDisplay(EQuickSlot quickSlot, UTexture2D* icon, const FText& label, int32 count); // 특정 슬롯의 표시 내용을 설정하는 함수

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetSlotCooldown(EQuickSlot quickSlot, float ratio); // 특정 슬롯의 재사용 대기 표시를 설정하는 함수

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetSelectedSlot(EQuickSlot quickSlot); // 현재 선택 슬롯을 설정하는 함수

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void ClearAllSlots(); // 모든 슬롯을 비우는 함수

protected:
	virtual void NativeConstruct() override;

	UQuickSlotWidget* GetSlotWidget(EQuickSlot slot) const; // 슬롯 종류에 대응하는 위젯을 반환하는 함수


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UQuickSlotWidget> recoverySlot; // 회복 슬롯

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UQuickSlotWidget> consumeSlot1; // 소모품 슬롯 1

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UQuickSlotWidget> consumeSlot2; // 소모품 슬롯 2

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UQuickSlotWidget> consumeSlot3; // 소모품 슬롯 3

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UQuickSlotWidget> consumeSlot4; // 소모품 슬롯 4


	/* 캐시 */
	EQuickSlot selectedSlot = EQuickSlot::EQS_None; // 현재 선택된 슬롯
};