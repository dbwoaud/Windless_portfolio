#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemGridWidget.generated.h"

class UImage;
class USizeBox;
class UCanvasPanel;
class UUniformGridPanel;
class UItemSlotWidget;
class UInventoryComponent;
class UInventoryWidget;
class UItemDragDrop;

UCLASS()
class WINDLESS_API UItemGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void InitializeGrid(UInventoryComponent* inventory, UInventoryWidget* screenWidget); // 대상 인벤토리와 화면 위젯을 설정하는 함수
	void RefreshGrid(); // 배경 격자와 아이템 배치를 다시 구성하는 함수


	/* 하이라이트 */
	void HidePlacementHighlight(); // 하이라이트 이미지를 숨기는 함수

protected:
	virtual void NativeConstruct() override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 마우스 드래그 중일 때 실행되는 함수
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 마우스 드래그 중인 아이템이 위젯 영역 밖으로 나갔을 때 실행되는 함수
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // // 마우스 드래그 완료 시 실행되는 함수
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 드래그 취소 시 실행되는 함수

	/* 초기화 함수 */
	void BuildBackgroundGrid(); // 배경 셀 격자를 생성하는 함수
	void RebuildItems(); // 아이템 인스턴스 위젯을 캔버스에 재배치하는 함수


	/* 하이라이트 */
	void CreatePlacementHighlight(); // 하이라이트 이미지를 생성하는 함수
	void UpdatePlacementHighlight(const FVector2D& screenPos, UItemDragDrop* op); // 하이라이트 위치를 갱신하는 함수

	
	/* 마우스 드롭 */
	bool ResolveDropCell(const FGeometry& InGeometry, const FVector2D& screenPos, UItemDragDrop* op, int32& outX, int32& outY) const; // 드롭 대상 셀을 계산하는 함수


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> itemCanvas; // 아이템이 배치될 캔버스

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UUniformGridPanel> backgroundGrid; // 배경 셀 격자

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> gridSizeBox; // 격자 전체 크기를 감싸는 영역

	/* 설정값 */
	UPROPERTY(EditAnywhere, Category = "Grid")
	float cellSize = 48.f; // 셀 한 칸의 픽셀 크기

	UPROPERTY(EditAnywhere, Category = "Grid")
	TSubclassOf<UItemSlotWidget> slotWidgetClass; // 아이템 인스턴스 위젯 클래스

	UPROPERTY(EditAnywhere, Category = "Grid")
	TSubclassOf<UUserWidget> backgroundCellClass; // 배경 셀 위젯 클래스

	UPROPERTY(EditAnywhere, Category = "Grid|Preview")
	FLinearColor validColor = FLinearColor(0.f, 1.f, 0.f, 0.25f); // 배치 가능 색

	UPROPERTY(EditAnywhere, Category = "Grid|Preview")
	FLinearColor invalidColor = FLinearColor(1.f, 0.f, 0.f, 0.25f); // 배치 불가 색

private:
	/* 캐싱 변수 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> owningInventory = nullptr; // 인벤토리 컴포넌트

	UPROPERTY()
	TObjectPtr<UInventoryWidget> owningScreen = nullptr; // 소유 화면 위젯

	UPROPERTY()
	TArray<TObjectPtr<UItemSlotWidget>> spawnedItemWidgets; // 생성된 아이템 위젯 배열

	UPROPERTY()
	TObjectPtr<UImage> placementHighlight = nullptr; // 드롭 위치 하이라이트 이미지

	int32 builtGridWidth = 0;  // 배경 격자를 마지막으로 만든 가로 칸 수
	int32 builtGridHeight = 0; // 배경 격자를 마지막으로 만든 세로 칸 수

public:
	/* 변수 getter 함수 */
	FORCEINLINE float GetCellSize() const { return cellSize; }
	FORCEINLINE void SetCellSize(float size) { cellSize = size; } // 격자를 만들기 전에 부른다
	FORCEINLINE UInventoryComponent* GetInventory() const { return owningInventory; }
};
