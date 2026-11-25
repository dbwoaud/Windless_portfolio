#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/BodyPartTypes.h"
#include "BodyPartSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class UInventoryComponent;
class UBodyPartComponent;
class UInventoryWidget;

UCLASS()
class WINDLESS_API UBodyPartSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void InitializeSlot(UInventoryComponent* inventory, UBodyPartComponent* bodyParts, UInventoryWidget* ownerWidget); // 컴포넌트와 슬롯을 초기화하는 함수
	void RefreshSlot(); // 부위 체력 표시를 갱신하는 함수
	void ClearHighlight(); // 하이라이트를 기본 상태로 되돌리는 함수

protected:
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 드래그 중 실행되는 함수
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 드래그가 영역 밖으로 나갔을 때 실행되는 함수
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override; // 드롭 시 실행되는 함수

private:
	void SetHighlight(bool bUsable); // 사용 가능 여부를 색으로 표시하는 함수


	/* 캐싱 변수 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> owningInventory = nullptr; // 소유 인벤토리

	UPROPERTY()
	TObjectPtr<UBodyPartComponent> owningBodyParts = nullptr; // 소유 부위 컴포넌트

	UPROPERTY()
	TObjectPtr<UInventoryWidget> owningInventoryWidget = nullptr; // 소유 인벤토리 위젯


	/* 부위 */
	UPROPERTY(EditAnywhere, Category = "BodyPart")
	EBodyPart partType = EBodyPart::EBP_None; // 이 위젯이 담당하는 신체 부위


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> partImage; // 부위 실루엣 이미지

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> hpBar; // 부위 체력 바

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> hpText; // 부위 체력 텍스트


	/* 설정값 */
	UPROPERTY(EditAnywhere, Category = "BodyPart|Color")
	FLinearColor normalColor = FLinearColor(0.35f, 0.34f, 0.30f, 1.f); // 기본 실루엣 색

	UPROPERTY(EditAnywhere, Category = "BodyPart|Color")
	FLinearColor usableColor = FLinearColor(0.10f, 0.75f, 0.20f, 1.f); // 사용 가능 색

	UPROPERTY(EditAnywhere, Category = "BodyPart|Color")
	FLinearColor unusableColor = FLinearColor(0.75f, 0.10f, 0.10f, 1.f); // 사용 불가 색

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "BodyPart")
	FORCEINLINE EBodyPart GetPartType() const { return partType; }
};