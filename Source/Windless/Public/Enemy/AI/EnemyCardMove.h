#pragma once

#include "CoreMinimal.h"

/* 비공격 카드 이동 계산 */
namespace EnemyCardMove
{
	WINDLESS_API FVector GetCirclePoint(const FVector& center, const FVector& from, float radius, float stepDistance, float direction); // 원 위의 점을 반환하는 함수 
	WINDLESS_API FVector GetBackOffPoint(const FVector& from, const FVector& target, float distance); // 물러나기 지점을 반환하는 함수
	WINDLESS_API float GetApproachSpeed(float distance, float runDistance, float walkSpeed, float runSpeed); // 접근 속도를 반환하는 함수
}