#pragma once

#include "CoreMinimal.h"
#include "Characters/BodyPartTypes.h"
#include "Items/QuickSlotTypes.h"
#include "Blueprint/UserWidget.h"
#include "WindlessOverlay.generated.h"

class UProgressBar;
class UTextBlock;
class UWidget;
class UBossHealthBarWidget;
class UQuickSlotBarWidget;
enum class EChaosState : uint8;

UCLASS()
class WINDLESS_API UWindlessOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/* 생명력 */
	void SetHealthBarPercent(float percent); // 퍼센트 단위 생명력 바를 설정하는 함수


	/* 스태미나 */
	void SetStaminaBarPercent(float percent); // 퍼센트 단위 스태미나 바를 설정하는 함수
	void SetStaminaLockRatio(float ratio); // 스태미나 잠금 비율을 설정하는 함수


	/* 재화 */
	void SetMoltStones(int32 moltStone); // 현재 탈피석을 설정하는 함수


	/* 혼돈 시스템 */
	void SetChaosBarPercent(float percent); // 퍼센트 단위 혼돈 게이지 바를 설정하는 함수
	void SetChaosBarColor(EChaosState state); // 혼돈 단계에 따른 혼돈 게이지 색상을 설정하는 함수 


	/* 부위 시스템 */
	void UpdateBodyPart(EBodyPart part, float percent, bool bInjured, bool bDisabled); // 특정 부위의 상태를 업데이트하는 함수
	void ToggleBodyStatus(); // 부위 상태 UI를 토글하는 함수 
	void SetBodyStatusVisible(bool bVisible); // 부위 상태 UI를 활성화/비활성화하는 함수
	bool IsBodyStatusVisible() const; // 현재 부위 상태 UI가 활성화되었는지 확인하는 함수

	/* 보스 */
	void ShowBossHealthBar(const FText& bossName, float percent); // 보스 체력 바를 표시하는 함수
	void HideBossHealthBar(); // 보스 체력 바를 숨기는 함수
	void SetBossHealthPercent(float percent); // 보스 체력 비율을 설정하는 함수


	/* 퀵슬롯 */
	void UpdateQuickSlot(EQuickSlot quickSlot, UTexture2D* icon, int32 count); // 퀵슬롯 표시 내용을 갱신하는 함수
	void SetQuickSlotCooldown(EQuickSlot quickSlot, float ratio); // 퀵슬롯의 재사용 대기 표시를 갱신하는 함수
	void SetSelectedQuickSlot(EQuickSlot quickSlot); // 현재 선택된 퀵슬롯을 표시하는 함수
	void ClearQuickSlots(); // 모든 퀵슬롯 표시를 비우는 함수

private:
	/* 생명력 */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* healthProgressBar; // 생명력 바
	

	/* 스태미나 */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* staminaProgressBar; // 스태미나 바

	UPROPERTY(meta = (BindWidgetOptional))
	UProgressBar* staminaLockBar; // 스태미나 잠금 영역


	/* 재화 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* moltStoneCountText; // 탈피석 개수 텍스트


	/* 혼돈 시스템 */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* chaosProgressBar; // 혼돈 게이지 바


	/* 부위 시스템 */
	void SetHealthBarColor(bool bDisabled, bool bInjured, UProgressBar* bar); // 부위 상태에 따른 생명력 바 색상을 설정하는 함수

	UPROPERTY(meta = (BindWidgetOptional)) 
	UWidget* bodyStatusPanel; // 부위 상태 UI 패널

	UPROPERTY(meta = (BindWidgetOptional)) 
	UProgressBar* headBar; // 머리 부위 생명력 바

	UPROPERTY(meta = (BindWidgetOptional)) 
	UProgressBar* chestBar; // 가슴 부위 생명력 바 

	UPROPERTY(meta = (BindWidgetOptional)) 
	UProgressBar* leftArmBar; // 왼쪽 팔 부위 생명력 바 

	UPROPERTY(meta = (BindWidgetOptional)) 
	UProgressBar* rightArmBar; // 오른쪽 팔 부위 생명력 바 

	UPROPERTY(meta = (BindWidgetOptional)) 
	UProgressBar* leftLegBar; // 왼쪽 다리 부위 생명력 바

	UPROPERTY(meta = (BindWidgetOptional)) 
	UProgressBar* rightLegBar; // 오른쪽 다리 부위 생명력 바 


	/* 보스 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBossHealthBarWidget> bossHealthBarWidget; // 보스 체력 바 위젯


	/* 퀵슬롯 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UQuickSlotBarWidget> quickSlotBar; // 퀵슬롯 바 위젯
};