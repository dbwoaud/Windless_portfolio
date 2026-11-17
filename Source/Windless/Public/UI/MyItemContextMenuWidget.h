#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemTypes.h"
#include "UI/ContextMenuButtonWidget.h"
#include "MyItemContextMenuWidget.generated.h"

class UCanvasPanel;
class UVerticalBox;
class UBorder;
class UTextBlock;
class UInventoryComponent;
class UInventoryWidget;

UCLASS()
class WINDLESS_API UMyItemContextMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void SetupContextMenu(UInventoryWidget* inOwnerWidget, UInventoryComponent* inInventory,
		int64 uid, EEquipSlot slot, const FVector2D& screenPos); // 대상 아이템으로 팝업을 구성하는 함수
	static FText GetBodyPartDisplayName(EBodyPart part); // 부위 텍스트를 반환하는 함수


	/* 닫기 */
	void CloseMenu(); // 팝업을 닫는 함수

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override; // 키 입력 시 실행되는 함수
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 클릭 시 실행되는 함수


	/* 버튼 구성 */
	void ClearActions(); // 기존 버튼을 모두 제거하는 함수
	void BuildActions(const FItemInstance& inst, const FItemData& data); // 아이템 조건에 맞는 버튼을 생성하는 함수
	void BuildBodyPartActions(); // 부위 선택 서브메뉴 버튼을 생성하는 함수
	void AddActionButton(EItemContextAction action, const FText& label, bool bEnabled, EBodyPart part = EBodyPart::EBP_None); // 버튼 1개를 생성해 등록하는 함수


	/* 배치 */
	void PlaceMenuAtCursor(); // 커서 위치에 팝업을 배치하는 함수


	/* 동작 실행 */
	void HandleActionClicked(EItemContextAction action, EBodyPart part); // 버튼 동작을 실행하는 함수
	void RequestItemCast(EBodyPart part); // 캐릭터에게 아이템 시전을 요청하는 함수


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> rootCanvas; // 팝업 루트 캔버스

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> menuBorder; // 실제 팝업 상자

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> actionBox; // 버튼이 쌓이는 세로 박스

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> backdrop; // 팝업 외부 클릭 감지용 전체 화면 배경

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> headerText; // 아이템 이름 헤더


	/* 설정값 */
	UPROPERTY(EditAnywhere, Category = "ContextMenu")
	TSubclassOf<UContextMenuButtonWidget> buttonWidgetClass; // 팝업 버튼 위젯 클래스

	UPROPERTY(EditAnywhere, Category = "ContextMenu")
	FVector2D screenEdgePadding = FVector2D(8.f, 8.f); // 화면 가장자리 여백

private:
	/* 캐싱 변수 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> owningInventory = nullptr; // 소유 인벤토리

	UPROPERTY()
	TObjectPtr<UInventoryWidget> owningInventoryWidget = nullptr; // 소유 인벤토리 위젯

	UPROPERTY()
	TArray<TObjectPtr<UContextMenuButtonWidget>> spawnedButtons; // 생성된 버튼 배열

	int64 targetUID = 0; // 대상 아이템 인스턴스 UID
	EEquipSlot targetSlot = EEquipSlot::EES_None; // 장비 슬롯에서 열렸을 때의 슬롯
	FVector2D cursorScreenPos = FVector2D::ZeroVector; // 팝업을 연 커서 좌표

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "ContextMenu")
	FORCEINLINE int64 GetTargetUID() const { return targetUID; }
};