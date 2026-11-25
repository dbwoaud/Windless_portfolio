#pragma once

#include "CoreMinimal.h"

/* 도달 판정 결과 */
enum class EEnemyReach : uint8
{
	EER_Reachable, // 도달 가능
	EER_NoPath,    // 경로 없음
	EER_Partial,   // 일부 경로만 있음
	EER_TooLong    // 경로 초과
};

/* 도달 불가 판정 */
namespace EnemyReach
{
	WINDLESS_API bool ShouldCheckPath(float distance, float heightDelta, float preferredRange, float heightLimit); // 경로 확인 여부를 확인하는 함수
	WINDLESS_API EEnemyReach Judge(bool bPathFound, bool bPartial, float pathLength, float straightDistance, float maxRatio); // 경로 결과로 도달 여부를 판단하는 함수
	WINDLESS_API const TCHAR* GetReachName(EEnemyReach reach); // 도달 이름을 반환하는 함수
}