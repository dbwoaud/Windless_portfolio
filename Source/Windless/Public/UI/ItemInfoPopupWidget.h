#pragma once

#include "CoreMinimal.h"
#include "UI/ItemInfoWidgetBase.h"
#include "ItemInfoPopupWidget.generated.h"

class UImage;
class UButton;


UCLASS()
class WINDLESS_API UItemInfoPopupWidget : public UItemInfoWidgetBase
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	virtual void SetupInfo(UInventoryWidget* inOwnerWidget, UInventoryComponent* inInventory, int64 uid); // 대상 아이템 정보를 표시하는 함수


	/* 닫기 */
	void ClosePopup(); // 팝업을 닫는 함수

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override; // 키 입력 시 실행되는 함수
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override; // 마우스 클릭 시 실행되는 함수


	virtual void ApplyItemInfo(const FItemInstance& inst, const FItemData& data) override; // 공통 정보를 위젯에 적용하는 함수
	virtual void HandleInvalidTarget() override; // 대상 아이템이 유효하지 않을 때 실행되는 함수


	/* UI 동작 */
	UFUNCTION()
	void HandleCloseClicked(); // 닫기 버튼 클릭 시 실행되는 함수


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> iconImage; // 아이템 아이콘

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> iconFrame; // 아이콘 등급 테두리

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> descriptionText; // 설명문

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> closeButton; // 닫기 버튼

private:

};