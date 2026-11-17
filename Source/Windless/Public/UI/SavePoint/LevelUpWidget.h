#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelUpWidget.generated.h"

class UButton;
class UTextBlock;
class UProgressBar;
class AWindlessCharacter;

UCLASS()
class WINDLESS_API ULevelUpWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LevelUp")
	void RefreshDisplay(); // 현재 레벨/탈피석/스탯 변화를 갱신하는 함수

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	/* 위젯 바인딩 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> levelText; // 현재 허물 텍스트

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> nextLevelText; // 다음 허물 텍스트

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> stoneText; // 보유 / 필요 탈피석 개수 텍스트

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> stoneBar; // 진행도 프로그레스 바

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> moltButton; // 탈피 실행 버튼

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> statPreviewText; // 스탯 변화 목록 텍스트

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> inventoryPreviewText; // 인벤토리 확장 텍스트


	/* 블루프린트용 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LevelUp")
	void OnMoltSucceededBP(); // 탈피 성공 시 실행되는 블루프린트용 함수

private:
	UFUNCTION()
	void HandleMoltClicked(); // 탈피 버튼 클릭 시 실행되는 함수

	AWindlessCharacter* GetPlayer() const;
};