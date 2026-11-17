#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

class AController;
class USkeletalMeshComponent;

/* 공격 종류 */
UENUM(BlueprintType)
enum class EAttackType : uint8
{
	EAT_Light UMETA(DisplayName = "Light"), // 약공격
	EAT_Heavy UMETA(DisplayName = "Heavy"), // 강공격
	EAT_Jump  UMETA(DisplayName = "Jump"),  // 점프 공격
	EAT_Ranged UMETA(DisplayName = "Ranged")  // 원거리 공격
};

/* 무기 공격 1회의 설정값 */
USTRUCT(BlueprintType)
struct FAttackSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float staminaCost = 10.f; // 공격 1회 소모 스태미나

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (ClampMin = "0.0"))
	float impact = 10.f; // 충격량

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bUnblockable = false; // 가드 불가 공격 여부
};

/* 무기 타격 1회의 판정 정보 */
struct FCombatHitInfo
{
	AActor* attacker = nullptr;                   // 공격한 캐릭터
	AActor* causer = nullptr;                     // 공격 무기
	AController* instigatorController = nullptr;  // 공격자 컨트롤러
	USkeletalMeshComponent* hitMesh = nullptr;    // 피격된 메시
	FName hitBoneName = NAME_None;                // 피격된 본 이름
	FVector impactPoint = FVector::ZeroVector;    // 타격 지점
	float damage = 0.f;                           // 최종 대미지
	float impact = 0.f;                           // 충격량
	float injuryChance = -1.f;                    // 기본 부상 확률
	EAttackType attackType = EAttackType::EAT_Light; // 공격 종류
	bool bUnblockable = false;						 // 가드 불가 공격 여부
	FName attackId = NAME_None;                      // 몬스터 공격 데이터 행 이름
	float windup = -1.f;                             // 몬스터 공격의 예고 길이
	bool bCanStaggerStrong = true;                   // 강한 경직 허용 여부
};

/* 무기 판정값 */
struct FWeaponHitOverride
{
	FName attackId = NAME_None;                      // 공격 데이터 행 이름
	EAttackType attackType = EAttackType::EAT_Light; // 공격 종류(강/약)
	float damage = 0.f;                              // 최종 대미지
	float impact = 0.f;                              // 충격량
	float windup = -1.f;                             // 실제 예고 길이
	bool bCanStaggerStrong = false;                  // 강한 경직 허용 여부
};

/* 경직 결정 결과 */
enum class EStaggerResult : uint8
{
	ESR_Endure,      // 버팀
	ESR_ShortWindup, // 경직 없음
	ESR_Weak,        // 약한 경직
	ESR_Strong       // 강한 경직
};

/* 경직 결정 입력 */
struct FStaggerInput
{
	float impact = 0.f;            // 충격량
	float toughness = 0.f;         // 맞는 쪽 강인도
	float attackWindup = -1.f;     // 몬스터 공격의 실제 예고 길이
	bool bCanStaggerStrong = true; // 공격의 강한 경직 허용 여부
	bool bKnockbackImmune = false; // 맞는 쪽 날아감 면역 여부
};


/* 경직 상태 */
enum class EStaggerState : uint8
{
	ESS_None,   // 경직 아님
	ESS_Weak,   // 약한 경직
	ESS_Strong  // 강한 경직
};

/* 경직 상태 정보 */
struct FStaggerStatus
{
	EStaggerState state = EStaggerState::ESS_None;
	bool bInvincible = false;

	bool IsStaggered() const { return state != EStaggerState::ESS_None; }
};

/* 가드 처리 결과 */
struct FGuardResult
{
	bool bBlocked = false;        // 가드 성공 여부
	bool bGuardBroken = false;    // 가드 브레이크 발생 여부
	float staminaCost = 0.f;      // 소모된 스태미나
	float residualDamage = 0.f;   // 잔여 대미지
};

/* 화살 1발의 발사 정보 */
struct FArrowShotParams
{
	float damage = 0.f;        // 최종 대미지
	float impact = 0.f;        // 충격량
	float injuryChance = -1.f; // 기본 부상 확률
	bool bUnblockable = false; // 가드 불가 여부
	float speed = 3000.f;      // 발사 속도
	float gravityScale = 0.5f; // 중력 배율
	float chargeRatio = 0.f;   // 차징 비율(0~1)
};

/* 적 약점 부위 설정 */
USTRUCT(BlueprintType)
struct FWeakPointRegion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeakPoint")
	FName regionName = NAME_None; // 약점 부위 이름

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeakPoint")
	FText displayName; // UI 표시 이름

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeakPoint")
	TArray<FName> boneNames; // 약점 부위에 포함되는 본 목록

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeakPoint")
	bool bIncludeChildBones = true; // 하위 본까지 같은 부위로 판정할지 여부
};

/* 전투 계산식 */
namespace WindlessCombat
{
	static constexpr float HeavyDamageMultiplier = 1.5f; // 강공격 대미지 배율
	static constexpr float JumpDamageMultiplier = 1.3f;  // 점프 공격 대미지 배율
	static constexpr float WeakPointCriticalMultiplier = 1.5f; // 약점 크리티컬 배율
	static constexpr float StaggerMinWindup = 0.50f; // 최소 경직 예고 값
	static constexpr float StrongStaggerMultiplier = 2.f; // 강한 경직 배수

	FORCEINLINE float GetAttackTypeMultiplier(EAttackType attackType) // 강공격 대미지 배율을 반환하는 함수
	{
		switch (attackType)
		{
			case EAttackType::EAT_Heavy: return HeavyDamageMultiplier;
			case EAttackType::EAT_Jump:  return JumpDamageMultiplier;
			default:                     return 1.f;
		}
	}

	FORCEINLINE const TCHAR* GetAttackTypeName(EAttackType attackType) // 공격 타입 이름을 반환하는 함수
	{
		switch (attackType)
		{
			case EAttackType::EAT_Heavy: return TEXT("강");
			case EAttackType::EAT_Jump:  return TEXT("점프");
			case EAttackType::EAT_Ranged: return TEXT("원거리");
			default:                     return TEXT("약");
		}
	}

	FORCEINLINE float CalcHitDamage(float attackPower, EAttackType attackType) // 최종 피격 대미지를 계산하는 함수
	{
		return FMath::Max(0.f, attackPower * GetAttackTypeMultiplier(attackType));
	}

	FORCEINLINE float CalcGuardStaminaCost(float incomingDamage, float guardValue) // 가드 스태미나를 계산하는 함수
	{
		return FMath::Max(0.f, incomingDamage - guardValue);
	}

	FORCEINLINE bool ShouldStagger(float impact, float tenacity) // 경직 여부를 확인하는 함수
	{
		return impact > tenacity;
	}

	FORCEINLINE EStaggerResult ResolveStagger(const FStaggerInput& input) // 경직 결과를 결정하는 함수
	{
		if (input.attackWindup >= 0.f && input.attackWindup < StaggerMinWindup)
			return EStaggerResult::ESR_ShortWindup;

		if (input.impact <= input.toughness)
			return EStaggerResult::ESR_Endure;

		if (input.impact >= input.toughness * StrongStaggerMultiplier)
			return (input.bCanStaggerStrong && !input.bKnockbackImmune) ? EStaggerResult::ESR_Strong : EStaggerResult::ESR_Weak;

		return EStaggerResult::ESR_Weak;
	}

	FORCEINLINE const TCHAR* GetStaggerResultName(EStaggerResult result) // 경직 결과 이름을 반환하는 함수
	{
		switch (result)
		{
		case EStaggerResult::ESR_ShortWindup: return TEXT("경직 없음");
		case EStaggerResult::ESR_Weak:        return TEXT("약한 경직");
		case EStaggerResult::ESR_Strong:      return TEXT("강한 경직");
		default:                              return TEXT("버팀");
		}
	}

	FORCEINLINE float CalcChargedDamage(float attackPower, float chargeRatio, float minMultiplier, float maxMultiplier) // 차징 대미지를 계산하는 함수
	{
		const float multiplier = FMath::Lerp(minMultiplier, maxMultiplier, FMath::Clamp(chargeRatio, 0.f, 1.f));
		return FMath::Max(0.f, attackPower * multiplier);
	}

	FORCEINLINE float CalcWeakPointMultiplier(float awakeningBonus) // 약점 크리티컬 최종 배율을 계산하는 함수
	{
		return WeakPointCriticalMultiplier * FMath::Max(1.f, awakeningBonus);
	}
}