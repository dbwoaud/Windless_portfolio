#pragma once

#include "CoreMinimal.h"
#include "EnemyAITypes.generated.h"

/* 행동 카드 */
UENUM(BlueprintType)
enum class EEnemyCard : uint8
{
	EC_LightAttack UMETA(DisplayName = "LightAttack"), // 약공격
	EC_HeavyAttack UMETA(DisplayName = "HeavyAttack"), // 강공격
	EC_Rush        UMETA(DisplayName = "Rush"),        // 돌진
	EC_Shoot       UMETA(DisplayName = "Shoot"),       // 사격
	EC_MeleeAttack UMETA(DisplayName = "MeleeAttack"), // 원거리형 근접 공격
	EC_SideStep    UMETA(DisplayName = "SideStep"),    // 옆걸음
	EC_BackOff     UMETA(DisplayName = "BackOff"),     // 뒤로 빠지기
	EC_Approach    UMETA(DisplayName = "Approach"),    // 걸어서 다가가기
	EC_Rest        UMETA(DisplayName = "Rest"),        // 숨 고르기
	EC_Special     UMETA(DisplayName = "Special")      // 원형 특수 카드/광증 전용 공격
};

/* 약공격 / 강공격 */
UENUM(BlueprintType)
enum class EEnemyAttackClass : uint8
{
	EATC_Light UMETA(DisplayName = "Light"), // 약공격 (엇박 없음, 최대 3타)
	EATC_Heavy UMETA(DisplayName = "Heavy")  // 강공격(엇박 있음, 1타)
};

/* 콤보 종류 */
UENUM(BlueprintType)
enum class EEnemyComboType : uint8
{
	ECT_Branch UMETA(DisplayName = "Branch"), // 갈림(사거리 안 90%, 밖 25%)
	ECT_Full   UMETA(DisplayName = "Full")    // 끝까지(엘리트 이상)
};

/* 추적 방식 */
UENUM(BlueprintType)
enum class EEnemyTrackProfile : uint8
{
	ETP_Strong UMETA(DisplayName = "Strong"), // 끝까지 추적
	ETP_Decay  UMETA(DisplayName = "Decay"),  // 점점 덜 추적
	ETP_None   UMETA(DisplayName = "None")    // 추적 없음
};


/* 등급 겹 */
UENUM(BlueprintType)
enum class EMonsterGrade : uint8
{
	EGRD_Normal    UMETA(DisplayName = "Normal"),    // 일반
	EGRD_Elite     UMETA(DisplayName = "Elite"),     // 엘리트
	EGRD_FieldBoss UMETA(DisplayName = "FieldBoss"), // 필드보스
	EGRD_MainBoss  UMETA(DisplayName = "MainBoss")   // 메인보스
};

/* 외형 겹 */
UENUM(BlueprintType)
enum class EMonsterAppearance : uint8
{
	EAPP_Humanoid UMETA(DisplayName = "Humanoid (HM)"), // 인간형
	EAPP_Beast    UMETA(DisplayName = "Beast (BS)"),    // 짐승형
	EAPP_Avian    UMETA(DisplayName = "Avian (AV)"),    // 조류형
	EAPP_Reptile  UMETA(DisplayName = "Reptile (RP)"),  // 파충형
	EAPP_Skeleton UMETA(DisplayName = "Skeleton (SK)"), // 해골형
	EAPP_Aberrant UMETA(DisplayName = "Aberrant (AB)")  // 기형형
};

/* 성향 겹 */
UENUM(BlueprintType)
enum class EEnemyTemperament : uint8
{
	ETMP_Default    UMETA(DisplayName = "Default"),    // 외형 기본값
	ETMP_Calm       UMETA(DisplayName = "Calm"),       // 차분(45%)
	ETMP_Normal     UMETA(DisplayName = "Normal"),     // 기본(55%)
	ETMP_Aggressive UMETA(DisplayName = "Aggressive")  // 공격적(65%)
};

/* 원형 겹 */
UENUM(BlueprintType)
enum class EEnemyOverride : uint8
{
	EOVR_Default UMETA(DisplayName = "Default"), // 외형 기본값
	EOVR_On      UMETA(DisplayName = "On"),      // 켬
	EOVR_Off     UMETA(DisplayName = "Off")      // 끔
};

/* ID용 공격 거리 분류 */
UENUM(BlueprintType)
enum class EMonsterRangeClass : uint8
{
	ERC_Melee   UMETA(DisplayName = "Melee"),   // 근거리형
	ERC_Ranged  UMETA(DisplayName = "Ranged"),  // 원거리형
	ERC_Special UMETA(DisplayName = "Special")  // 특수형
};

/* 공격 인지 */
UENUM(BlueprintType)
enum class EMonsterAggression : uint8
{
	EAGG_Aggressive UMETA(DisplayName = "Aggressive"), // 선공형
	EAGG_Passive    UMETA(DisplayName = "Passive")     // 비선공형
};

/* 이동 유형 */
UENUM(BlueprintType)
enum class EMonsterMoveClass : uint8
{
	EMC_Mobile UMETA(DisplayName = "Mobile"), // 이동형
	EMC_Fixed  UMETA(DisplayName = "Fixed")   // 고정형
};

/* 사회성 */
UENUM(BlueprintType)
enum class EMonsterSociality : uint8
{
	ESOC_None  UMETA(DisplayName = "None"),  // 없음
	ESOC_Call  UMETA(DisplayName = "Call"),  // 호출형
	ESOC_Group UMETA(DisplayName = "Group")  // 그룹형
};

/* 전투 역할 */
UENUM(BlueprintType)
enum class EEnemyCombatRole : uint8
{
	ECR_Melee  UMETA(DisplayName = "Melee"),  // 근거리 카드 7장
	ECR_Ranged UMETA(DisplayName = "Ranged")  // 원거리 카드 6장
};

/* 가드 정책 */
UENUM(BlueprintType)
enum class EEnemyGuardPolicy : uint8
{
	EGP_None     UMETA(DisplayName = "None"),     // 가드 없음
	EGP_Always   UMETA(DisplayName = "Always"),   // 상시 가드
	EGP_Reactive UMETA(DisplayName = "Reactive")  // 반응 가드
};

/* 대기 유형 */
UENUM(BlueprintType)
enum class EEnemyIdleType : uint8
{
	EIT_None         UMETA(DisplayName = "None (Archetype Default)"), // 원형 기본값 
	EIT_Stand        UMETA(DisplayName = "Stand"),        // 대기
	EIT_Patrol       UMETA(DisplayName = "Patrol"),       // 순찰
	EIT_Wander       UMETA(DisplayName = "Wander"),       // 배회
	EIT_Circle       UMETA(DisplayName = "Circle"),       // 맴돌기
	EIT_ScratchWall  UMETA(DisplayName = "ScratchWall"),  // 벽 긁기
	EIT_Tremble      UMETA(DisplayName = "Tremble"),      // 웅크려 떪
	EIT_PeerCrack    UMETA(DisplayName = "PeerCrack"),    // 균열 들여다보기
	EIT_Bow          UMETA(DisplayName = "Bow"),          // 헛절
	EIT_Ambush       UMETA(DisplayName = "Ambush"),       // 매복
	EIT_PlayDead     UMETA(DisplayName = "PlayDead")      // 시체 흉내
};

/* 거리 구간 */
UENUM(BlueprintType)
enum class EEnemyRangeBand : uint8
{
	ERB_Near       UMETA(DisplayName = "Near"),       // 가까이
	ERB_Mid        UMETA(DisplayName = "Mid"),        // 중간
	ERB_Far        UMETA(DisplayName = "Far"),        // 멀리
	ERB_OutOfRange UMETA(DisplayName = "OutOfRange")  // 사거리 밖
};

/* 카드 묶음 */
UENUM(BlueprintType)
enum class EEnemyCardBundle : uint8
{
	ECB_Attack UMETA(DisplayName = "Attack"), // 공격 묶음
	ECB_Probe  UMETA(DisplayName = "Probe")   // 간 보기 묶음
};

/* 특수 카드 위치 조건 */
UENUM(BlueprintType)
enum class EEnemySpecialCondition : uint8
{
	ESC_None           UMETA(DisplayName = "None"),           // 조건 없음
	ESC_Flank          UMETA(DisplayName = "Flank"),          // 측후방
	ESC_UnderBody      UMETA(DisplayName = "UnderBody"),      // 몸통 아래에 조건 시간 이상
	ESC_InsideMinRange UMETA(DisplayName = "InsideMinRange")  // 다른 공격의 최소 사거리 안
};

/* 카드 점수 덮어쓰기 1칸 */
USTRUCT(BlueprintType)
struct WINDLESS_API FEnemyCardScoreOverride
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyCard card = EEnemyCard::EC_Rest; // 덮어쓸 카드

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyRangeBand band = EEnemyRangeBand::ERB_Near; // 덮어쓸 거리 구간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card", meta = (ClampMin = "0.0"))
	float score = 0.f; // 기본 점수
};

/* 원형 특수 카드 1장 */
USTRUCT(BlueprintType)
struct WINDLESS_API FEnemySpecialCard
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	FName attackId = NAME_None; // 공격 ID(DT_EnemyAttack 행 이름)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemySpecialCondition condition = EEnemySpecialCondition::ESC_None; // 위치 조건

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card", meta = (ClampMin = "0.0"))
	float conditionTime = 0.f; // 조건 유지 시간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card", meta = (ClampMin = "0.0"))
	float scoreNear = 0.f; // 가까이 점수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card", meta = (ClampMin = "0.0"))
	float scoreMid = 0.f; // 중간 점수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card", meta = (ClampMin = "0.0"))
	float scoreFar = 0.f; // 멀리 점수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyCardBundle bundle = EEnemyCardBundle::ECB_Attack; // 카드 묶음
};

/* 추가 피해 종류 */
UENUM(BlueprintType)
enum class EEnemyAddedDamage : uint8
{
	EAD_None   UMETA(DisplayName = "None"),   // 없음
	EAD_Fire   UMETA(DisplayName = "Fire"),   // 화염
	EAD_Poison UMETA(DisplayName = "Poison")  // 독
};

/* 반응 방식 */
UENUM(BlueprintType)
enum class EEnemyReactionType : uint8
{
	ERT_Counter UMETA(DisplayName = "Counter"), // 카운터
	ERT_Guard   UMETA(DisplayName = "Guard"),   // 가드
	ERT_Dodge   UMETA(DisplayName = "Dodge")    // 회피
};

/* 공격 구간 */
UENUM(BlueprintType)
enum class EEnemyAttackPhase : uint8
{
	EAP_None     UMETA(DisplayName = "None"),
	EAP_PreTurn  UMETA(DisplayName = "PreTurn"),  // 선회
	EAP_Windup   UMETA(DisplayName = "Windup"),   // 예고
	EAP_Hold     UMETA(DisplayName = "Hold"),     // 엇박 멈춤
	EAP_Release  UMETA(DisplayName = "Release"),  // 엇박 멈춤 뒤 내려옴
	EAP_Active   UMETA(DisplayName = "Active"),   // 판정
	EAP_Recovery UMETA(DisplayName = "Recovery")  // 후딜
};

/* 공격 종료 원인 */
UENUM()
enum class EEnemyAttackEnd : uint8
{
	EAE_Completed  UMETA(DisplayName = "Completed"),  // 후딜까지 끝남
	EAE_Cancelled  UMETA(DisplayName = "Cancelled"),  // 피격 경직, 사망으로 취소
	EAE_TurnFailed UMETA(DisplayName = "TurnFailed")  // 선회 실패
};

/* 끼어들기 종류 */
UENUM()
enum class EEnemyInterrupt : uint8
{
	EIN_None          UMETA(DisplayName = "None"),
	EIN_StaggerWeak   UMETA(DisplayName = "StaggerWeak"),  // 약한 경직
	EIN_StaggerStrong UMETA(DisplayName = "StaggerStrong") // 강한 경직
};