#include "Enemy/AI/EnemyReach.h"

bool EnemyReach::ShouldCheckPath(float distance, float heightDelta, float preferredRange, float heightLimit)
{
	return distance > preferredRange || FMath::Abs(heightDelta) > heightLimit;
}

EEnemyReach EnemyReach::Judge(bool bPathFound, bool bPartial, float pathLength, float straightDistance, float maxRatio)
{
	if (!bPathFound)
		return EEnemyReach::EER_NoPath;

	if (bPartial)
		return EEnemyReach::EER_Partial;

	if (straightDistance > KINDA_SMALL_NUMBER && pathLength > straightDistance * maxRatio)
		return EEnemyReach::EER_TooLong;

	return EEnemyReach::EER_Reachable;
}

const TCHAR* EnemyReach::GetReachName(EEnemyReach reach)
{
	switch (reach)
	{
	case EEnemyReach::EER_NoPath:  return TEXT("경로 없음");
	case EEnemyReach::EER_Partial: return TEXT("일부 경로");
	case EEnemyReach::EER_TooLong: return TEXT("너무 돌아감");
	default:                       return TEXT("도달 가능");
	}
}