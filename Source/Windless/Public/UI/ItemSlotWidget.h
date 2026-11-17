#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemTypes.h"
#include "ItemSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UInventoryComponent;
class USizeBox;

UCLASS()
class WINDLESS_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void InitializeItem(UInventoryComponent* inventory, int64 uid, float inCellSize); // 인벤토리와 인스턴스 UID를 설정하는 함수
	void RefreshSlot(); // 현재 인스턴스 상태를 위젯에 반영하는 함수
	void SetOwningInventoryWidget(class UInventoryWidget* widget) { owningInventoryWidget = widget; } // 인벤토리 위젯을 설정하는 함수


	/* 그리드 배치용 */
	void GetFootprint(int32& outW, int32& outH) const; // 회전 반영 점유 크기를 반환하는 함수

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override; // 키 클릭 시 실행되는 함수
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 클릭 시 실행되는 함수
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 포인터가 들어왔을 때 실행되는 함수
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override; // 마우스 포인터가 나갔을 때 실행되는 함수
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override; // 드래그 감지 시 실행되는 함수


private:
	/* 캐싱 변수 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> owningInventory = nullptr; // 소유한 인벤토리

	UPROPERTY()
	TObjectPtr<class UInventoryWidget> owningInventoryWidget = nullptr; // 소유한 인벤토리 위젯

	UPROPERTY()
	int64 instanceUID = 0; // 표현할 아이템 인스턴스 UID

	float cellSize = 64.f; // 셀 픽셀 크기


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> icon; // 아이템 아이콘

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> quantityText; // 수량 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> gradeBorder; // 등급 색 테두리

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> durabilityBar; // 내구도 바


	/* 마우스 드래그 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> rootSizeBox; // 고스트 비주얼 크기 박스

	int32 grabOffsetX = 0; // 드래그 시작 X셀 위치
	int32 grabOffsetY = 0; // 드래그 시작 Y셀 위치

	bool bPendingSplitDrag = false; // 시프트 키를 누른 채 드래그를 시작했는지 여부

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int64 GetInstanceUID() const { return instanceUID; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE UInventoryComponent* GetOwningInventory() const { return owningInventory; }
};
