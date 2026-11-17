#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/SavePoint/SavePointMenuTypes.h"
#include "SavePointNavButtonWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSavePointNavClicked, ESavePointMenuPage, page);

UCLASS()
class WINDLESS_API USavePointNavButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 선택 설정 */
	void SetSelected(bool bInSelected); // 선택 페이지를 설정하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "SavePoint|Nav")
	FOnSavePointNavClicked onNavClicked; // 버튼 클릭 시 실행되는 델리게이트

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;


	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> navButton; // 클릭/포커스 버튼

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> labelText; // 기능 이름

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> highlightImage; // 선택/포커스 배경

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> iconImage; // 기능 아이콘

	UPROPERTY(EditAnywhere, Category = "SavePoint|Nav")
	ESavePointMenuPage page = ESavePointMenuPage::ESMP_Storage; // 버튼 페이지

	UPROPERTY(EditAnywhere, Category = "SavePoint|Nav")
	FText label = FText::FromString(TEXT("창고")); // 버튼 텍스트

	UPROPERTY(EditAnywhere, Category = "SavePoint|Nav")
	FText subtitle = FText::FromString(TEXT("STORAGE")); // 버튼 부제목

	UPROPERTY(EditAnywhere, Category = "SavePoint|Nav")
	FSlateBrush iconBrush; // 아이콘 브러시

	UPROPERTY(EditAnywhere, Category = "SavePoint|Nav")
	float iconAngle = 0.f; // 아이콘 회전


	/* 스타일 */
	UPROPERTY(EditAnywhere, Category = "SavePoint|Nav|Style")
	FSlateColor normalTextColor = FSlateColor(FLinearColor(FColor(190, 182, 164, 179))); // 일반 텍스트 색

	UPROPERTY(EditAnywhere, Category = "SavePoint|Nav|Style")
	FSlateColor highlightTextColor = FSlateColor(FLinearColor(FColor(244, 236, 214, 255))); // 강조 텍스트 색

	UPROPERTY(EditAnywhere, Category = "SavePoint|Nav|Style", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float focusOnlyOpacity = 0.6f; // 선택되지 않고 포커스만 있을 때 배경 불투명도

private:
	/* UI 설정 */
	UFUNCTION()
	void HandleClicked(); // 클릭 시 실행되는 함수

	UFUNCTION()
	void HandleHovered(); // 마우스 호버 시 포커스를 옮기는 함수

	void RefreshVisual(); // 강조 상태를 반영하는 함수

	bool bSelected = false; // 현재 열린 페이지 여부
	bool bFocused = false;  // 포커스 여부

public:
	/* 변수 getter 함수 */
	FORCEINLINE ESavePointMenuPage GetPage() const { return page; }
	FORCEINLINE const FText& GetLabel() const { return label; }
	FORCEINLINE const FText& GetSubtitle() const { return subtitle; }
	FORCEINLINE UButton* GetButton() const { return navButton; }
};