#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Characters/CombatTypes.h"

/* 경직 결정 */
namespace
{
	EStaggerResult StaggerTest(float impact, float toughness, float windup = -1.f, bool bCanStrong = true, bool bImmune = false)
	{
		FStaggerInput input;
		input.impact = impact;
		input.toughness = toughness;
		input.attackWindup = windup;
		input.bCanStaggerStrong = bCanStrong;
		input.bKnockbackImmune = bImmune;
		return WindlessCombat::ResolveStagger(input);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyHitStaggerTest, "Windless.EnemyAI.Hit.Stagger",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyHitStaggerTest::RunTest(const FString& Parameters)
{
	/* 경직 경계 */
	TestTrue(TEXT("충격량 = 강인도: 버팀"), StaggerTest(10.f, 10.f) == EStaggerResult::ESR_Endure);
	TestTrue(TEXT("충격량 = 강인도 + a: 약한 경직"), StaggerTest(10.01f, 10.f) == EStaggerResult::ESR_Weak);
	TestTrue(TEXT("충격량 = 강인도 * 2 미만: 약한 경직"), StaggerTest(19.99f, 10.f) == EStaggerResult::ESR_Weak);
	TestTrue(TEXT("충격량 = 강인도 * 2 이상: 강한 경직"), StaggerTest(20.f, 10.f) == EStaggerResult::ESR_Strong);
	TestTrue(TEXT("충격량 = 강인도 = 0: 버팀"), StaggerTest(0.f, 0.f) == EStaggerResult::ESR_Endure);

	/* 예고 하한 */
	TestTrue(TEXT("예고 0.45: 경직 없음"), StaggerTest(40.f, 10.f, 0.45f) == EStaggerResult::ESR_ShortWindup);
	TestTrue(TEXT("예고 0.50: 경직 가능"), StaggerTest(40.f, 10.f, 0.50f) == EStaggerResult::ESR_Strong);
	TestTrue(TEXT("광증 예고 0.42: 경직 없음"), StaggerTest(20.f, 15.f, 0.42f) == EStaggerResult::ESR_ShortWindup);
	TestTrue(TEXT("플레이어 공격: 하한 없음"), StaggerTest(20.f, 15.f) == EStaggerResult::ESR_Weak);

	/* 강한 경직 불가 조건 시 약한 경직 대체 */
	TestTrue(TEXT("강한 경직 불가 공격: 약한 경직"), StaggerTest(40.f, 10.f, 0.70f, false) == EStaggerResult::ESR_Weak);
	TestTrue(TEXT("날아감 면역: 약한 경직"), StaggerTest(40.f, 10.f, -1.f, true, true) == EStaggerResult::ESR_Weak);

	/* 실제 데이터 */
	TestTrue(TEXT("늘어진 팔 내리치기 20(약한 경직)"), StaggerTest(20.f, 15.f, 0.70f, false) == EStaggerResult::ESR_Weak);
	TestTrue(TEXT("거구 내려치기 40(강한 경직)"), StaggerTest(40.f, 15.f, 1.65f, true) == EStaggerResult::ESR_Strong);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS