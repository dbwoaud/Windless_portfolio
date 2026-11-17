#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class WINDLESS_API UBossHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 표시 제어 */
	void ShowBar(const FText& bossName, float percent); // 보스 조우 시 체력 바를 표시하는 함수
	void HideBar(); // 체력 바를 숨기는 함수
	void SetHealthPercent(float percent); // 퍼센트 단위 보스 체력을 설정하는 함수

protected:
	virtual void NativeConstruct() override;

	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> bossHealthBar; // 보스 체력 바

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> bossNameText; // 보스 이름
};