#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Enemy/AI/EnemyAITypes.h"
#include "EnemyAttackRow.generated.h"

class USoundBase;
class UAnimMontage;

/* 적 공격 1개의 공격 데이터 */
USTRUCT(BlueprintType)
struct WINDLESS_API FEnemyAttackRow : public FTableRowBase
{
	GENERATED_BODY()

	/* 분류 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	FText attackName; // 공격 이름 텍스트

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	EEnemyCard card = EEnemyCard::EC_LightAttack; // 이 공격이 채우는 카드

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class", meta = (ClampMin = "0.0"))
	float shareWeight = 1.f; // 같은 분류 공격끼리의 점수 비율

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	EEnemyAttackClass attackClass = EEnemyAttackClass::EATC_Light; // 공격 종류

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class")
	EEnemyComboType comboType = EEnemyComboType::ECT_Branch; // 공격 콤보 종류


	/* 사거리 및 콤보 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range")
	TArray<float> hitRanges; // 콤보 공격 사거리

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0.0"))
	float minRange = 0.f; // 최소 사거리

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range")
	TArray<float> hitIntervals; // 콤보 공격 간격

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float comboContinueChance = 0.9f; // 다음 콤보가 사거리 안일 때 이어갈 확률 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float outOfRangeContinue = 0.25f; // 다음 콤보가 사거리 밖일 때 이어갈 확률


	/* 공격 구간 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	TArray<float> windups; // 콤보 예고 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float windupHoldTime = 0.f; // 콤보 엇박 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float holdReleaseTime = 0.f; // 콤보 정지 후 판정 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	TArray<float> activeTimes; // 콤보별 판정 길이

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float recovery = 0.8f; // 콤보 후딜

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float cancelWindowStart = 0.f; // 캔슬창 시작

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float cancelWindowEnd = 0.f; // 캔슬창 끝

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float comboWindowStart = 0.f; // 콤보 분기창 시작

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing", meta = (ClampMin = "0.0"))
	float comboWindowEnd = 0.f; // 콤보 분기창 끝

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timing")
	TArray<EEnemyTrackProfile> trackProfiles; // 콤보별 추적 방식


	/* 피해 및 경직 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit", meta = (ClampMin = "0.0"))
	float impact = 10.f; // 충격량

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit")
	bool bCanStaggerStrong = false; // 강한 경직 유발 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit", meta = (ClampMin = "0.0"))
	float damageMultiplier = 1.f; // 데미지 배율

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit")
	EEnemyAddedDamage addedDamage = EEnemyAddedDamage::EAD_None; // 추가 피해

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hit")
	bool bGuardWhile = false; // 공격의 가드 유지 여부


	/* 발동 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger", meta = (ClampMin = "0.0"))
	float cooldown = 0.f; // 재사용 대기 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float attackAngleH = 30.f; // 발동 수평 반각도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float attackAngleUp = 25.f; // 발동 위쪽 각도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float attackAngleDown = 20.f; // 발동 아래쪽 각도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger", meta = (ClampMin = "0.0"))
	float preTurnTime = 1.f; // 선회 제한 시간


	/* 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<USoundBase> windupSound; // 예고 효과음

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<UAnimMontage> montage; // 공격 애니메이션

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	FName montageSection = NAME_None; // 재생 애니메이션 섹션 이름


	/* 헬퍼 함수 */
	FORCEINLINE int32 GetHitCount() const { return hitRanges.Num(); } // 콤보 횟수를 반환하는 함수
	FORCEINLINE float GetRange() const { return hitRanges.Num() > 0 ? hitRanges[0] : 0.f; } // 1타 사거리를 반환하는 함수
	FORCEINLINE bool HasHold() const { return windupHoldTime > 0.f; } // 엇박 여부를 반환하는 함수
	FORCEINLINE float GetWindup(int32 hitIndex) const { return windups.IsValidIndex(hitIndex) ? windups[hitIndex] : 0.f; } // 콤보별 예고를 반환하는 함수
	FORCEINLINE float GetActiveTime(int32 hitIndex) const { return activeTimes.IsValidIndex(hitIndex) ? activeTimes[hitIndex] : 0.1f; } // 콤보별 판정 길이를 반환하는 함수
	FORCEINLINE EEnemyTrackProfile GetTrackProfile(int32 hitIndex) const { return trackProfiles.IsValidIndex(hitIndex) ? trackProfiles[hitIndex] : EEnemyTrackProfile::ETP_Decay; } // 콤보별 추적 방식을 반환하는 함수
	bool UsesMultiHitTracking() const; // 콤보 2타 이후 끝까지 추적이 있는지 확인하는 함수

	/* 데이터 검사 */
	virtual void OnPostDataImport(const UDataTable* inDataTable, const FName inRowName, TArray<FString>& outCollectedImportProblems) override;
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& context) const override;
#endif

};