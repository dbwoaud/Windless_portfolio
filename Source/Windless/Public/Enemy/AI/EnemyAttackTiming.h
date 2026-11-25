#pragma once

#include "CoreMinimal.h"
#include "Enemy/AI/EnemyAITypes.h"

struct FEnemyAttackRow;

/* 공격 구간 1개 */
struct FEnemyAttackPhaseSpan
{
	EEnemyAttackPhase phase = EEnemyAttackPhase::EAP_None;
	float startTime = 0.f; // 시작 시각
	float duration = 0.f;  // 길이

	float GetEndTime() const { return startTime + duration; }
};

/* 공격 시간 계산 */
namespace EnemyAttackTiming
{
	/* 공격 중 회전 */
	constexpr float TrackLockLeadTime = 0.15f;      // 판정 전 방향 고정 시간
	constexpr float RecoveryTurnRestoreTime = 1.f;  // 후딜 회전 복귀 시간 

	TArray<FEnemyAttackPhaseSpan> BuildTimeline(const FEnemyAttackRow& attack, int32 hitIndex = 0, float windupScale = 1.f); // 예고 -> 판정 -> 후딜을 설정하는 함수
	float GetTotalDuration(const TArray<FEnemyAttackPhaseSpan>& timeline); // 공격 전체 시간을 반환하는 함수
	FFloatInterval GetRollWindow(const FEnemyAttackRow& attack, float invincibleTime, int32 hitIndex = 0); // 구르기 시작 성공 구간을 반환하는 함수

	float GetTrackScale(EEnemyTrackProfile profile, float windup, float timeInWindup); // 예고 중 추적 계수를 반환하는 함수
	float GetRecoveryTurnScale(float timeInRecovery); // 후딜 회전 복귀 시간을 반환하는 함수
	float GetTurnScale(const FEnemyAttackRow& attack, const TArray<FEnemyAttackPhaseSpan>& timeline, float elapsed, int32 hitIndex = 0); // 예고 시작 후 특정 시점의 회전 계수를 반환하는 함수

	FName GetPhaseSectionName(EEnemyAttackPhase phase, FName prefix); // 몽타주 섹션 이름을 반환하는 함수
	const TCHAR* GetPhaseName(EEnemyAttackPhase phase); // 공격 단계 이름을 반환하는 함수
	const TCHAR* GetTrackProfileName(EEnemyTrackProfile profile); // 추적 방식 이름을 반환하는 함수
}