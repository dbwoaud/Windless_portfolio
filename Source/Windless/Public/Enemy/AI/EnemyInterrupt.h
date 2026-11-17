#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatTypes.h"
#include "Enemy/AI/EnemyAITypes.h"

/* 경직 중 끼어들기 반응 */
enum class EInterruptHitAction : uint8
{
	EIHA_None,     // 변화 없음
	EIHA_Begin,    // 경직 시작
	EIHA_Restart,  // 약한 경직 중 약한 결정
	EIHA_Promote,  // 약한 경직 중 강한 결정
	EIHA_Relaunch, // 강한 경직 중 강한 결정
	EIHA_HpOnly    // 강한 경직 중 약한 결정
};

enum class EStrongStaggerPhase : uint8
{
	ESSP_Launch, // 날아감
	ESSP_Down,   // 다운
	ESSP_GetUp,  // 기상
	ESSP_Done    // 끝
};

/* 강한 경직 구간 시간 */
struct FStrongStaggerTimes
{
	float launch = 0.5f;
	float down = 1.0f;
	float getUp = 0.8f;

	float GetTotal() const { return launch + down + getUp; }
	float GetPhaseTime(EStrongStaggerPhase phase) const
	{
		switch (phase)
		{
		case EStrongStaggerPhase::ESSP_Launch: return launch;
		case EStrongStaggerPhase::ESSP_Down:   return down;
		case EStrongStaggerPhase::ESSP_GetUp:  return getUp;
		default:                               return 0.f;
		}
	}
};

/* 끼어들기 계산 */
namespace EnemyInterrupt
{
	EEnemyInterrupt ToInterrupt(EStaggerResult result); // 경직 결정 결과를 끼어들기 종류로 바꾸는 함수
	EInterruptHitAction ResolveRehit(EEnemyInterrupt current, EStaggerResult result); // 현재 끼어들기 중 다시 맞았을 때 할 일을 반환하는 함수
	EStrongStaggerPhase GetStrongPhase(const FStrongStaggerTimes& times, float elapsed); // 강한 경직 시작 후 특정 시점의 구간을 반환하는 함수
	FVector ComputeLaunchVelocity(const FVector& direction, float distance, float time, float gravityZ); // 시간 안에 거리만큼 날아가 착지하는 발사 속도를 반환하는 함수
	FName GetStrongSectionName(EStrongStaggerPhase phase); // 강한 경직 몽타주 섹션 이름을 반환하는 함수
	const TCHAR* GetInterruptName(EEnemyInterrupt interrupt); // 끼어들기 이름을 반환하는 함수
	const TCHAR* GetHitActionName(EInterruptHitAction action); // 재피격 처리 이름을 반환하는 함수
	const TCHAR* GetStrongPhaseName(EStrongStaggerPhase phase); // 강한 경직 구간 이름을 반환하는 함수
}