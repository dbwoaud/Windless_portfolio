#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/QuickSlotTypes.h"
#include "ConsumableSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UInventoryComponent;
class UInventoryWidget;

UCLASS()
class WINDLESS_API UConsumableSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void InitializeSlot(UInventoryComponent* inventory, UInventoryWidget* ownerWidget); // 인벤토리와 슬롯을 초기화하는 함수
	void RefreshSlot(); // 등록 상태를 위젯에 반영하는 함수

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override; // 키 입력 시 실행되는 함수
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 클릭 시 실행되는 함수
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 진입 시 실행되는 함수
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override; // 마우스 이탈 시 실행되는 함수
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override; // 드래그 감지 시 실행되는 함수
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 드래그 중 실행되는 함수
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 드래그가 영역 밖으로 나갔을 때 실행되는 함수
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 드롭 시 실행되는 함수

private:
	/* 배경 색 설정 */
	void SetSlotHighlight(bool bValid); // 드래그 중 장착 가능 여부를 배경색으로 표시하는 함수
	void ClearSlotHighlight(); // 배경색을 기본 상태로 되돌리는 함수


	/* 캐싱 변수 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> owningInventory = nullptr; // 소유 인벤토리

	UPROPERTY()
	TObjectPtr<UInventoryWidget> owningInventoryWidget = nullptr; // 소유 인벤토리 위젯


	/* 슬롯 */
	UPROPERTY(EditAnywhere, Category = "QuickSlot")
	EQuickSlot slotType = EQuickSlot::EQS_None; // 퀵슬롯 종류


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> slotBackground; // 슬롯 배경

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> icon; // 등록 아이템 아이콘

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> emptyIcon; // 빈 슬롯 표시 아이콘

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> countText; // 수량 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> keyText; // 단축키 라벨


	/* 설정값 */
	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor normalColor = FLinearColor(0.126f, 0.121f, 0.094f, 1.f); // 기본 배경색

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor validColor = FLinearColor(0.10f, 0.45f, 0.12f, 1.f); // 장착 가능 배경색

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Color")
	FLinearColor invalidColor = FLinearColor(0.50f, 0.08f, 0.08f, 1.f); // 장착 불가 배경색

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "QuickSlot")
	FORCEINLINE EQuickSlot GetSlotType() const { return slotType; }
};