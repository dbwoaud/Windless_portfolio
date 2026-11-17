#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RefineDropWidget.generated.h"

class UTextBlock;
class UImage;
class URefineConfirmWidget;
class UInventoryComponent;

UCLASS()
class WINDLESS_API URefineDropWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> glowImage; // 드래그 중 강조되는 균열 이미지

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> hintText; // 안내 문구


	/* 설정 */
	UPROPERTY(EditAnywhere, Category = "Refine")
	TSubclassOf<URefineConfirmWidget> confirmWidgetClass; // 정제 확인창 클래스

	UPROPERTY(EditAnywhere, Category = "Refine")
	int32 confirmZOrder = 120; // 확인창 Z 순서

	UPROPERTY(EditAnywhere, Category = "Refine")
	FText refusalText = FText::FromString(TEXT("정제할 수 없는 아이템")); // 불가 안내

private:
	void SetHighlight(bool bActive, bool bAllowed); // 드롭 가능 여부를 시각적으로 표시하는 함수
};