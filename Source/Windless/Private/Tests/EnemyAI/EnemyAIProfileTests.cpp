#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/AI/EnemyAIProfile.h"
#include "Enemy/AI/MonsterArchetypeRow.h"

/* 적 AI 1장 5겹 테스트 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIProfileDroopingArmTest, "Windless.EnemyAI.Profile.DroopingArm",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIProfileDroopingArmTest::RunTest(const FString& Parameters)
{
	FMonsterArchetypeRow archetype; // 기본값 = 늘어진 팔 (일반, 기형형, 기본 성향, 추적 25, 멈춰 섬, 광증 없음)
	const FMonsterProfile profile = EnemyAIProfile::Resolve(archetype, EEnemyIdleType::EIT_None);

	TestTrue(TEXT("성향 기본 ← 외형"), profile.temperament.value == EEnemyTemperament::ETMP_Normal && profile.temperament.source == EProfileSource::Appearance);
	TestTrue(TEXT("무리 공격 X ← 외형"), !profile.bPackAttacker.value && profile.bPackAttacker.source == EProfileSource::Appearance);
	TestTrue(TEXT("반응 회피 X ← 외형"), !profile.bCanDodgeRoll.value && profile.bCanDodgeRoll.source == EProfileSource::Appearance);
	TestTrue(TEXT("반응 맞받아침 ← 외형"), profile.reaction.value == EEnemyReactionType::ERT_Counter && profile.reaction.source == EProfileSource::Appearance);
	TestTrue(TEXT("대기 멈춰 섬 ← 원형"), profile.idleType.value == EEnemyIdleType::EIT_Stand && profile.idleType.source == EProfileSource::Archetype);

	TestEqual(TEXT("가까이 2.5 (근거리 기본)"), profile.nearRange.value, 2.5f);
	TestTrue(TEXT("가까이 ← 공통"), profile.nearRange.source == EProfileSource::Common);
	TestEqual(TEXT("중간 6 (근거리 기본)"), profile.midRange.value, 6.f);
	TestEqual(TEXT("멀리 0 (근거리형)"), profile.farRange.value, 0.f);

	TestEqual(TEXT("추적 25"), profile.leash.value, 25.f);
	TestEqual(TEXT("최대 추적 40 = 25 + 15"), profile.maxLeash.value, 40.f);
	TestEqual(TEXT("광증 없음"), profile.frenzyConvulsionEvery.value, 0);
	TestFalse(TEXT("리셋 부활 (일반)"), profile.bPermanentDeath.value);
	TestFalse(TEXT("광증 방식 = 예고 X 0.6 (일반)"), profile.bFrenzyAttackSet.value);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIProfileCollapsedGiantTest, "Windless.EnemyAI.Profile.CollapsedGiant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIProfileCollapsedGiantTest::RunTest(const FString& Parameters)
{
	FMonsterArchetypeRow archetype; // 무너진 거구
	archetype.grade = EMonsterGrade::EGRD_Elite;
	archetype.nearRange = 4.f;
	archetype.midRange = 9.f;
	archetype.leashFromHome = 30.f;
	archetype.bHasFrenzy = true;

	const FMonsterProfile profile = EnemyAIProfile::Resolve(archetype, EEnemyIdleType::EIT_None);

	TestEqual(TEXT("가까이 4.0 ← 원형"), profile.nearRange.value, 4.f);
	TestTrue(TEXT("가까이 ← 원형"), profile.nearRange.source == EProfileSource::Archetype);
	TestEqual(TEXT("중간 9.0"), profile.midRange.value, 9.f);
	TestEqual(TEXT("최대 추적 45"), profile.maxLeash.value, 45.f);
	TestEqual(TEXT("광증 경련 3회마다"), profile.frenzyConvulsionEvery.value, 3);
	TestTrue(TEXT("광증 경련 ← 성향"), profile.frenzyConvulsionEvery.source == EProfileSource::Temperament);
	TestTrue(TEXT("광증 방식 = 전용 세트 ← 등급"), profile.bFrenzyAttackSet.value && profile.bFrenzyAttackSet.source == EProfileSource::Grade);
	TestFalse(TEXT("리셋 부활 (엘리트)"), profile.bPermanentDeath.value);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIProfilePlacementTest, "Windless.EnemyAI.Profile.Placement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIProfilePlacementTest::RunTest(const FString& Parameters)
{
	FMonsterArchetypeRow archetype;
	archetype.leashFromHome = 30.f;

	const FMonsterProfile defaultProfile = EnemyAIProfile::Resolve(archetype, EEnemyIdleType::EIT_None);
	const FMonsterProfile placedProfile = EnemyAIProfile::Resolve(archetype, EEnemyIdleType::EIT_Circle);

	TestTrue(TEXT("배치 None → 원형 기본 대기"), defaultProfile.idleType.value == EEnemyIdleType::EIT_Stand);
	TestTrue(TEXT("배치 맴돌기 ← 배치"), placedProfile.idleType.value == EEnemyIdleType::EIT_Circle && placedProfile.idleType.source == EProfileSource::Placement);
	TestEqual(TEXT("배치는 추적 거리를 바꾸지 않는다"), placedProfile.leash.value, defaultProfile.leash.value);

	archetype.defaultIdleType = EEnemyIdleType::EIT_None;
	const FMonsterProfile emptyProfile = EnemyAIProfile::Resolve(archetype, EEnemyIdleType::EIT_None);
	TestTrue(TEXT("원형 및 배치 모두 None → 멈춰 섬 ← 공통"), emptyProfile.idleType.value == EEnemyIdleType::EIT_Stand && emptyProfile.idleType.source == EProfileSource::Common);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIProfileAppearanceTest, "Windless.EnemyAI.Profile.AppearanceDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIProfileAppearanceTest::RunTest(const FString& Parameters)
{
	/* 짐승형 */
	FMonsterArchetypeRow beast;
	beast.appearance = EMonsterAppearance::EAPP_Beast;
	beast.bHasFrenzy = true;
	const FMonsterProfile beastProfile = EnemyAIProfile::Resolve(beast, EEnemyIdleType::EIT_None);
	TestTrue(TEXT("짐승형 무리 공격 O ← 외형"), beastProfile.bPackAttacker.value && beastProfile.bPackAttacker.source == EProfileSource::Appearance);
	TestTrue(TEXT("짐승형 성향 공격적 ← 외형"), beastProfile.temperament.value == EEnemyTemperament::ETMP_Aggressive && beastProfile.temperament.source == EProfileSource::Appearance);
	TestEqual(TEXT("공격적 광증 N = 4"), beastProfile.frenzyConvulsionEvery.value, 4);

	/* 조류형 */
	FMonsterArchetypeRow avian;
	avian.appearance = EMonsterAppearance::EAPP_Avian;
	TestTrue(TEXT("조류형 무리 공격 O"), EnemyAIProfile::Resolve(avian, EEnemyIdleType::EIT_None).bPackAttacker.value);

	/* 인간형 */
	FMonsterArchetypeRow humanoid;
	humanoid.appearance = EMonsterAppearance::EAPP_Humanoid;
	const FMonsterProfile humanoidProfile = EnemyAIProfile::Resolve(humanoid, EEnemyIdleType::EIT_None);
	TestTrue(TEXT("인간형 반응 회피 O ← ②외형"), humanoidProfile.bCanDodgeRoll.value && humanoidProfile.bCanDodgeRoll.source == EProfileSource::Appearance);
	TestTrue(TEXT("인간형 반응 = 회피"), humanoidProfile.reaction.value == EEnemyReactionType::ERT_Dodge);

	/* 살가죽 방패 */
	FMonsterArchetypeRow shield = humanoid;
	shield.bCanGuard = true;
	shield.canDodgeRoll = EEnemyOverride::EOVR_On;
	const FMonsterProfile shieldProfile = EnemyAIProfile::Resolve(shield, EEnemyIdleType::EIT_None);
	TestTrue(TEXT("방패 → 반응 회피 X ← 원형"), !shieldProfile.bCanDodgeRoll.value && shieldProfile.bCanDodgeRoll.source == EProfileSource::Archetype);
	TestTrue(TEXT("방패 → 반응 가드 ← 원형"), shieldProfile.reaction.value == EEnemyReactionType::ERT_Guard && shieldProfile.reaction.source == EProfileSource::Archetype);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIProfileOverridesTest, "Windless.EnemyAI.Profile.OverridesAndRoles",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIProfileOverridesTest::RunTest(const FString& Parameters)
{
	/* 묶인 늘어진 팔 */
	FMonsterArchetypeRow calm;
	calm.temperament = EEnemyTemperament::ETMP_Calm;
	calm.bHasFrenzy = true;
	const FMonsterProfile calmProfile = EnemyAIProfile::Resolve(calm, EEnemyIdleType::EIT_None);
	TestTrue(TEXT("성향 차분 ← 원형"), calmProfile.temperament.value == EEnemyTemperament::ETMP_Calm && calmProfile.temperament.source == EProfileSource::Archetype);
	TestEqual(TEXT("차분 광증 N = 2"), calmProfile.frenzyConvulsionEvery.value, 2);

	/* 광증 N 직접 지정 */
	calm.frenzyConvulsionEvery = 5;
	const FMonsterProfile fixedProfile = EnemyAIProfile::Resolve(calm, EEnemyIdleType::EIT_None);
	TestTrue(TEXT("광증 N 5 ← ④원형"), fixedProfile.frenzyConvulsionEvery.value == 5 && fixedProfile.frenzyConvulsionEvery.source == EProfileSource::Archetype);

	/* 무리 공격 해제 */
	FMonsterArchetypeRow beastOff;
	beastOff.appearance = EMonsterAppearance::EAPP_Beast;
	beastOff.packAttacker = EEnemyOverride::EOVR_Off;
	const FMonsterProfile beastOffProfile = EnemyAIProfile::Resolve(beastOff, EEnemyIdleType::EIT_None);
	TestTrue(TEXT("짐승형 무리 공격 해제 ← 원형"), !beastOffProfile.bPackAttacker.value && beastOffProfile.bPackAttacker.source == EProfileSource::Archetype);
	TestTrue(TEXT("최종 무리 공격 X → 성향 기본"), beastOffProfile.temperament.value == EEnemyTemperament::ETMP_Normal);

	/* 원거리 역할 기본 구간 */
	FMonsterArchetypeRow ranged;
	ranged.combatRole = EEnemyCombatRole::ECR_Ranged;
	const FMonsterProfile rangedProfile = EnemyAIProfile::Resolve(ranged, EEnemyIdleType::EIT_None);
	TestEqual(TEXT("원거리 가까이 3"), rangedProfile.nearRange.value, 3.f);
	TestEqual(TEXT("원거리 중간 12"), rangedProfile.midRange.value, 12.f);
	TestEqual(TEXT("원거리 멀리 20"), rangedProfile.farRange.value, 20.f);
	TestTrue(TEXT("원거리 멀리 ← 공통"), rangedProfile.farRange.source == EProfileSource::Common);

	/* 보스 */
	FMonsterArchetypeRow boss;
	boss.grade = EMonsterGrade::EGRD_MainBoss;
	const FMonsterProfile bossProfile = EnemyAIProfile::Resolve(boss, EEnemyIdleType::EIT_None);
	TestTrue(TEXT("메인보스 리셋 영구 사망 ← 등급"), bossProfile.bPermanentDeath.value && bossProfile.bPermanentDeath.source == EProfileSource::Grade);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS