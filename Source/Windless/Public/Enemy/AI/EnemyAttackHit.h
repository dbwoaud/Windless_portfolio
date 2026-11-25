#pragma once

#include "CoreMinimal.h"
#include "Characters/CombatTypes.h"
#include "Enemy/AI/EnemyAITypes.h"

struct FEnemyAttackRow;

/* 몬스터 공격 판정값 계산 */
namespace EnemyAttackHit
{
	FWeaponHitOverride BuildHitOverride(const FEnemyAttackRow& attack, FName attackId, float attackPower, float actualWindup); // 공격 대미지를 반환하는 함수
	EAttackType ToAttackType(EEnemyAttackClass attackClass); // 공격 클래스로 공격 종류를 반환하는 함수
}