#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/AI/EnemyAIDataValidation.h"
#include "Enemy/AI/EnemyAttackRow.h"
#include "Enemy/AI/MonsterArchetypeRow.h"
#include "Sound/SoundBase.h"

/* 적 AI 부록 B 저장 시 검사 테스트 */

namespace
{
	/* 로그에 태그를 확인하는 함수 */
	bool ValidationTestHas(const TArray<FString>& lines, const TCHAR* prefix)
	{
		return lines.ContainsByPredicate([prefix](const FString& line) { return line.StartsWith(prefix); });
	}

	/* 공격이 규칙을 지켰는지 확인하는 함수 */
	FEnemyAttackRow ValidationTestAttack(EEnemyAttackClass attackClass, const TArray<float>& windups, float range = 2.f)
	{
		FEnemyAttackRow attack;
		attack.attackClass = attackClass;
		attack.card = attackClass == EEnemyAttackClass::EATC_Light ? EEnemyCard::EC_LightAttack : EEnemyCard::EC_HeavyAttack;
		attack.windups = windups;
		for (int32 i = 0; i < windups.Num(); ++i)
		{
			attack.hitRanges.Add(range);
			attack.activeTimes.Add(0.1f);
			attack.trackProfiles.Add(EEnemyTrackProfile::ETP_Decay);
			if (i + 1 < windups.Num())
				attack.hitIntervals.Add(FMath::Max(windups[i + 1], 0.5f));
		}
		attack.windupSound = TSoftObjectPtr<USoundBase>(FSoftObjectPath(TEXT("/Game/Test/S_TestWindup.S_TestWindup")));
		return attack;
	}

	/* 엇박 강공격이 규칙을 지켰는지 확인하는 함수 */
	FEnemyAttackRow ValidationTestHoldAttack()
	{
		FEnemyAttackRow attack = ValidationTestAttack(EEnemyAttackClass::EATC_Heavy, { 1.65f });
		attack.windupHoldTime = 0.55f;
		attack.holdReleaseTime = 0.45f;
		return attack;
	}

	/* 콤보 공격 추적(2타) */
	FEnemyAttackRow ValidationTestMultiTrackAttack()
	{
		FEnemyAttackRow attack = ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.75f, 0.6f });
		attack.hitIntervals = { 0.8f };
		attack.trackProfiles[1] = EEnemyTrackProfile::ETP_Strong;
		return attack;
	}

	/* 몬스터 분류 칸에 맞춰 ID를 채웠는지 확인하는 함수 */
	FMonsterArchetypeRow ValidationTestArchetype(EMonsterGrade grade, const TArray<FName>& attackIds)
	{
		FMonsterArchetypeRow archetype;
		archetype.grade = grade;
		archetype.attackIds = attackIds;
		archetype.monsterId = EnemyAIDataValidation::BuildExpectedMonsterId(archetype) + TEXT("_01");
		return archetype;
	}

	/* 테스트용 공격 테이블 */
	struct FValidationTestAttackSet
	{
		TMap<FName, FEnemyAttackRow> attacks;

		FValidationTestAttackSet()
		{
			attacks.Add(TEXT("Plain"), ValidationTestAttack(EEnemyAttackClass::EATC_Heavy, { 0.7f }));
			attacks.Add(TEXT("PlainB"), ValidationTestAttack(EEnemyAttackClass::EATC_Heavy, { 0.9f }));
			attacks.Add(TEXT("PlainLight"), ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.5f }));
			attacks.Add(TEXT("Hold"), ValidationTestHoldAttack());
			attacks.Add(TEXT("Multi"), ValidationTestMultiTrackAttack());

			FEnemyAttackRow full = ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.55f, 0.45f });
			full.comboType = EEnemyComboType::ECT_Full;
			attacks.Add(TEXT("Full"), full);

			FEnemyAttackRow farSecond = ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.55f, 0.45f });
			farSecond.hitRanges[1] = 3.f;
			attacks.Add(TEXT("FarSecond"), farSecond);

			FEnemyAttackRow strongLight = ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.6f });
			strongLight.bCanStaggerStrong = true;
			attacks.Add(TEXT("StrongLight"), strongLight);
		}

		/* 실제 공격 테스트를 수행하는 함수 */
		FEnemyAIValidationResult Run(const FMonsterArchetypeRow& archetype) const
		{
			FEnemyAIValidationResult result;
			EnemyAIDataValidation::ValidateArchetype(archetype, TEXT("Test"), [this](FName id) -> const FEnemyAttackRow*
				{
					return attacks.Find(id);
				}, result);
			return result;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIValidationBaselineTest, "Windless.EnemyAI.Validation.Baseline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIValidationBaselineTest::RunTest(const FString& Parameters)
{
	const FEnemyAIReferenceValues reference;
	const FValidationTestAttackSet set;

	for (const TPair<FName, FEnemyAttackRow>& pair : set.attacks)
	{
		FEnemyAIValidationResult result;
		EnemyAIDataValidation::ValidateAttack(pair.Value, pair.Key.ToString(), reference, result);
		TestEqual(FString::Printf(TEXT("%s 에러 0"), *pair.Key.ToString()), result.errors.Num(), 0);
		TestEqual(FString::Printf(TEXT("%s 경고 0"), *pair.Key.ToString()), result.warnings.Num(), 0);
	}

	/* 일반 원형(늘어진 팔의 정박 강공격) */
	const FEnemyAIValidationResult normal = set.Run(ValidationTestArchetype(EMonsterGrade::EGRD_Normal, { TEXT("Plain") }));
	TestEqual(TEXT("일반 원형 에러 0"), normal.errors.Num(), 0);
	TestEqual(TEXT("일반 원형 경고 0"), normal.warnings.Num(), 0);

	/* 엘리트 원형(무너진 거구의 엇박 공격 1개와 정박 공격 3개) */
	const FEnemyAIValidationResult elite = set.Run(ValidationTestArchetype(EMonsterGrade::EGRD_Elite, { TEXT("Hold"), TEXT("Plain"), TEXT("PlainB"), TEXT("PlainLight") }));
	TestEqual(TEXT("엘리트 원형 에러 0"), elite.errors.Num(), 0);
	TestEqual(TEXT("엘리트 원형 경고 0"), elite.warnings.Num(), 0);

	/* 엘리트 원형(긴 뼈 창 의 콤보 공격) */
	const FEnemyAIValidationResult spear = set.Run(ValidationTestArchetype(EMonsterGrade::EGRD_Elite, { TEXT("Multi"), TEXT("Full"), TEXT("Plain"), TEXT("PlainLight") }));
	TestEqual(TEXT("엘리트(콤보 공격 추적) 원형 에러 0"), spear.errors.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIValidationAttackRulesTest, "Windless.EnemyAI.Validation.AttackRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIValidationAttackRulesTest::RunTest(const FString& Parameters)
{
	const FEnemyAIReferenceValues reference;

	auto expectError = [this, &reference](const TCHAR* what, const FEnemyAttackRow& attack, const TCHAR* prefix)
		{
			FEnemyAIValidationResult result;
			EnemyAIDataValidation::ValidateAttack(attack, TEXT("Test"), reference, result);
			TestTrue(FString::Printf(TEXT("%s → %s 에러"), what, prefix), ValidationTestHas(result.errors, prefix));
		};

	FEnemyAttackRow badArray = ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.5f, 0.5f });
	badArray.windups.Pop();
	expectError(TEXT("windups 개수 ≠ 타수"), badArray, TEXT("[배열]"));

	/* 예고 */
	expectError(TEXT("예고 0.39"), ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.39f }), TEXT("[V5 "));

	/* 타수 */
	expectError(TEXT("약공격 4타"), ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.5f, 0.5f, 0.5f, 0.5f }), TEXT("[V7 "));
	expectError(TEXT("강공격 2타"), ValidationTestAttack(EEnemyAttackClass::EATC_Heavy, { 0.8f, 0.8f }), TEXT("[V7 "));

	/* 엇박 */
	FEnemyAttackRow lightHold = ValidationTestHoldAttack();
	lightHold.attackClass = EEnemyAttackClass::EATC_Light;
	expectError(TEXT("약공격 엇박"), lightHold, TEXT("[V4 "));

	FEnemyAttackRow shortHold = ValidationTestHoldAttack();
	shortHold.windupHoldTime = 0.40f;
	expectError(TEXT("멈춤 0.40"), shortHold, TEXT("[V4 "));

	/* 멈춤 뒤 판정 */
	FEnemyAttackRow shortRelease = ValidationTestHoldAttack();
	shortRelease.holdReleaseTime = 0.40f;
	expectError(TEXT("멈춤 뒤 0.40"), shortRelease, TEXT("[V11 "));

	/* 예고 < 멈춤 + 내려옴 */
	FEnemyAttackRow shortWindup = ValidationTestHoldAttack();
	shortWindup.windups = { 0.9f };
	expectError(TEXT("예고 0.9 < 0.55 + 0.45"), shortWindup, TEXT("[엇박]"));

	/* 간격 < 다음 타 예고 */
	FEnemyAttackRow shortInterval = ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.5f, 0.6f });
	shortInterval.hitIntervals = { 0.5f };
	expectError(TEXT("간격 0.5 < 2타 예고 0.6"), shortInterval, TEXT("[V12 "));

	/* 예고 효과음 */
	FEnemyAttackRow noSound = ValidationTestAttack(EEnemyAttackClass::EATC_Heavy, { 0.7f });
	noSound.windupSound.Reset();
	expectError(TEXT("예고 효과음 없음"), noSound, TEXT("[V9 "));

	/* 약한 경직 확정 연타 */
	FEnemyAttackRow lockCombo = ValidationTestAttack(EEnemyAttackClass::EATC_Light, { 0.55f, 0.55f });
	lockCombo.hitIntervals = { 0.55f };
	lockCombo.impact = 20.f;
	FEnemyAIValidationResult lockResult;
	EnemyAIDataValidation::ValidateAttack(lockCombo, TEXT("Test"), reference, lockResult);
	TestTrue(TEXT("충격량 20 > 15 · 예고 0.55 · 간격 0.55 < 0.6 → [V16 경고"), ValidationTestHas(lockResult.warnings, TEXT("[V16 ")));
	TestEqual(TEXT("V16은 에러가 아님"), lockResult.errors.Num(), 0);

	FEnemyAttackRow safeCombo = lockCombo;
	safeCombo.windups[1] = 0.45f;
	FEnemyAIValidationResult safeResult;
	EnemyAIDataValidation::ValidateAttack(safeCombo, TEXT("Test"), reference, safeResult);
	TestFalse(TEXT("2타 예고 0.45 → V16 없음"), ValidationTestHas(safeResult.warnings, TEXT("[V16 ")));

	/* V광증 전용 공격 */
	FEnemyAttackRow frenzyAttack = ValidationTestHoldAttack();
	frenzyAttack.windupHoldTime = 0.30f;
	frenzyAttack.holdReleaseTime = 0.30f;
	expectError(TEXT("광증 공격 멈춤 0.30"), frenzyAttack, TEXT("[V4 "));
	expectError(TEXT("광증 공격 멈춤 뒤 0.30"), frenzyAttack, TEXT("[V11 "));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIValidationArchetypeRulesTest, "Windless.EnemyAI.Validation.ArchetypeRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIValidationArchetypeRulesTest::RunTest(const FString& Parameters)
{
	const FValidationTestAttackSet set;

	auto expectError = [this, &set](const TCHAR* what, const FMonsterArchetypeRow& archetype, const TCHAR* prefix)
		{
			TestTrue(FString::Printf(TEXT("%s → %s 에러"), what, prefix), ValidationTestHas(set.Run(archetype).errors, prefix));
		};

	const FMonsterArchetypeRow normalBase = ValidationTestArchetype(EMonsterGrade::EGRD_Normal, { TEXT("Plain") });
	const FMonsterArchetypeRow eliteBase = ValidationTestArchetype(EMonsterGrade::EGRD_Elite, { TEXT("Hold"), TEXT("Plain"), TEXT("PlainB"), TEXT("PlainLight") });

	FMonsterArchetypeRow dodgeHigh = normalBase;
	dodgeHigh.dodgeChance = 0.6f;
	expectError(TEXT("회피 확률 0.6"), dodgeHigh, TEXT("[V1 "));

	FMonsterArchetypeRow guardDodge = normalBase;
	guardDodge.bCanGuard = true;
	guardDodge.canDodgeRoll = EEnemyOverride::EOVR_On;
	expectError(TEXT("방패 + 반응 회피 On"), guardDodge, TEXT("[V15 "));

	/* 참조 */
	FMonsterArchetypeRow missingAttack = normalBase;
	missingAttack.attackIds.Add(TEXT("NoSuchAttack"));
	expectError(TEXT("없는 공격 행"), missingAttack, TEXT("[참조]"));

	FMonsterArchetypeRow strayCard = normalBase;
	FEnemySpecialCard card;
	card.attackId = TEXT("PlainB");
	strayCard.specialCards.Add(card);
	expectError(TEXT("목록에 없는 특수 카드"), strayCard, TEXT("[참조]"));

	/* 엘리트 공격 */
	FMonsterArchetypeRow eliteThree = eliteBase;
	eliteThree.attackIds.Pop();
	expectError(TEXT("엘리트 공격 3종"), eliteThree, TEXT("[V14 "));

	/* 일반 등급 광증 세트 */
	FMonsterArchetypeRow normalFrenzySet = normalBase;
	normalFrenzySet.bHasFrenzy = true;
	normalFrenzySet.frenzyAttackSet.Add(TEXT("PlainB"));
	expectError(TEXT("일반 등급 광증 세트"), normalFrenzySet, TEXT("[광증]"));

	/* 일반 등급 공격 규칙 */
	expectError(TEXT("일반 등급 끝까지 연타"), ValidationTestArchetype(EMonsterGrade::EGRD_Normal, { TEXT("Full") }), TEXT("[V2 "));
	expectError(TEXT("일반 등급 2타 사거리 > 1타"), ValidationTestArchetype(EMonsterGrade::EGRD_Normal, { TEXT("FarSecond") }), TEXT("[V3 "));
	expectError(TEXT("일반 등급 엇박"), ValidationTestArchetype(EMonsterGrade::EGRD_Normal, { TEXT("Hold") }), TEXT("[V4 "));
	expectError(TEXT("일반 등급 약공격 강한 경직"), ValidationTestArchetype(EMonsterGrade::EGRD_Normal, { TEXT("StrongLight") }), TEXT("[V8 "));
	expectError(TEXT("일반 등급 다단 추적"), ValidationTestArchetype(EMonsterGrade::EGRD_Normal, { TEXT("Multi") }), TEXT("[V13 "));

	/* 엘리트 */
	expectError(TEXT("엘리트 엇박 + 다단 추적"), ValidationTestArchetype(EMonsterGrade::EGRD_Elite, { TEXT("Hold"), TEXT("Multi"), TEXT("Plain"), TEXT("PlainB") }), TEXT("[V13 "));
	expectError(TEXT("엘리트 회피 방해 없음"), ValidationTestArchetype(EMonsterGrade::EGRD_Elite, { TEXT("Plain"), TEXT("PlainB"), TEXT("PlainLight"), TEXT("Full") }), TEXT("[V13 "));

	/* 보스 */
	expectError(TEXT("보스 다단 추적 없음"), ValidationTestArchetype(EMonsterGrade::EGRD_FieldBoss, { TEXT("Hold"), TEXT("Plain"), TEXT("PlainB"), TEXT("PlainLight") }), TEXT("[V13 "));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIValidationMonsterIdTest, "Windless.EnemyAI.Validation.MonsterId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIValidationMonsterIdTest::RunTest(const FString& Parameters)
{
	FMonsterArchetypeRow giant;
	giant.grade = EMonsterGrade::EGRD_Elite;
	TestEqual(TEXT("무너진 거구"), EnemyAIDataValidation::BuildExpectedMonsterId(giant), FString(TEXT("MON_AB_121111")));

	FMonsterArchetypeRow spear;
	spear.grade = EMonsterGrade::EGRD_Elite;
	spear.appearance = EMonsterAppearance::EAPP_Humanoid;
	spear.sociality = EMonsterSociality::ESOC_Call;
	TestEqual(TEXT("긴 뼈 창"), EnemyAIDataValidation::BuildExpectedMonsterId(spear), FString(TEXT("MON_HM_121112")));

	FMonsterArchetypeRow dogs;
	dogs.appearance = EMonsterAppearance::EAPP_Beast;
	dogs.sociality = EMonsterSociality::ESOC_Group;
	TestEqual(TEXT("들개 떼"), EnemyAIDataValidation::BuildExpectedMonsterId(dogs), FString(TEXT("MON_BS_111113")));

	FMonsterArchetypeRow skinShield;
	skinShield.appearance = EMonsterAppearance::EAPP_Humanoid;
	skinShield.sociality = EMonsterSociality::ESOC_Group;
	TestEqual(TEXT("살가죽 방패"), EnemyAIDataValidation::BuildExpectedMonsterId(skinShield), FString(TEXT("MON_HM_111113")));

	/* 일치 판정 */
	TestTrue(TEXT("순서 번호 있음"), EnemyAIDataValidation::MatchesMonsterId(TEXT("MON_AB_121111_01"), TEXT("MON_AB_121111")));
	TestTrue(TEXT("순서 번호 없음"), EnemyAIDataValidation::MatchesMonsterId(TEXT("MON_AB_121111"), TEXT("MON_AB_121111")));
	TestFalse(TEXT("등급 자리 다름"), EnemyAIDataValidation::MatchesMonsterId(TEXT("MON_AB_111111_01"), TEXT("MON_AB_121111")));
	TestFalse(TEXT("순서 번호가 숫자 아님 (옛 시트 _xx)"), EnemyAIDataValidation::MatchesMonsterId(TEXT("MON_AB_121111_xx"), TEXT("MON_AB_121111")));
	TestTrue(TEXT("공격 방식 여럿 → 4번째 자리 검사 안 함 (OPEN-03)"), EnemyAIDataValidation::MatchesMonsterId(TEXT("MON_AB_111211_01"), TEXT("MON_AB_111?11")));

	const FValidationTestAttackSet set;
	FMonsterArchetypeRow wrongId = ValidationTestArchetype(EMonsterGrade::EGRD_Normal, { TEXT("Plain") });
	wrongId.monsterId = TEXT("MON_AB_121111_01"); 
	const FEnemyAIValidationResult result = set.Run(wrongId);
	TestTrue(TEXT("ID 불일치 → [V10 경고"), ValidationTestHas(result.warnings, TEXT("[V10 ")));
	TestFalse(TEXT("V10은 에러가 아님"), ValidationTestHas(result.errors, TEXT("[V10 ")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyAIDataCurrentTablesTest, "Windless.EnemyAI.Data.CurrentTables",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyAIDataCurrentTablesTest::RunTest(const FString& Parameters)
{
	int32 archetypeCount = 0;
	int32 attackCount = 0;
	const FEnemyAIValidationResult result = EnemyAIDataValidation::ValidateAllTables(archetypeCount, attackCount);

	AddInfo(FString::Printf(TEXT("원형 %d종 · 공격 %d종 - 에러 %d · 경고 %d"), archetypeCount, attackCount, result.errors.Num(), result.warnings.Num()));
	TestTrue(TEXT("원형 테이블에 행이 있음"), archetypeCount > 0);
	TestTrue(TEXT("공격 테이블에 행이 있음"), attackCount > 0);

	for (const FString& error : result.errors)
	{
		if (!error.StartsWith(TEXT("[V9 ")))
			AddError(FString::Printf(TEXT("V9 외 에러: %s"), *error));
	}
	for (const FString& warning : result.warnings)
		AddError(FString::Printf(TEXT("경고: %s"), *warning));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS