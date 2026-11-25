#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RefineConfirmWidget.generated.h"

class UButton;
class UTextBlock;
class UInventoryComponent;

UCLASS()
class WINDLESS_API URefineConfirmWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void InitializeConfirm(UInventoryComponent* inventory, int64 instanceUID); // 확인 창을 초기화하는 함수

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> messageText; // 확인 문구 텍스트

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> confirmButton; // 정제 실행 버튼

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> cancelButton; // 취소 버튼

private:
	/* 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UInventoryComponent> targetInventory = nullptr; // 정제 대상 인벤토리


	/* 캐시 변수 */
	int64 targetUID = 0; // 정제 대상 인스턴스


	/* UI 동작 */
	UFUNCTION()
	void HandleConfirm(); // 정제를 실행하고 창을 닫는 함수

	UFUNCTION()
	void HandleCancel(); // 창을 닫는 함수
};