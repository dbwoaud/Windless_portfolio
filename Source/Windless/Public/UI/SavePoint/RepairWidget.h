#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/Repairs/RepairTypes.h"
#include "RepairWidget.generated.h"

class UPanelWidget;
class UTextBlock;
class URepairComponent;
class URepairToolRowWidget;
class URepairTargetRowWidget;

UCLASS()
class WINDLESS_API URepairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 수리 시스템 */
	void SelectTool(int64 toolUID); // 사용할 수리도구를 선택하는 함수
	bool RepairTarget(int64 targetUID); // 선택한 도구로 장비를 수리하는 함수


	/* UI 설정 */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	void RefreshDisplay(); // 수리도구/수리 대상 목록을 업데이트하는 함수

protected:
	virtual void NativeConstruct() override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> toolList; // 좌측 수리도구 목록 컨테이너

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> targetList; // 우측 장착 장비 목록 컨테이너

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> emptyToolText; // 수리도구가 없을 때 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> emptyTargetText; // 수리 대상이 없을 때 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> selectedToolText; // 선택한 도구 이름과 잔여 충전량


	/* 행 위젯 클래스 */
	UPROPERTY(EditAnywhere, Category = "Repair")
	TSubclassOf<URepairToolRowWidget> toolRowClass; // 수리도구 행 위젯

	UPROPERTY(EditAnywhere, Category = "Repair")
	TSubclassOf<URepairTargetRowWidget> targetRowClass; // 장착 장비 행 위젯

	UPROPERTY(EditAnywhere, Category = "Repair")
	FMargin toolRowPadding = FMargin(0.f, 0.f, 0.f, 6.f); // 도구 행 간격

	UPROPERTY(EditAnywhere, Category = "Repair")
	FMargin targetRowPadding = FMargin(0.f, 0.f, 0.f, 8.f); // 장비 행 간격


	/* 블루프린트용 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Repair")
	void OnRepairSucceededBP(); // 수리 성공 시 실행되는 블루프린트용 함수

private:
	/* 수리 시스템 */
	UPROPERTY()
	TArray<FRepairToolEntry> tools; // 현재 수리도구 목록

	UPROPERTY()
	TArray<TObjectPtr<URepairToolRowWidget>> toolRows; // 생성된 도구 행

	int64 selectedToolUID = 0; // 선택한 수리도구

	
	/* UI 설정 */
	void RebuildToolList();      // 좌측 목록을 업데이트하는 함수
	void RebuildTargetList();    // 우측 목록을 업데이트하는 함수
	void RefreshToolSelection(); // 선택 표시와 요약 문구를 업데이트하는 함수

	/* 변수 Getter 함수 */
	URepairComponent* GetRepairComponent() const;

public:
	/* 변수 Getter 함수 */
	FORCEINLINE int64 GetSelectedToolUID() const { return selectedToolUID; }
};