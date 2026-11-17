#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Crafts/CraftTypes.h"
#include "CraftWidget.generated.h"

class UPanelWidget;
class UTextBlock;
class UButton;
class USlider;
class UCraftingComponent;
class UCraftRecipeRowWidget;

UCLASS()
class WINDLESS_API UCraftWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* UI 설정 */
	UFUNCTION(BlueprintCallable, Category = "Craft")
	void RefreshDisplay(); // 레시피 목록과 상세를 다시 만드는 함수
	
	UFUNCTION(BlueprintCallable, Category = "Craft")
	void SetCategory(ECraftCategory newCategory); // 분류 탭을 전환하는 함수


	/* 제작 */
	void SelectRecipe(FName recipeId); // 레시피를 선택하는 함수

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> recipeList; // 레시피 목록 컨테이너

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> craftButton; // 제작 실행 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> emptyRecipeText; // 빈 레시피 안내 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> detailNameText; // 선택 레시피 이름 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> detailResultText; // 선택 레시피 결과물 텍스트 

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> detailIngredientText; // 선택 레시피 재료 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> countText; // 제작 수량 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USlider> countSlider; // 수량 슬라이더

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> blockReasonText; // 제작 불가 사유 텍스트

	/* 분류 탭 버튼 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> tabConsumableButton; // 소비 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> tabEquipmentButton; // 장비 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> tabToolButton; // 도구 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> tabFacilityButton; // 시설 버튼


	/* 설정 */
	UPROPERTY(EditAnywhere, Category = "Craft")
	TSubclassOf<UCraftRecipeRowWidget> recipeRowClass; // 레시피 행 위젯 클래스

	UPROPERTY(EditAnywhere, Category = "Craft")
	FMargin recipeRowPadding = FMargin(0.f, 0.f, 0.f, 6.f); // 행 간격


	/* 블루프린트용 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Craft")
	void OnCraftSucceededBP(); // 제작 성공 시 실행되는 블루프린트용 함수

private:
	/* UI 설정 */
	UFUNCTION()
	void HandleCraftClicked(); // 제작 버튼 클릭 시 실행되는 함수

	UFUNCTION()
	void HandleSliderChanged(float value); // 슬라이더 변경 시 실행되는 함수

	UFUNCTION() 
	void HandleTabConsumable(); // 소비 버튼 클릭 시 실행되는 함수

	UFUNCTION() 
	void HandleTabEquipment(); // 장비 버튼 클릭 시 실행되는 함수

	UFUNCTION() 
	void HandleTabTool(); // 도구 버튼 클릭 시 실행되는 함수

	UFUNCTION() 
	void HandleTabFacility(); // 시설 버튼 클릭 시 실행되는 함수

	void RebuildList();     // 목록을 다시 만드는 함수
	void RefreshDetail();   // 우측 상세와 수량 컨트롤을 갱신하는 함수


	/* 조회 */
	UCraftingComponent* GetCrafting() const;


	/* 분류 */
	ECraftCategory currentCategory = ECraftCategory::ECC_Consumable; // 현재 분류
	UPROPERTY()
	TArray<FCraftRecipeEntry> entries; // 현재 분류의 레시피 목록

	UPROPERTY()
	TArray<TObjectPtr<UCraftRecipeRowWidget>> rows; // 생성된 행


	/* 레시피 */
	FName selectedRecipeId = NAME_None; // 선택한 레시피


	/* 제작 */
	int32 craftCount = 1; // 제작 수량
};