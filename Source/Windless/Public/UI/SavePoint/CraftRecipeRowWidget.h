#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Crafts/CraftTypes.h"
#include "CraftRecipeRowWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UCraftWidget;

UCLASS()
class WINDLESS_API UCraftRecipeRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* UI 설정 */
	void Setup(const FCraftRecipeEntry& entry, UCraftWidget* owner); // 제작 레시피 위젯을 설정하는 함수
	void SetSelected(bool bInSelected); // 선택 상태 표시를 갱신하는 함수

protected:
	virtual void NativeOnInitialized() override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> rowButton; // 제작 버튼

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> nameText; // 레시피 이름 텍스트 

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> resultText; // 결과물 요약 텍스트 

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ingredientText; // 재료 요약 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> selectedImage; // 선택 표시 배경 이미지


	/* 스타일 */
	UPROPERTY(EditAnywhere, Category = "Craft|Style")
	FSlateColor craftableColor = FSlateColor(FLinearColor(FColor(244, 236, 214, 255))); // 제작 가능 텍스트 색

	UPROPERTY(EditAnywhere, Category = "Craft|Style")
	FSlateColor blockedColor = FSlateColor(FLinearColor(FColor(156, 148, 132, 204))); // 제작 불가 텍스트 색

private:
	/* 캐시 변수 */
	UPROPERTY()
	TObjectPtr<UCraftWidget> ownerWidget = nullptr; // 소유 제작 위젯
	FName recipeId = NAME_None; // 레시피 ID


	/* UI 설정 */
	UFUNCTION()
	void HandleClicked(); // 제작 버튼 클릭 시 실행되는 함수

public:
	/* 변수 Getter 함수 */
	FORCEINLINE FName GetRecipeId() const { return recipeId; }
};