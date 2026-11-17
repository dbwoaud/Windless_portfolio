#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/SavePoint/SavePointMenuTypes.h"
#include "SavePointMenuWidget.generated.h"

class UWidgetSwitcher;
class UTextBlock;
class USavePointNavButtonWidget;
class UInventoryWidget;
class ULevelUpWidget;
class URefineComponent;
class URepairWidget;
class UCraftWidget;
class UFastTravelWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSavePointMenuCloseRequested);

UCLASS()
class WINDLESS_API USavePointMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void InitializeMenu(const FText& savePointName, int32 currencyAmount); // 메뉴를 열 때 초기화하는 함수


	/* 페이지 이동 */
	UFUNCTION(BlueprintCallable, Category = "SavePoint|Menu")
	void OpenPage(ESavePointMenuPage newPage); // 페이지를 전환하는 함수

	UFUNCTION(BlueprintCallable, Category = "SavePoint|Menu")
	void Back(); // 페이지를 되돌아가는 함수


	/* UI 설정 */
	void FocusCurrentNav(); // 현재 페이지의 버튼에 포커스를 주는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "SavePoint|Menu")
	FOnSavePointMenuCloseRequested onCloseRequested; // 필드 상태에서 뒤로가기 시 실행되는 델리게이트

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> pageRoot; // 페이지 루트

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> pageSwitcher; // 기능별 페이지

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> titleText; // 머리말 제목

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> subtitleText; // 머리말 부제

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> currencyText; // 탈피석 수량

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> fieldShade; // 필드 상태 좌측 음영

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UInventoryWidget> storageInventoryWidget; // 창고에 배치한 인벤토리 위젯

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<ULevelUpWidget> levelUpWidget; // 레벨업 페이지 위젯

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<URepairWidget> repairWidget; // 수리 페이지 위젯

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCraftWidget> craftWidget; // 제작 페이지 위젯

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UFastTravelWidget> fastTravelWidget; // 빠른 이동 페이지 위젯


	/* 설정 */
	UPROPERTY(EditAnywhere, Category = "SavePoint|Menu")
	FText fieldSubtitle = FText::FromString(TEXT("SAVE POINT")); // 필드 상태 부제

	UPROPERTY(EditAnywhere, Category = "SavePoint|Menu")
	FText defaultSavePointName = FText::FromString(TEXT("은신처")); // 기본 세이브 포인트 이름

	UPROPERTY(EditAnywhere, Category = "SavePoint|Menu")
	TArray<FKey> backKeys = { EKeys::Escape, EKeys::BackSpace, EKeys::Gamepad_FaceButton_Right }; // 뒤로가기 키


	/* 블루프린트용 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SavePoint|Menu")
	void OnPageChangedBP(ESavePointMenuPage newPage, bool bFullscreen); // 페이지 전환 시 실행되는 블루프린트용 함수

private:
	/* UI 설정 */
	void RefreshHeader(); // 머리말을 갱신하는 함수
	void RefreshNavSelection(); // 버튼 선택 표시를 갱신하는 함수
	void FocusCurrentNavDeferred(); // 다음 프레임에 포커스를 지정하는 함수
	void SetupStoragePage(); // 창고 페이지에 통합 창고를 연결하는 함수
	void RefreshCurrency(); // 우상단 탈피석 표시를 갱신하는 함수

	UFUNCTION()
	void HandleNavClicked(ESavePointMenuPage page); // 기능 버튼 클릭 시 실행되는 함수

	ESavePointMenuPage currentPage = ESavePointMenuPage::ESMP_None;       // 현재 페이지
	ESavePointMenuPage lastOpenedPage = ESavePointMenuPage::ESMP_Storage; // 마지막 페이지


	/* 버튼 */
	USavePointNavButtonWidget* FindNavButton(ESavePointMenuPage page) const; // 페이지에 해당하는 버튼을 찾는 함수

	UPROPERTY()
	TArray<TObjectPtr<USavePointNavButtonWidget>> navButtons; // 기능 버튼 배열


	/* 세이브포인트 */
	FText savePointTitle; // 세이브포인트 이름


	/* 탈피(레벨업) */
	UFUNCTION()
	void HandleMoltingChanged(int32 newLevel, float expPercent); // 탈피석/허물 변경을 처리하는 함수

public:
	/* 변수 Getter 함수 */
	UFUNCTION(BlueprintPure, Category = "SavePoint|Menu")
	FORCEINLINE ESavePointMenuPage GetCurrentPage() const { return currentPage; }
};