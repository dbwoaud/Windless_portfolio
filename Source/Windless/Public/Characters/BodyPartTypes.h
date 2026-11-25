#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "BodyPartTypes.generated.h"

/* 신체 부위 열거형 */
UENUM(BlueprintType)
enum class EBodyPart : uint8
{
    EBP_None     UMETA(DisplayName = "None"),
    EBP_Head     UMETA(DisplayName = "Head"),     // 머리
    EBP_Chest    UMETA(DisplayName = "Chest"),    // 몸통
    EBP_LeftArm  UMETA(DisplayName = "LeftArm"),  // 왼팔
    EBP_RightArm UMETA(DisplayName = "RightArm"), // 오른팔
    EBP_LeftLeg  UMETA(DisplayName = "LeftLeg"),  // 왼다리
    EBP_RightLeg UMETA(DisplayName = "RightLeg"), // 오른다리
    EBP_MAX      UMETA(Hidden)
};

/* 부상 종류 열거형 */
UENUM(BlueprintType)
enum class EInjuryType : uint8
{
    EIT_None        UMETA(DisplayName = "None"),
    EIT_Concussion  UMETA(DisplayName = "Concussion"),  // 뇌진탕(머리)
    EIT_EyeDamage   UMETA(DisplayName = "EyeDamage"),   // 안구 손상(머리)
    EIT_Breathless  UMETA(DisplayName = "Breathless"),  // 호흡곤란(몸통)
    EIT_ArmFracture UMETA(DisplayName = "ArmFracture"), // 골절(팔)
    EIT_LegFracture UMETA(DisplayName = "LegFracture"), // 골절(다리)
    EIT_Bleeding    UMETA(DisplayName = "Bleeding")     // 과다출혈(부위)
};

/* 부상 종류의 정보를 저장한 구조체 */
USTRUCT(BlueprintType)
struct FInjuryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart")
    EInjuryType type = EInjuryType::EIT_None; // 부상 종류

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart")
    float chance = 0.f; // 부상 발동 확률

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BodyPart")
    bool bActive = false; // 현재 활성 여부
};

/* 부위의 현재 상태 정보를 저장한 구조체 */
USTRUCT(BlueprintType)
struct FBodyPartState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart")
    float maxHealth = 100.f; // 부위 최대 체력

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BodyPart")
    float baseMaxHealth = 0.f; // 장비 보정을 제외한 원본 최대 체력

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BodyPart")
    float dataMaxHealth = 0.f; // 데이터 테이블 원본 최대 체력

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BodyPart")
    float health = 100.f; // 부위 현재 체력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart|Regen")
    float baseHealthRegenRate = 0.f; // 부위 기본 생명력 변동 속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart|Regen")
    float flatHealthRegenAdd = 0.f; // 부위 합연산 재생력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart|Regen")
    float multHealthRegenAdd = 0.f; // 부위 곱연산 재생력

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BodyPart|Regen")
    float finalHealthRegenRate = 0.f; // 부위 최종 생명력 변동 속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart|Injury")
    TArray<FInjuryEntry> injuries; // 부위가 가질 수 있는 부상 목록

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BodyPart|Injury")
    bool bInjured = false; // 현재 부상 상태인지

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BodyPart")
    bool bDisabled = false; // 부위 HP가 0이 되어 사용 불가 상태인지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart")
    float partDamageMultiplier = 1.f; // 최대 체력 비례 대미지 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart")
    float disturbWeight = 0.f; // 감각 교란 추첨 가중치
};

/* 부위의 상태 정보를 저장한 구조체 */
USTRUCT(BlueprintType)
struct FBodyPartData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart")
    float maxHealth = 100.f; // 부위 최대 체력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart")
    float partDamageMultiplier = 1.f; // 최대 체력 비례 대미지 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart")
    float disturbWeight = 0.f; // 감각 교란 추첨 가중치

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart|Regen")
    float baseHealthRegenRate = 0.f; // 부위 기본 생명력 변동 속도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart|Regen")
    float flatHealthRegenAdd = 0.f; // 부위 합연산 재생력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart|Regen")
    float multHealthRegenAdd = 0.f; // 부위 곱연산 재생력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BodyPart|Injury")
    TArray<FInjuryEntry> injuries; // 부위가 가질 수 있는 부상 목록
};

/* 부상으로 발생한 2차 효과를 저장한 구조체 */
USTRUCT(BlueprintType)
struct FInjuryData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float attackRateMultiplier = 1.f; // 일반 공격 속도 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float strongAttackChargingTimeMultiplier = 1.f; // 강공격 차징 시간 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float dodgeDistanceMultiplier = 1.f; // 구르기 이동 거리 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float lockOnRangeMultiplier = 1.f; // 락온 거리 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float staminaRegenMultiplier = 1.f; // 스태미나 회복 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float moveHpCost = 0.f; // 이동/구르기 초당 자해 HP

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float actionDelay = 0.f; // 액션 간 추가 딜레이

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float onHitSelfHpCost = 0.f; // 무기 히트당 자해 HP

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Injury")
    float onActivateChaosLossRatio = 0.f; // 발동 즉시 감소되는 혼돈 비율
};