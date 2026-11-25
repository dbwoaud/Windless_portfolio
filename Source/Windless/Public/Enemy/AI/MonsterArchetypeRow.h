#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Enemy/AI/EnemyAITypes.h"
#include "MonsterArchetypeRow.generated.h"

/* 몬스터 원형 1종 */
USTRUCT(BlueprintType)
struct WINDLESS_API FMonsterArchetypeRow : public FTableRowBase
{
	GENERATED_BODY()

	/* 몬스터 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FString monsterId; // 몬스터 ID

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText archetypeName; // 원형 이름

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMonsterGrade grade = EMonsterGrade::EGRD_Normal; // 등급

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMonsterAppearance appearance = EMonsterAppearance::EAPP_Aberrant; // 외형

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EEnemyTemperament temperament = EEnemyTemperament::ETMP_Default; // 성향

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EEnemyCombatRole combatRole = EEnemyCombatRole::ECR_Melee; // 사거리에 따른 공격 카드

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMonsterRangeClass rangeClass = EMonsterRangeClass::ERC_Melee; // 사거리 종류

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMonsterAggression aggression = EMonsterAggression::EAGG_Aggressive; // 선공 / 비선공

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	bool bAttackDirect = true; // 직접 공격 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	bool bAttackSupport = false; // 지원형 공격 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	bool bAttackSummon = false; // 소환형 공격 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	EMonsterMoveClass moveClass = EMonsterMoveClass::EMC_Mobile; // 이동형 / 고정형

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName faction = FName("Dueoksini"); // 세력 이름


	/* 공격 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float attackPower = 10.f; // 원형 공격력

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TArray<FName> attackIds; // 공격 목록


	/* 행동 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float nearRange = 0.f; // 가까이 상한

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float midRange = 0.f; // 중간 상한

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float farRange = 0.f; // 멀리 상한

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float rangeHysteresis = 0.5f; // 구간 완충

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float preferredRange = 1.8f; // 다가가기 임계값

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float runDistance = 5.f; // 달리기 임계값

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float walkSpeed = 1.6f; // 걷기 속도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float runSpeed = 4.2f; // 달리기 속도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float minAttackInterval = 0.7f; // 최소 공격 딜레이

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float repeatSuppression = 0.3f; // 반복 억제 배율

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float turnRateIdle = 90.f; // 평소 회전 속도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (ClampMin = "0.0"))
	float turnRateCombat = 180.f; // 전투 회전 속도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	TArray<FEnemyCardScoreOverride> cardScoreOverrides; // 카드 점수 덮어쓰기

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	TArray<FEnemySpecialCard> specialCards; // 원형 특수 카드

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bCanPassNarrowLedge = true; // 좁은 지형 통과 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bCanPassDoor = true; // 문 통과 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bRetreatWhenClose = false; // 거리 유지형 여부


	/* 몸 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "0.0"))
	float maxHP = 100.f; // 전체 HP

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "0.0"))
	float toughness = 10.f; // 강인도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	bool bKnockbackImmune = false; // 넉백 면역 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	bool bCanGuard = false; // 방패 보유 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	EEnemyGuardPolicy guardPolicy = EEnemyGuardPolicy::EGP_None; // 가드 종류

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float guardArcH = 60.f; // 가드 정면 반각

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float guardOnAttackChance = 0.f; // 휘두르기 시작에 가드로 전환할 확률

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float projectileGuardChance = 0.f; // 투사체에 가드로 전환할 확률

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "0.0"))
	float staminaMax = 100.f; // 몬스터 스태미나

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	bool bLowStaminaRetreat = true; // 스태미나 30% 이하 시 후퇴 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	EEnemyOverride canDodgeRoll = EEnemyOverride::EOVR_Default; // 반응 회피 종류

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float dodgeChance = 0.f; // 반응 회피 확률

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float dodgeStaminaCost = 0.3f; // 반응 회피 1회 스태미나 비율

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "0"))
	int32 weakPointCount = 2; // 약점 수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	TArray<FName> weakPointCandidates; // 약점 후보 부위

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Body")
	TArray<FName> fixedWeakPoints; // 보스 고정 약점


	/* 고유 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique")
	bool bHasConvulsion = true; // 경련 보유 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0"))
	float convulsionInterval = 18.f; // 경련 검사 주기

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float convulsionChance = 0.35f; // 평소 경련 확률

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float convulsionCombatChance = 0.04f; // 전투 경련 확률

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0"))
	float convulsionTime = 2.2f; // 경련 길이

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique")
	bool bHasFrenzy = false; // 광증 보유 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float frenzyHpThreshold = 30.f; // 광증 임계 HP

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0"))
	float frenzySelfDamage = 1.2f; // 광증 자기 피해 속도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float frenzyWindupScale = 0.6f; // 광증 예고 배율

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0"))
	int32 frenzyConvulsionEvery = 0; // 광증 확정 경련 주기

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique")
	TArray<FName> frenzyAttackSet; // 광증 전용 공격

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0"))
	float fixationChaos = 1.5f; // 응시 혼돈 증가량

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0"))
	float fixationBreakRange = 3.f; // 응시 임계 거리

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0"))
	float fixationTimeout = 12.f; // 응시 임계 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0"))
	float lightHoldTime = 10.f; // 기피 임계 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique")
	bool bCanReform = false; // 부활 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0"))
	float reformDelay = 8.f; // 부활 딜레이

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float reformHpRatio = 50.f; // 부활 HP 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Unique", meta = (ClampMin = "0.0"))
	float reformKillDamage = 0.f; // 부활 중 영구 사망 누적 피해량


	/* 감각 및 대기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense")
	EEnemyIdleType defaultIdleType = EEnemyIdleType::EIT_Stand; // 기본 대기 유형

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0.0"))
	float sightRange = 14.f; // 시야 거리 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float sightHalfAngleH = 60.f; // 수평 반시야각

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float sightHalfAngleV = 35.f; // 수직 반시야각

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0.0"))
	float sightGainBase = 150.f; // 정면 3m 초당 게이지 증가량

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0.0"))
	float hearingRange = 16.f; // 청각 반경 상한

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0", ClampMax = "100"))
	int32 hearingCap = 70; // 행동음 게이지 상한

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0", ClampMax = "100"))
	int32 alertThreshold = 25; // 경계 진입 임계값

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0", ClampMax = "100"))
	int32 combatThreshold = 100; // 전투 진입 임계값

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0.0"))
	float detectDecay = 20.f; // 게이지 감소 속도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0.0"))
	float sightForgetTime = 3.f; // 시야를 잃고도 위치를 아는 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sense", meta = (ClampMin = "0.0"))
	float searchDuration = 8.f; // 탐색 시간


	/* 행동 범위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range")
	EEnemyOverride packAttacker = EEnemyOverride::EOVR_Default; // 무리 공격형 종류

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range")
	bool bCanHold = false; // 보류 가능

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0.0"))
	float leashFromHome = 25.f; // 추적 거리 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range")
	EMonsterSociality sociality = EMonsterSociality::ESOC_None; // 사회성

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0.0"))
	float callRadius = 12.f; // 적 호출 반경

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0.0"))
	float callDelay = 1.f; // 적 호출 딜레이

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0"))
	int32 summonMax = 0; // 적 동시 소환 수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Range", meta = (ClampMin = "0"))
	int32 summonTotal = 0; // 총 소환 수


	/* 데이터 검사 */
	virtual void OnPostDataImport(const UDataTable* inDataTable, const FName inRowName, TArray<FString>& outCollectedImportProblems) override;
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& context) const override;
#endif
};