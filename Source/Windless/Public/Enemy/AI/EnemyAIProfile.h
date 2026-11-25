#pragma once

#include "CoreMinimal.h"
#include "Enemy/AI/EnemyAITypes.h"

struct FMonsterArchetypeRow;

/* 값이 정해진 겹 */
enum class EProfileSource : uint8
{
	Grade,       // 등급
	Appearance,  // 외형
	Temperament, // 성향
	Archetype,   // 원형
	Placement,   // 배치
	Common       // 기본값
};

/* 확정 값 및 출처 */
template<typename T>
struct TProfileValue
{
	T value{};
	EProfileSource source = EProfileSource::Common;

	void Set(T inValue, EProfileSource inSource) { value = inValue; source = inSource; }
};

/* 5겹 확정 결과 */
struct WINDLESS_API FMonsterProfile
{
	const FMonsterArchetypeRow* archetype = nullptr; // 원형 데이터


	/* 등급 */
	TProfileValue<bool> bPermanentDeath;  // 리셋 시 부활 안 함
	TProfileValue<bool> bFrenzyAttackSet; // 광증 방식

	/* 외형, 성향, 원형 */
	TProfileValue<bool> bPackAttacker;               // 무리 공격 여부
	TProfileValue<EEnemyTemperament> temperament;    // 확정 성향
	TProfileValue<bool> bCanDodgeRoll;               // 반응 회피 가능 여부
	TProfileValue<EEnemyReactionType> reaction;      // 휘두르기 에 대한 반응

	TProfileValue<float> nearRange; // 가까이 상한 거리
	TProfileValue<float> midRange;  // 중간 상한 거리
	TProfileValue<float> farRange;  // 멀리 상한 거리

	TProfileValue<float> leash;    // 추적 거리
	TProfileValue<float> maxLeash; // 최대 추적 거리

	TProfileValue<int32> frenzyConvulsionEvery; // 광증 확정 경련 주기


	/* 배치 */
	TProfileValue<EEnemyIdleType> idleType; // 대기 유형

	bool IsValid() const { return archetype != nullptr; }
};

/* 5겹 확정 계산 */
namespace EnemyAIProfile
{
	constexpr float MaxLeashExtra = 15.f; // 최대 추적 거리

	FMonsterProfile Resolve(const FMonsterArchetypeRow& archetype, EEnemyIdleType placementIdleType); // 원형과 배치를 확정하는 함수
	TArray<FString> DescribeLines(const FMonsterProfile& profile); // 최종 값과 출처를 출력하는 함수

	const TCHAR* GetSourceName(EProfileSource source); // 등급부터 배치까지 출처의 이름을 반환하는 함수
	bool GetAppearancePackAttacker(EMonsterAppearance appearance); // 외형에 따른 무리 공격 여부를 반환하는 함수
	bool GetAppearanceDodgeRoll(EMonsterAppearance appearance); // 외형에 따른 반응 회부 여부를 반환하는 함수
}