#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/AI/EnemyAttackTiming.h"
#include "Enemy/AI/EnemyAttackRow.h"

/* 적 AI 2장 공격 구간 테스트 */

namespace
{
	FEnemyAttackRow TimingTestSmash() // 늘어진 팔 내리치기
	{
		FEnemyAttackRow attack;
		attack.attackClass = EEnemyAttackClass::EATC_Heavy;
		attack.hitRanges = { 2.5f };
		attack.windups = { 0.70f };
		attack.activeTimes = { 0.12f };
		attack.recovery = 1.6f;
		return attack;
	}

	FEnemyAttackRow TimingTestSlam() // 무너진 거구 내려치기
	{
		FEnemyAttackRow attack;
		attack.attackClass = EEnemyAttackClass::EATC_Heavy;
		attack.hitRanges = { 3.f };
		attack.windups = { 1.65f };
		attack.windupHoldTime = 0.55f;
		attack.holdReleaseTime = 0.45f;
		attack.activeTimes = { 0.14f };
		attack.recovery = 2.0f;
		return attack;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAttackTimelineTest, "Windless.EnemyAI.Attack.Timeline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAttackTimelineTest::RunTest(const FString& Parameters)
{
	/* 정박(예고 -> 판정 -> 후딜) */
	const TArray<FEnemyAttackPhaseSpan> smash = EnemyAttackTiming::BuildTimeline(TimingTestSmash());
	if (TestEqual(TEXT("내리치기 구간 3개"), smash.Num(), 3))
	{
		TestTrue(TEXT("1구간 예고"), smash[0].phase == EEnemyAttackPhase::EAP_Windup);
		TestEqual(TEXT("예고 0.70"), smash[0].duration, 0.70f);
		TestTrue(TEXT("2구간 판정"), smash[1].phase == EEnemyAttackPhase::EAP_Active);
		TestEqual(TEXT("판정 시작 0.70 (예고 끝 = 판정 시작)"), smash[1].startTime, 0.70f);
		TestEqual(TEXT("후딜 시작 0.82"), smash[2].startTime, 0.82f);
	}
	TestEqual(TEXT("내리치기 전체 2.42초"), EnemyAttackTiming::GetTotalDuration(smash), 2.42f);

	/* 엇박(들어 올림 -> 멈춤 -> 내려옴 -> 판정 -> 후딜) */
	const TArray<FEnemyAttackPhaseSpan> slam = EnemyAttackTiming::BuildTimeline(TimingTestSlam());
	if (TestEqual(TEXT("엇박 구간 5개"), slam.Num(), 5))
	{
		TestEqual(TEXT("들어 올림 0.65"), slam[0].duration, 0.65f);
		TestTrue(TEXT("2구간 멈춤"), slam[1].phase == EEnemyAttackPhase::EAP_Hold);
		TestEqual(TEXT("멈춤 0.55"), slam[1].duration, 0.55f);
		TestTrue(TEXT("3구간 내려옴"), slam[2].phase == EEnemyAttackPhase::EAP_Release);
		TestEqual(TEXT("판정 시작 1.65"), slam[3].startTime, 1.65f);
	}

	/* 광증 예고 배율 */
	const TArray<FEnemyAttackPhaseSpan> frenzy = EnemyAttackTiming::BuildTimeline(TimingTestSmash(), 0, 0.6f);
	TestEqual(TEXT("광증 예고 0.42"), frenzy[0].duration, 0.42f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAttackRollWindowTest, "Windless.EnemyAI.Attack.RollWindow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAttackRollWindowTest::RunTest(const FString& Parameters)
{
	constexpr float invincibleTime = 0.35f;

	const FFloatInterval smash = EnemyAttackTiming::GetRollWindow(TimingTestSmash(), invincibleTime);
	TestEqual(TEXT("늘어진 팔 구르기 시작 0.47"), smash.Min, 0.47f);
	TestEqual(TEXT("늘어진 팔 구르기 끝 0.70"), smash.Max, 0.70f);

	const FFloatInterval slam = EnemyAttackTiming::GetRollWindow(TimingTestSlam(), invincibleTime);
	TestEqual(TEXT("거구 내려치기 구르기 시작 1.44"), slam.Min, 1.44f);
	TestEqual(TEXT("거구 내려치기 구르기 끝 1.65"), slam.Max, 1.65f);

	TestEqual(TEXT("섹션 이름 기본"), EnemyAttackTiming::GetPhaseSectionName(EEnemyAttackPhase::EAP_Windup, NAME_None), FName(TEXT("Windup")));
	TestEqual(TEXT("섹션 이름 접두어"), EnemyAttackTiming::GetPhaseSectionName(EEnemyAttackPhase::EAP_Active, FName(TEXT("Smash"))), FName(TEXT("Smash_Active")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAttackTrackingTest, "Windless.EnemyAI.Attack.Tracking",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAttackTrackingTest::RunTest(const FString& Parameters)
{
	using namespace EnemyAttackTiming;
	constexpr float tolerance = 0.001f;

	/* 점점 덜(예고 0.70) */
	TestEqual(TEXT("점점 덜 0.20 = 100%"), GetTrackScale(EEnemyTrackProfile::ETP_Decay, 0.70f, 0.20f), 1.f, tolerance);
	TestEqual(TEXT("점점 덜 0.35 = 100%"), GetTrackScale(EEnemyTrackProfile::ETP_Decay, 0.70f, 0.35f), 1.f, tolerance);
	TestEqual(TEXT("점점 덜 0.45 = 50%"), GetTrackScale(EEnemyTrackProfile::ETP_Decay, 0.70f, 0.45f), 0.5f, tolerance);
	TestEqual(TEXT("점점 덜 0.55 = 고정"), GetTrackScale(EEnemyTrackProfile::ETP_Decay, 0.70f, 0.55f), 0.f, tolerance);

	/* 끝까지(판정 0.15초 전까지 100%) */
	TestEqual(TEXT("끝까지 0.54 = 100%"), GetTrackScale(EEnemyTrackProfile::ETP_Strong, 0.70f, 0.54f), 1.f, tolerance);
	TestEqual(TEXT("끝까지 0.55 = 고정"), GetTrackScale(EEnemyTrackProfile::ETP_Strong, 0.70f, 0.55f), 0.f, tolerance);

	/* 없음 */
	TestEqual(TEXT("추적 없음 = 0"), GetTrackScale(EEnemyTrackProfile::ETP_None, 0.70f, 0.f), 0.f, tolerance);

	/* 들개 단발(예고 0.40, 점점 덜) */
	TestEqual(TEXT("들개 0.19 = 100%"), GetTrackScale(EEnemyTrackProfile::ETP_Decay, 0.40f, 0.19f), 1.f, tolerance);
	TestEqual(TEXT("들개 0.225 = 50%"), GetTrackScale(EEnemyTrackProfile::ETP_Decay, 0.40f, 0.225f), 0.5f, tolerance);
	TestEqual(TEXT("들개 0.25 이후 고정"), GetTrackScale(EEnemyTrackProfile::ETP_Decay, 0.40f, 0.2501f), 0.f, tolerance);

	/* 후딜 회전 복귀 */
	TestEqual(TEXT("후딜 0초 = 0"), GetRecoveryTurnScale(0.f), 0.f, tolerance);
	TestEqual(TEXT("후딜 0.5초 = 50%"), GetRecoveryTurnScale(0.5f), 0.5f, tolerance);
	TestEqual(TEXT("후딜 1.5초 = 100%"), GetRecoveryTurnScale(1.5f), 1.f, tolerance);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAttackTurnScaleTest, "Windless.EnemyAI.Attack.TurnScale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAttackTurnScaleTest::RunTest(const FString& Parameters)
{
	using namespace EnemyAttackTiming;
	constexpr float tolerance = 0.001f;

	/* 늘어진 팔 내리치기 */
	const FEnemyAttackRow smash = TimingTestSmash();
	const TArray<FEnemyAttackPhaseSpan> smashTimeline = BuildTimeline(smash);
	TestEqual(TEXT("예고 0.30 추적"), GetTurnScale(smash, smashTimeline, 0.30f), 1.f, tolerance);
	TestEqual(TEXT("예고 0.60 고정"), GetTurnScale(smash, smashTimeline, 0.60f), 0.f, tolerance);
	TestEqual(TEXT("판정 중 회전 0"), GetTurnScale(smash, smashTimeline, 0.76f), 0.f, tolerance);
	TestEqual(TEXT("후딜 0.5초 = 50%"), GetTurnScale(smash, smashTimeline, 1.32f), 0.5f, tolerance);
	TestEqual(TEXT("후딜 1초 뒤 100%"), GetTurnScale(smash, smashTimeline, 2.0f), 1.f, tolerance);

	/* 거구 내려치기 */
	const FEnemyAttackRow slam = TimingTestSlam();
	const TArray<FEnemyAttackPhaseSpan> slamTimeline = BuildTimeline(slam);
	TestEqual(TEXT("멈춤 1.0초 추적 74%"), GetTurnScale(slam, slamTimeline, 1.0f), 0.5f / 0.675f, tolerance);
	TestEqual(TEXT("내려옴 1.55초 고정"), GetTurnScale(slam, slamTimeline, 1.55f), 0.f, tolerance);

	TestEqual(TEXT("선회 구간 이름"), FString(GetPhaseName(EEnemyAttackPhase::EAP_PreTurn)), FString(TEXT("선회")));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS