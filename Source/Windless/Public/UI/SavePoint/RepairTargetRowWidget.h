#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Repairs/RepairTypes.h"
#include "RepairTargetRowWidget.generated.h"

class UButton;
class UTextBlock;
class UProgressBar;
class URepairWidget;

UCLASS()
class WINDLESS_API URepairTargetRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* UI 설정 */
	void Setup(const FRepairTargetEntry& entry, URepairWidget* owner); // 수리 대상 행 위젯을 초기화하는 함수

protected:
	virtual void NativeOnInitialized() override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> repairButton; // 수리 실행 버튼

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> nameText; // 장비 이름 텍스트

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> durabilityText; // 내구도 표시 텍스트 

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> durabilityBar; // 내구도 게이지 프로그레스 바


	/* 스타일 */
	UPROPERTY(EditAnywhere, Category = "Repair|Style")
	FLinearColor normalBarColor = FLinearColor(FColor(198, 168, 107, 255)); // 기본 바 색

	UPROPERTY(EditAnywhere, Category = "Repair|Style")
	FLinearColor brokenBarColor = FLinearColor(FColor(180, 74, 66, 255)); // 비활성화 바 색

	UPROPERTY(EditAnywhere, Category = "Repair|Style")
	FText brokenSuffix = FText::FromString(TEXT("  [파괴]")); // 파괴 표시 텍스트

private:
	/* 수리 시스템 */
	UPROPERTY()
	TObjectPtr<URepairWidget> ownerWidget = nullptr; // 소유 수리 위젯

	int64 targetUID = 0; // 수리 대상 장비 ID


	/* UI 설정 */
	UFUNCTION()
	void HandleRepairClicked(); // 수리 버튼 클릭 시 실행되는 함수

public:
	FORCEINLINE int64 GetTargetUID() const { return targetUID; }

};