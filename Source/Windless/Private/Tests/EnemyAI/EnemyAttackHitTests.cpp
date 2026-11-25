#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/AI/EnemyAttackHit.h"
#include "Enemy/AI/EnemyAttackRow.h"

/* 적 데미지 계산 정보 */

namespace
{
	FEnemyAttackRow HitTestAttack(EEnemyAttackClass attackClass, float damageMultiplier, float impact, bool bCanStaggerStrong)
	{
		FEnemyAttackRow attack;
		attack.attackClass = attackClass;
		attack.damageMultiplier = damageMultiplier;
		attack.impact = impact;
		attack.bCanStaggerStrong = bCanStaggerStrong;
		attack.hitRanges = { 2.5f };
		attack.windups = { 0.70f };
		attack.activeTimes = { 0.12f };
		return attack;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAttackHitDataTest, "Windless.EnemyAI.Attack.HitData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAttackHitDataTest::RunTest(const FString& Parameters)
{
	/* 늘어진 팔 내리치기 */
	const FWeaponHitOverride smash = EnemyAttackHit::BuildHitOverride(
		HitTestAttack(EEnemyAttackClass::EATC_Heavy, 1.f, 20.f, false), FName(TEXT("DA_Smash")), 20.f, 0.70f);
	TestEqual(TEXT("내리치기 대미지 20"), smash.damage, 20.f);
	TestEqual(TEXT("내리치기 충격량 20"), smash.impact, 20.f);
	TestTrue(TEXT("내리치기 = 강공격"), smash.attackType == EAttackType::EAT_Heavy);
	TestFalse(TEXT("내리치기 강한 경직 불가"), smash.bCanStaggerStrong);
	TestEqual(TEXT("예고 길이 0.70 전달"), smash.windup, 0.70f);
	TestEqual(TEXT("공격 ID 전달"), smash.attackId, FName(TEXT("DA_Smash")));

	/* 거구 돌진 */
	const FWeaponHitOverride rush = EnemyAttackHit::BuildHitOverride(
		HitTestAttack(EEnemyAttackClass::EATC_Heavy, 1.3f, 40.f, true), FName(TEXT("CG_Rush")), 30.f, 1.70f);
	TestEqual(TEXT("돌진 대미지 39"), rush.damage, 39.f, 0.001f);
	TestTrue(TEXT("돌진 강한 경직 가능"), rush.bCanStaggerStrong);

	/* 거구 후려치기 */
	const FWeaponHitOverride swipe = EnemyAttackHit::BuildHitOverride(
		HitTestAttack(EEnemyAttackClass::EATC_Light, 0.4f, 20.f, false), FName(TEXT("CG_Swipe")), 30.f, 0.60f);
	TestEqual(TEXT("후려치기 대미지 12"), swipe.damage, 12.f, 0.001f);
	TestTrue(TEXT("후려치기 = 약공격"), swipe.attackType == EAttackType::EAT_Light);

	/* 광증 예고 배율 반영 예고 길이 계산 */
	const FWeaponHitOverride frenzy = EnemyAttackHit::BuildHitOverride(
		HitTestAttack(EEnemyAttackClass::EATC_Heavy, 1.f, 20.f, false), FName(TEXT("DA_Smash")), 20.f, 0.42f);
	TestEqual(TEXT("광증 예고 0.42 전달"), frenzy.windup, 0.42f);

	/* 음수 대미지 없음 */
	const FWeaponHitOverride zero = EnemyAttackHit::BuildHitOverride(
		HitTestAttack(EEnemyAttackClass::EATC_Light, 1.f, 5.f, false), NAME_None, 0.f, 0.50f);
	TestEqual(TEXT("공격력 0 → 대미지 0"), zero.damage, 0.f);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS