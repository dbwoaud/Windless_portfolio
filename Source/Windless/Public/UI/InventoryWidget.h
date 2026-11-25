#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemTypes.h"
#include "InventoryWidget.generated.h"

class UImage;
class UCanvasPanel;
class UUniformGridPanel;
class UItemSlotWidget;
class UEquipmentSlotWidget;
class UInventoryComponent;
class UMyItemContextMenuWidget;
class UItemSplitPopupWidget;
class UItemInfoPopupWidget;
class UItemHoverInfoWidget;
class UItemGridWidget;
class UConsumableSlotWidget;
class UBodyPartSlotWidget;
class UDragDropOperation;
class UItemDragDrop;

UCLASS()
class WINDLESS_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 우클릭 팝업 시스템 */
	void OpenContextMenu(UInventoryComponent* inventory, int64 uid, EEquipSlot slot, const FVector2D& screenPos); // 우클릭 팝업을 여는 함수
	void CloseContextMenu(); // 우클릭 팝업을 닫는 함수
	void OpenSplitPopup(UInventoryComponent* inventory, int64 uid, int32 targetX = -1, int32 targetY = -1, bool bRotated = false);  // 나누기 팝업을 여는 함수
	void CloseSplitPopup(); // 나누기 팝업을 닫는 함수
	void OpenInfoPopup(UInventoryComponent* inventory, int64 uid); // 정보 팝업을 여는 함수
	void CloseInfoPopup(); // 정보 팝업을 닫는 함수
	void CloseAllPopups(); // 모든 팝업을 닫는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Popup")
	bool IsAnyPopupOpen() const; // 팝업이 하나라도 열려 있는지 확인하는 함수


	/* 호버 정보 팝업 시스템 */
	void SetHoveredItem(UInventoryComponent* inventory, int64 uid); // 마우스가 올라간 아이템을 설정하는 함수
	void BeginHoverInfo(UInventoryComponent* inventory, int64 uid); // 호버 정보 팝업 표시 대기를 시작하는 함수
	void CancelHoverInfo(); // 호버 대기를 취소하고 호버 정보 팝업을 닫는 함수

	UPROPERTY()
	TObjectPtr<UInventoryComponent> hoveredInventory = nullptr; // 호버된 인벤토리 컴포넌트

	UPROPERTY()
	TObjectPtr<UInventoryComponent> pendingHoverInventory = nullptr;   // 호버 보류 중인 인벤토리 컴포넌트


	/* 그리드 */
	void SetStorageInventory(UInventoryComponent* storage); // 창고 인벤토리를 연결하는 함수
	void RefreshAllGrids(); // 모든 그리드를 갱신하는 함수
	float GetCellSize() const; // 플레이어 그리드의 셀 크기를 반환하는 함수
	void HideAllPlacementHighlights(); // 모든 그리드의 배치 하이라이트를 숨기는 함수


	/* 빠른 보관 시스템 */
	bool RequestQuickTransfer(UInventoryComponent* source, int64 instanceUID); // 빠른 보관을 수행하는 함수
	UInventoryComponent* GetQuickTransferTarget(UInventoryComponent* source) const; // 빠른 보관 대상 컨테이너를 반환하는 함수

	/* 부위 사용 시스템 */
	void BeginConsumableDrag(UItemDragDrop* op); // 소모품 드래그 시작 시 부위 사용 모드로 전환하는 함수
	void SetBodyPartModeActive(bool bActive); // 부위 사용 모드를 켜고 끄는 함수

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 버튼 클릭 시 실행되는 함수
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;   // 마우스 버튼 클릭 완료 시 실행되는 함수
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override; // 키 버튼 클릭 시 실행되는 함수


	/* 인벤토리 */
	UFUNCTION()
	void HandleInventoryChanged(); // 인벤토리 상태 변경 시 실행되는 함수

	UFUNCTION()
	void HandleEquipmentChanged(); // 장착 상태 변경 시 실행되는 함수

	UFUNCTION()
	void HandleQuickSlotChanged(EQuickSlot quickSlot); // 퀵슬롯 변경 시 실행되는 함수

	/* 부위 사용 시스템 */
	UFUNCTION()
	void HandleConsumableDragEnded(UDragDropOperation* operation); // 소모품 드래그가 끝났을 때 실행되는 함수

private:
	/* 초기화 헬퍼 함수 */
	void CollectEquipmentSlots(); // 장비 슬롯 위젯을 수집하는 함수
	void RefreshEquipmentSlots(); // 수집된 장비 슬롯을 일괄 갱신하는 함수
	void CollectConsumableSlots(); // 소모품 슬롯 위젯을 수집하는 함수
	void RefreshConsumableSlots(); // 수집된 소모품 슬롯을 일괄 갱신하는 함수
	void CollectBodyPartSlots(); // 트리에서 부위 슬롯 위젯을 수집하는 함수
	void RefreshBodyPartSlots(); // 수집된 부위 슬롯을 일괄 갱신하는 함수


	/* 캐싱 변수 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> owningInventory = nullptr; // 소유한 인벤토리 

	UPROPERTY()
	TArray<TObjectPtr<UEquipmentSlotWidget>> equipmentSlots; // 수집된 장비 슬롯 위젯 배열

	UPROPERTY()
	TArray<TObjectPtr<UConsumableSlotWidget>> consumableSlots; // 수집된 소모품 슬롯 위젯 배열


	/* 그리드 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UItemGridWidget> playerGrid; // 플레이어 인벤토리 그리드


	/* 아이템 */
	int64 hoveredUID = 0; // 현재 마우스가 올라간 아이템


	/* 우클릭 팝업 시스템 */
	void ValidatePopupTargets(); // 대상 아이템이 사라진 팝업을 닫는 함수

	UPROPERTY()
	TObjectPtr<UInventoryComponent> popupInventory = nullptr; // 현재 팝업이 대상으로 하는 인벤토리

	UPROPERTY(EditAnywhere, Category = "Inventory|Popup")
	TSubclassOf<UMyItemContextMenuWidget> contextMenuClass; // 우클릭 팝업 위젯 클래스

	UPROPERTY(EditAnywhere, Category = "Inventory|Popup")
	TSubclassOf<UItemSplitPopupWidget> splitPopupClass; // 나누기 팝업 위젯 클래스

	UPROPERTY(EditAnywhere, Category = "Inventory|Popup")
	TSubclassOf<UItemInfoPopupWidget> infoPopupClass; // 정보 팝업 위젯 클래스

	UPROPERTY(EditAnywhere, Category = "Inventory|Popup")
	int32 popupZOrder = 100; // 팝업 뷰포트 Z 순서

	UPROPERTY()
	TObjectPtr<UMyItemContextMenuWidget> contextMenu = nullptr; // 현재 열린 우클릭 팝업

	UPROPERTY()
	TObjectPtr<UItemSplitPopupWidget> splitPopup = nullptr; // 현재 열린 나누기 팝업

	UPROPERTY()
	TObjectPtr<UItemInfoPopupWidget> infoPopup = nullptr; // 현재 열린 정보 팝업


	/* 호버 정보 팝업 시스템 */
	void ShowHoverInfo(); // 대기 시간 경과 후 호버 정보 팝업을 여는 함수
	void CloseHoverInfo(); // 호버 정보 팝업을 닫는 함수

	UPROPERTY(EditAnywhere, Category = "Inventory|Popup")
	TSubclassOf<UItemHoverInfoWidget> hoverInfoClass; // 호버 정보 팝업 위젯 클래스

	UPROPERTY()
	TObjectPtr<UItemHoverInfoWidget> hoverInfo = nullptr; // 현재 열린 호버 정보 팝업

	UPROPERTY(EditAnywhere, Category = "Inventory|Popup", meta = (ClampMin = "0.0"))
	float hoverInfoDelay = 1.5f; // 호버 정보 팝업이 뜨기까지의 대기 시간

	FTimerHandle hoverInfoTimer; // 호버 대기 타이머 핸들
	int64 pendingHoverUID = 0; // 호버 대기 중인 아이템 인스턴스 UID


	/* 창고 시스템 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UItemGridWidget> storageGrid; // 창고 그리드

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> storagePanel; // 창고 패널

	UPROPERTY()
	TObjectPtr<UInventoryComponent> storageInventory = nullptr; // 창고 인벤토리 컴포넌트


	/* 부위 사용 시스템 */
	UPROPERTY()
	TArray<TObjectPtr<UBodyPartSlotWidget>> bodyPartSlots; // 수집된 부위 슬롯 위젯 배열

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> equipSlotPanel; // 장비 슬롯 패널

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> bodyPartPanel; // 부위 사용 패널

	bool bBodyPartModeActive = false; // 부위 사용 모드 활성 여부

public:

};