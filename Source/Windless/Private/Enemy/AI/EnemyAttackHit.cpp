#include "Enemy/AI/EnemyAttackHit.h"
#include "Enemy/AI/EnemyAttackRow.h"

EAttackType EnemyAttackHit::ToAttackType(EEnemyAttackClass attackClass)
{
	return attackClass == EEnemyAttackClass::EATC_Heavy ? EAttackType::EAT_Heavy : EAttackType::EAT_Light;
}

FWeaponHitOverride EnemyAttackHit::BuildHitOverride(const FEnemyAttackRow& attack, FName attackId, float attackPower, float actualWindup)
{
	FWeaponHitOverride hit;
	hit.attackId = attackId;
	hit.attackType = ToAttackType(attack.attackClass);
	hit.damage = FMath::Max(0.f, attackPower * attack.damageMultiplier);
	hit.impact = attack.impact;
	hit.windup = actualWindup;
	hit.bCanStaggerStrong = attack.bCanStaggerStrong;
	return hit;
}