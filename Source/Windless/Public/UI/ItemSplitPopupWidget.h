#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemSplitPopupWidget.generated.h"

class UButton;
class UTextBlock;
class USlider;
class UBorder;
class UInventoryComponent;
class UInventoryWidget;

UCLASS()
class WINDLESS_API UItemSplitPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void SetupPopup(UInventoryWidget* inOwnerWidget, UInventoryComponent* inInventory, int64 uid, 
		int32 inTargetX = -1, int32 inTargetY = -1, bool bInRotated = false); // 대상 아이템으로 나누기 팝업을 구성하는 함수


	/* 닫기 */
	void ClosePopup(); // 팝업을 닫는 함수

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override; // 키 입력 시 실행되는 함수
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 클릭 시 실행되는 함수


	/* UI 조작 */
	UFUNCTION()
	void HandleSliderChanged(float value); // 슬라이더 값 변경 시 실행되는 함수

	UFUNCTION()
	void HandleMinusClicked(); // 수량 감소 버튼 클릭 시 실행되는 함수

	UFUNCTION()
	void HandlePlusClicked(); // 수량 증가 버튼 클릭 시 실행되는 함수

	UFUNCTION()
	void HandleConfirmClicked(); // 확인 버튼 클릭 시 실행되는 함수

	UFUNCTION()
	void HandleCancelClicked(); // 취소 버튼 클릭 시 실행되는 함수

	void RefreshAmountText(); // 현재 수량 표시를 갱신하는 함수


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> popupBorder; // 팝업 상자

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> amountSlider; // 수량 선택 슬라이더

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> amountText; // 수량 표시 문구

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> confirmButton; // 확인 버튼

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> cancelButton; // 취소 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> minusButton; // 수량 감소 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> plusButton; // 수량 증가 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> headerText; // 아이템 이름 헤더

private:
	/* 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> owningInventory = nullptr; // 소유 인벤토리

	UPROPERTY()
	TObjectPtr<UInventoryWidget> owningInventoryWidget = nullptr; // 소유 인벤토리 위젯


	/* 나누기 배치 설정 */
	int64 targetUID = 0; // 대상 아이템 인스턴스 UID
	int32 targetX = -1; // 배치할 그리드 X
	int32 targetY = -1; // 배치할 그리드 Y
	int32 splitAmount = 1; // 나눌 수량
	int32 maxSplitAmount = 1; // 나눌 수 있는 최대 수량
	bool bTargetRotated = false; // 배치 시 회전 상태

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "Split")
	FORCEINLINE int64 GetTargetUID() const { return targetUID; }
};