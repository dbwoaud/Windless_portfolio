#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/AI/EnemyInterrupt.h"

/* 경직 중 재피격 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyHitRehitTest, "Windless.EnemyAI.Hit.Rehit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyHitRehitTest::RunTest(const FString& Parameters)
{
	using EnemyInterrupt::ResolveRehit;
	const EEnemyInterrupt none = EEnemyInterrupt::EIN_None;
	const EEnemyInterrupt weak = EEnemyInterrupt::EIN_StaggerWeak;
	const EEnemyInterrupt strong = EEnemyInterrupt::EIN_StaggerStrong;

	/* 경직이 아닐 때 */
	TestTrue(TEXT("평소 + 약한 결정: 시작"), ResolveRehit(none, EStaggerResult::ESR_Weak) == EInterruptHitAction::EIHA_Begin);
	TestTrue(TEXT("평소 + 강한 결정: 시작"), ResolveRehit(none, EStaggerResult::ESR_Strong) == EInterruptHitAction::EIHA_Begin);
	TestTrue(TEXT("평소 + 버팀: 변화 없음"), ResolveRehit(none, EStaggerResult::ESR_Endure) == EInterruptHitAction::EIHA_None);
	TestTrue(TEXT("평소 + 예고 0.50 미만: 변화 없음"), ResolveRehit(none, EStaggerResult::ESR_ShortWindup) == EInterruptHitAction::EIHA_None);

	/* 경직 세기와 결정 */
	TestTrue(TEXT("약 중 약: 시간 다시 시작"), ResolveRehit(weak, EStaggerResult::ESR_Weak) == EInterruptHitAction::EIHA_Restart);
	TestTrue(TEXT("약 중 강: 강한 경직으로"), ResolveRehit(weak, EStaggerResult::ESR_Strong) == EInterruptHitAction::EIHA_Promote);
	TestTrue(TEXT("강 중 강: 처음부터 다시 날림"), ResolveRehit(strong, EStaggerResult::ESR_Strong) == EInterruptHitAction::EIHA_Relaunch);
	TestTrue(TEXT("강 중 약: HP만 감소"), ResolveRehit(strong, EStaggerResult::ESR_Weak) == EInterruptHitAction::EIHA_HpOnly);

	/* 경직 중 버팀, 경직 없음 */
	TestTrue(TEXT("약 중 버팀: 변화 없음"), ResolveRehit(weak, EStaggerResult::ESR_Endure) == EInterruptHitAction::EIHA_None);
	TestTrue(TEXT("강 중 예고 0.50 미만: 변화 없음"), ResolveRehit(strong, EStaggerResult::ESR_ShortWindup) == EInterruptHitAction::EIHA_None);

	/* 경직 결정 */
	TestTrue(TEXT("약한 결정: 약한 경직"), EnemyInterrupt::ToInterrupt(EStaggerResult::ESR_Weak) == weak);
	TestTrue(TEXT("강한 결정: 강한 경직"), EnemyInterrupt::ToInterrupt(EStaggerResult::ESR_Strong) == strong);
	TestTrue(TEXT("버팀: 없음"), EnemyInterrupt::ToInterrupt(EStaggerResult::ESR_Endure) == none);
	return true;
}


/* 강한 경직 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyHitStrongStaggerTest, "Windless.EnemyAI.Hit.StrongStagger",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyHitStrongStaggerTest::RunTest(const FString& Parameters)
{
	using EnemyInterrupt::GetStrongPhase;
	const FStrongStaggerTimes times;

	/* 구간 경계 */
	TestEqual(TEXT("전체 2.3초"), times.GetTotal(), 2.3f, 0.001f);
	TestTrue(TEXT("0.00: 날아감"), GetStrongPhase(times, 0.f) == EStrongStaggerPhase::ESSP_Launch);
	TestTrue(TEXT("0.49: 날아감"), GetStrongPhase(times, 0.49f) == EStrongStaggerPhase::ESSP_Launch);
	TestTrue(TEXT("0.50: 다운"), GetStrongPhase(times, 0.50f) == EStrongStaggerPhase::ESSP_Down);
	TestTrue(TEXT("1.49: 다운"), GetStrongPhase(times, 1.49f) == EStrongStaggerPhase::ESSP_Down);
	TestTrue(TEXT("1.50: 기상"), GetStrongPhase(times, 1.50f) == EStrongStaggerPhase::ESSP_GetUp);
	TestTrue(TEXT("2.30: 끝"), GetStrongPhase(times, 2.30f) == EStrongStaggerPhase::ESSP_Done);

	/* 날아감 */
	const float gravityZ = -980.f;
	const FVector velocity = EnemyInterrupt::ComputeLaunchVelocity(FVector(1.f, 0.f, 0.f), 300.f, 0.5f, gravityZ);
	TestEqual(TEXT("수평 속도 600cm/s"), velocity.X, 600.0, 0.01);
	TestEqual(TEXT("옆으로 새지 않음"), velocity.Y, 0.0, 0.01);
	TestEqual(TEXT("수직 속도 245cm/s"), velocity.Z, 245.0, 0.01);

	const float t = 0.5f;
	TestEqual(TEXT("0.5초 뒤 수평 거리 3m"), velocity.X * t, 300.0, 0.01);
	TestEqual(TEXT("0.5초 뒤 같은 높이로 착지"), velocity.Z * t + 0.5f * gravityZ * t * t, 0.0, 0.01);

	const FVector diagonal = EnemyInterrupt::ComputeLaunchVelocity(FVector(0.f, 1.f, 1.f), 300.f, 0.5f, gravityZ);
	TestEqual(TEXT("대각선 입력도 수평 600cm/s"), FVector(diagonal.X, diagonal.Y, 0.f).Size(), 600.0, 0.01);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS