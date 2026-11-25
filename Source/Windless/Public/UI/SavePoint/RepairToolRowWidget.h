#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Repairs/RepairTypes.h"
#include "RepairToolRowWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class URepairWidget;

UCLASS()
class WINDLESS_API URepairToolRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* UI 설정 */
	void Setup(const FRepairToolEntry& entry, URepairWidget* owner); // 수리도구 행 위젯을 초기화하는 함수
	void SetSelected(bool bInSelected); // 선택 상태 표시를 갱신하는 함수

protected:
	virtual void NativeOnInitialized() override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> rowButton; // 행 버튼

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> nameText; // 도구 이름 텍스트

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> chargeText; // 충전량 표시 텍스트 

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> selectedImage; // 선택 표시 배경 이미지


	/* 스타일 */
	UPROPERTY(EditAnywhere, Category = "Repair|Style")
	FSlateColor normalTextColor = FSlateColor(FLinearColor(FColor(200, 192, 175, 255))); // 기본 텍스트 색

	UPROPERTY(EditAnywhere, Category = "Repair|Style")
	FSlateColor selectedTextColor = FSlateColor(FLinearColor(FColor(244, 236, 214, 255))); // 선택 텍스트 색

private:
	/* UI 설정 */
	UPROPERTY()
	TObjectPtr<URepairWidget> ownerWidget = nullptr; // 소유 수리 위젯

	UFUNCTION()
	void HandleClicked(); // 행 버튼 클릭 시 실행되는 함수 


	/* 수리 시스템 */
	int64 toolUID = 0; // 이 줄이 가리키는 수리도구

public:
	/* 변수 Getter 함수 */
	FORCEINLINE int64 GetToolUID() const { return toolUID; }
};