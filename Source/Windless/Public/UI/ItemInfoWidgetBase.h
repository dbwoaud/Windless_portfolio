#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemTypes.h"
#include "ItemInfoWidgetBase.generated.h"

class UTextBlock;
class UBorder;
class UInventoryComponent;
class UInventoryWidget;

UCLASS(Abstract)
class WINDLESS_API UItemInfoWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	virtual void SetupInfo(UInventoryWidget* inOwnerWidget, UInventoryComponent* inInventory, int64 uid); // 대상 아이템으로 정보를 설정하는 함수
	virtual void RefreshInfo(); // 대상 아이템 정보를 업데이트하는 함수

protected:
	/* UI 업데이트 */
	bool FindTargetItem(FItemInstance& outInstance, FItemData& outData) const; // 대상 인스턴스와 원본 데이터를 조회하는 함수
	virtual void ApplyItemInfo(const FItemInstance& inst, const FItemData& data); // 공통 정보를 위젯에 적용하는 함수
	virtual void HandleInvalidTarget(); // 대상 아이템이 유효하지 않을 때 실행되는 함수


	/* 텍스트 설정 */
	FText SetCategoryText(const FItemData& data) const; // 카테고리 텍스트를 설정하는 함수
	FText SetStatText(const FItemInstance& inst, const FItemData& data) const; // 스테이터스 텍스트를 설정하는 함수


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> popupBorder; // 팝업 상자

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> itemNameText; // 아이템 이름

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> categoryText; // 카테고리/등급 문구

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> statText; // 스테이터스 문구


	/* 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> owningInventory = nullptr; // 소유 인벤토리

	UPROPERTY()
	TObjectPtr<UInventoryWidget> owningInventoryWidget = nullptr; // 소유 인벤토리 위젯


	/* 캐시 */
	int64 targetUID = 0; // 대상 아이템 인스턴스 UID

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "Info")
	FORCEINLINE int64 GetTargetUID() const { return targetUID; }
};