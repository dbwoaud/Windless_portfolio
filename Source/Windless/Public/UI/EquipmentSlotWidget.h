#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemTypes.h"
#include "EquipmentSlotWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UInventoryComponent;
class UInventoryWidget;

UCLASS()
class WINDLESS_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/* 초기화 함수 */
	void InitializeSlot(UInventoryComponent* inventory, UInventoryWidget* ownerWidget); // 인벤토리와 슬롯을 초기화하는 함수 
	void RefreshSlot(); // 슬롯의 장착 상태를 업데이트하는 함수


	UFUNCTION(BlueprintPure, Category = "Equip")
	EEquipSlot GetSlotType() const { return slotType; } // 장비 슬롯 장착 부위를 반환하는 함수

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override; // 키 버튼 클릭 시 실행되는 함수
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 버튼 클릭 시 실행되는 함수
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 진입 시 실행되는 함수
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override; // 마우스 이탈 시 실행되는 함수
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override; // 드래그 감지 시 실행되는 함수
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 마우스 드래그동안 실행되는 함수
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 마우스 드래그 중인 아이템이 위젯 영역 밖으로 나갔을 때 실행되는 함수
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 마우스 드래그 완료 시 실행되는 함수

private:
	/* 슬롯 설정 */
	EEquipSlot GetSourceSlot() const; // 슬롯의 실제 원본 슬롯을 반환하는 함수


	/* 배경색 설정*/
	void SetSlotHighlight(bool bValid); // 드래그 중 장착 가능 여부를 배경색으로 표시하는 함수
	void ClearSlotHighlight(); // 배경색을 기본 상태로 되돌리는 함수


	/* 캐싱 변수 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> owningInventory = nullptr; // 소유 인벤토리

	UPROPERTY()
	TObjectPtr<UInventoryWidget> owningInventoryWidget = nullptr; // 소유 인벤토리 위젯


	/* 슬롯 */
	UPROPERTY(EditAnywhere, Category = "Equip")
	EEquipSlot slotType = EEquipSlot::EES_None; // 장비 슬롯 장착 부위


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> slotBackground; // 슬롯 배경

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> icon; // 장착 아이템 아이콘

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> emptyIcon; // 미장착 시 표시할 부위 실루엣

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> gradeBorder; // 등급 색 테두리

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> durabilityBar; // 내구도 바
};
