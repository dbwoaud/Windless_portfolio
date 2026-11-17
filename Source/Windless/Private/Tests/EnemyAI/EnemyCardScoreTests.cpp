#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Enemy/AI/EnemyCardScore.h"
#include "Enemy/AI/EnemyCardBaseRow.h"
#include "Enemy/AI/EnemyAttackRow.h" 
#include "Enemy/AI/EnemyAISettings.h"
#include "Enemy/AI/EnemyCardMove.h"
#include "Enemy/AI/EnemyReach.h"
#include "Engine/DataTable.h"

namespace
{
	/* 근거리(가까이) */
	FCardBaseScores CardTestMeleeNear(EEnemyTemperament temperament)
	{
		const int32 column = temperament == EEnemyTemperament::ETMP_Calm ? 0 : (temperament == EEnemyTemperament::ETMP_Aggressive ? 2 : 1);
		const float light[3] = { 28.f, 35.f, 45.f };
		const float heavy[3] = { 17.f, 20.f, 20.f };
		const float side[3] = { 25.f, 20.f, 15.f };
		const float back[3] = { 12.f, 10.f, 5.f };
		const float rest[3] = { 18.f, 15.f, 15.f };

		FCardBaseScores scores;
		scores.Add(EEnemyCard::EC_LightAttack, light[column]);
		scores.Add(EEnemyCard::EC_HeavyAttack, heavy[column]);
		scores.Add(EEnemyCard::EC_Rush, 0.f);
		scores.Add(EEnemyCard::EC_SideStep, side[column]);
		scores.Add(EEnemyCard::EC_BackOff, back[column]);
		scores.Add(EEnemyCard::EC_Approach, 0.f);
		scores.Add(EEnemyCard::EC_Rest, rest[column]);
		return scores;
	}

	FCardAttackOption CardTestAttack(const TCHAR* attackId, EEnemyCard card, float weight = 1.f)
	{
		FCardAttackOption option;
		option.attackId = FName(attackId);
		option.card = card;
		option.shareWeight = weight;
		return option;
	}

	FEnemyCardScoreOverride CardTestOverride(EEnemyCard card, EEnemyRangeBand band, float score)
	{
		FEnemyCardScoreOverride scoreOverride;
		scoreOverride.card = card;
		scoreOverride.band = band;
		scoreOverride.score = score;
		return scoreOverride;
	}

	/* 카드의 기본 점수 합 */
	float CardTestScore(const TArray<FCardEntry>& entries, EEnemyCard card, FName attackId = NAME_None)
	{
		float sum = 0.f;
		for (const FCardEntry& entry : entries)
		{
			if (entry.card == card && (attackId.IsNone() || entry.attackId == attackId))
				sum += entry.baseScore;
		}
		return sum;
	}

	float CardTestTotal(const TArray<FCardEntry>& entries)
	{
		float sum = 0.f;
		for (const FCardEntry& entry : entries)
			sum += entry.baseScore;
		return sum;
	}

	/* 기본 성향, 근거리, 가까이, 약공격 1 + 강공격 1 */
	TArray<FCardEntry> CardTestNormalNear()
	{
		const FCardBaseScores scores = CardTestMeleeNear(EEnemyTemperament::ETMP_Normal);
		FCardBaseInput input;
		input.baseScores = &scores;
		input.attacks = { CardTestAttack(TEXT("Light"), EEnemyCard::EC_LightAttack), CardTestAttack(TEXT("Heavy"), EEnemyCard::EC_HeavyAttack) };
		return EnemyCardScore::BuildBaseScores(input);
	}

	FCardModifier CardTestMod(EEnemyCard card, float multiplier)
	{
		FCardModifier modifier;
		modifier.card = card;
		modifier.multiplier = multiplier;
		return modifier;
	}

	FCardModifier CardTestBundleMod(EEnemyCardBundle bundle, float multiplier)
	{
		FCardModifier modifier;
		modifier.bWholeBundle = true;
		modifier.bundle = bundle;
		modifier.multiplier = multiplier;
		return modifier;
	}

	/* 플레이어 헛방 후딜 보정 (약공격 3, 강공격 1, 숨 고르기 0.1) */
	TArray<FCardModifier> CardTestWhiffMods()
	{
		return { CardTestMod(EEnemyCard::EC_LightAttack, 3.f), CardTestMod(EEnemyCard::EC_HeavyAttack, 1.f), CardTestMod(EEnemyCard::EC_Rest, 0.1f) };
	}

	float CardTestFinal(const TArray<FCardEntry>& entries, EEnemyCard card)
	{
		float sum = 0.f;
		for (const FCardEntry& entry : entries)
		{
			if (entry.card == card)
				sum += entry.finalScore;
		}
		return sum;
	}

	float CardTestChance(const TArray<FCardEntry>& entries, const TArray<float>& chances, EEnemyCard card)
	{
		float sum = 0.f;
		for (int32 i = 0; i < entries.Num(); ++i)
		{
			if (entries[i].card == card)
				sum += chances[i];
		}
		return sum;
	}

	float CardTestBundleChance(const TArray<FCardEntry>& entries, const TArray<float>& chances, EEnemyCardBundle bundle)
	{
		float sum = 0.f;
		for (int32 i = 0; i < entries.Num(); ++i)
		{
			if (entries[i].bundle == bundle)
				sum += chances[i];
		}
		return sum;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardDistanceBandTest, "Windless.EnemyAI.Card.DistanceBand",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardDistanceBandTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;
	const FCardBandBounds melee{ 2.5f, 6.f, 0.f, 0.5f };
	const FCardBandBounds ranged{ 3.f, 12.f, 20.f, 0.5f };

	/* 가까이 ~ 중간: 멀어질 때 3.0m, 가까워질 때 2.0m */
	TestTrue(TEXT("가까이에서 2.99m: 가까이"), ResolveBand(2.99f, EEnemyRangeBand::ERB_Near, melee) == EEnemyRangeBand::ERB_Near);
	TestTrue(TEXT("가까이에서 3.0m: 중간"), ResolveBand(3.f, EEnemyRangeBand::ERB_Near, melee) == EEnemyRangeBand::ERB_Mid);
	TestTrue(TEXT("중간에서 2.01m: 중간"), ResolveBand(2.01f, EEnemyRangeBand::ERB_Mid, melee) == EEnemyRangeBand::ERB_Mid);
	TestTrue(TEXT("중간에서 2.0m: 가까이"), ResolveBand(2.f, EEnemyRangeBand::ERB_Mid, melee) == EEnemyRangeBand::ERB_Near);

	/* 중간 ~ 멀리 */
	TestTrue(TEXT("중간에서 6.49m: 중간"), ResolveBand(6.49f, EEnemyRangeBand::ERB_Mid, melee) == EEnemyRangeBand::ERB_Mid);
	TestTrue(TEXT("중간에서 6.5m: 멀리"), ResolveBand(6.5f, EEnemyRangeBand::ERB_Mid, melee) == EEnemyRangeBand::ERB_Far);
	TestTrue(TEXT("멀리에서 5.51m: 멀리"), ResolveBand(5.51f, EEnemyRangeBand::ERB_Far, melee) == EEnemyRangeBand::ERB_Far);
	TestTrue(TEXT("멀리에서 5.5m: 중간"), ResolveBand(5.5f, EEnemyRangeBand::ERB_Far, melee) == EEnemyRangeBand::ERB_Mid);

	/* 여러 구간 건너뛰기 */
	TestTrue(TEXT("가까이에서 10m: 멀리"), ResolveBand(10.f, EEnemyRangeBand::ERB_Near, melee) == EEnemyRangeBand::ERB_Far);
	TestTrue(TEXT("근거리형 100m: 멀리"), ResolveBand(100.f, EEnemyRangeBand::ERB_Far, melee) == EEnemyRangeBand::ERB_Far);
	TestTrue(TEXT("멀리에서 0.5m: 가까이"), ResolveBand(0.5f, EEnemyRangeBand::ERB_Far, melee) == EEnemyRangeBand::ERB_Near);

	/* 원거리형 멀리 ~ 사거리 밖 */
	TestTrue(TEXT("원거리 멀리에서 20.49m: 멀리"), ResolveBand(20.49f, EEnemyRangeBand::ERB_Far, ranged) == EEnemyRangeBand::ERB_Far);
	TestTrue(TEXT("원거리 멀리에서 20.5m: 사거리 밖"), ResolveBand(20.5f, EEnemyRangeBand::ERB_Far, ranged) == EEnemyRangeBand::ERB_OutOfRange);
	TestTrue(TEXT("원거리 사거리 밖에서 19.51m: 사거리 밖"), ResolveBand(19.51f, EEnemyRangeBand::ERB_OutOfRange, ranged) == EEnemyRangeBand::ERB_OutOfRange);
	TestTrue(TEXT("원거리 사거리 밖에서 19.5m: 멀리"), ResolveBand(19.5f, EEnemyRangeBand::ERB_OutOfRange, ranged) == EEnemyRangeBand::ERB_Far);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardBaseScoreTest, "Windless.EnemyAI.Card.BaseScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardBaseScoreTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;

	/* 성향별 가까이 공격 비율 */
	{
		const EEnemyTemperament temperaments[3] = { EEnemyTemperament::ETMP_Calm, EEnemyTemperament::ETMP_Normal, EEnemyTemperament::ETMP_Aggressive };
		const float expected[3] = { 0.45f, 0.55f, 0.65f };
		for (int32 i = 0; i < 3; ++i)
		{
			const FCardBaseScores scores = CardTestMeleeNear(temperaments[i]);
			FCardBaseInput input;
			input.baseScores = &scores;
			input.attacks = { CardTestAttack(TEXT("Light"), EEnemyCard::EC_LightAttack), CardTestAttack(TEXT("Heavy"), EEnemyCard::EC_HeavyAttack) };
			TestEqual(FString::Printf(TEXT("성향 %d 가까이 공격 비율"), i), GetBundleRatio(BuildBaseScores(input), EEnemyCardBundle::ECB_Attack), expected[i], 0.001f);
		}
	}

	/* 늘어진 팔(강공격 1종, 가까이 덮어쓰기, 공격 80%) */
	{
		const FCardBaseScores scores = CardTestMeleeNear(EEnemyTemperament::ETMP_Normal);
		const TArray<FEnemyCardScoreOverride> overrides = {
			CardTestOverride(EEnemyCard::EC_HeavyAttack, EEnemyRangeBand::ERB_Near, 60.f),
			CardTestOverride(EEnemyCard::EC_SideStep, EEnemyRangeBand::ERB_Near, 0.f),
			CardTestOverride(EEnemyCard::EC_BackOff, EEnemyRangeBand::ERB_Near, 0.f),
			CardTestOverride(EEnemyCard::EC_Rest, EEnemyRangeBand::ERB_Near, 15.f),
			CardTestOverride(EEnemyCard::EC_Rest, EEnemyRangeBand::ERB_Mid, 99.f) };
		FCardBaseInput input;
		input.baseScores = &scores;
		input.overrides = &overrides;
		input.attacks = { CardTestAttack(TEXT("DA_Smash"), EEnemyCard::EC_HeavyAttack) };
		const TArray<FCardEntry> entries = BuildBaseScores(input);
		TestEqual(TEXT("늘어진 팔 공격 80%"), GetBundleRatio(entries, EEnemyCardBundle::ECB_Attack), 0.8f, 0.001f);
		TestEqual(TEXT("늘어진 팔 숨 고르기 15 (중간 덮어쓰기 무시)"), CardTestScore(entries, EEnemyCard::EC_Rest), 15.f, 0.001f);
		TestEqual(TEXT("약공격 없음(약공격 카드 없음)"), CardTestScore(entries, EEnemyCard::EC_LightAttack), 0.f, 0.001f);
	}

	/* 일그러진 손(강공격 카드 없음, 공격 43.75%) */
	{
		const FCardBaseScores scores = CardTestMeleeNear(EEnemyTemperament::ETMP_Normal);
		FCardBaseInput input;
		input.baseScores = &scores;
		input.attacks = { CardTestAttack(TEXT("DH_Combo"), EEnemyCard::EC_LightAttack) };
		TestEqual(TEXT("일그러진 손 공격 43.75%"), GetBundleRatio(BuildBaseScores(input), EEnemyCardBundle::ECB_Attack), 0.4375f, 0.001f);
	}

	/* 들개 떼(공격적, 단발 60, 2연타 40, 단발 33.75%, 2연타 22.5%, 공격 56.25%) */
	{
		const FCardBaseScores scores = CardTestMeleeNear(EEnemyTemperament::ETMP_Aggressive);
		FCardBaseInput input;
		input.baseScores = &scores;
		input.attacks = { CardTestAttack(TEXT("Bite"), EEnemyCard::EC_LightAttack, 60.f), CardTestAttack(TEXT("DoubleBite"), EEnemyCard::EC_LightAttack, 40.f) };
		const TArray<FCardEntry> entries = BuildBaseScores(input);
		const float total = CardTestTotal(entries);
		TestEqual(TEXT("들개 단발 33.75%"), CardTestScore(entries, EEnemyCard::EC_LightAttack, FName(TEXT("Bite"))) / total, 0.3375f, 0.001f);
		TestEqual(TEXT("들개 2연타 22.5%"), CardTestScore(entries, EEnemyCard::EC_LightAttack, FName(TEXT("DoubleBite"))) / total, 0.225f, 0.001f);
		TestEqual(TEXT("들개 옆걸음 18.75%"), CardTestScore(entries, EEnemyCard::EC_SideStep) / total, 0.1875f, 0.001f);
		TestEqual(TEXT("들개 뒤로 빠지기 6.25%"), CardTestScore(entries, EEnemyCard::EC_BackOff) / total, 0.0625f, 0.001f);
		TestEqual(TEXT("들개 숨 고르기 18.75%"), CardTestScore(entries, EEnemyCard::EC_Rest) / total, 0.1875f, 0.001f);
	}

	/* 같은 분류 나누기 */
	{
		const FCardBaseScores scores = CardTestMeleeNear(EEnemyTemperament::ETMP_Normal);
		FCardBaseInput twoLights;
		twoLights.baseScores = &scores;
		twoLights.attacks = { CardTestAttack(TEXT("A"), EEnemyCard::EC_LightAttack), CardTestAttack(TEXT("B"), EEnemyCard::EC_LightAttack) };
		TestEqual(TEXT("약공격 2종 균등: 각 17.5"), CardTestScore(BuildBaseScores(twoLights), EEnemyCard::EC_LightAttack, FName(TEXT("A"))), 17.5f, 0.001f);

		FCardBaseInput oneLeft = twoLights;
		oneLeft.attacks = { CardTestAttack(TEXT("A"), EEnemyCard::EC_LightAttack) };
		FCardSpecialOption stomp;
		stomp.attackId = FName(TEXT("Stomp"));
		stomp.score = 40.f;
		oneLeft.specials = { stomp };
		const TArray<FCardEntry> entries = BuildBaseScores(oneLeft);
		TestEqual(TEXT("1종만 남으면 35 전부"), CardTestScore(entries, EEnemyCard::EC_LightAttack, FName(TEXT("A"))), 35.f, 0.001f);
		TestEqual(TEXT("특수 카드 40 그대로"), CardTestScore(entries, EEnemyCard::EC_Special), 40.f, 0.001f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardBaseTableTest, "Windless.EnemyAI.Data.CardBase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardBaseTableTest::RunTest(const FString& Parameters)
{
	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const UDataTable* table = settings ? settings->cardBaseTable.LoadSynchronous() : nullptr;
	if (!table)
	{
		AddError(TEXT("프로젝트 설정에 Card Base Table이 없음"));
		return false;
	}

	/* 역할마다 모든 카드와 구간 검사 */
	const EEnemyCombatRole roles[2] = { EEnemyCombatRole::ECR_Melee, EEnemyCombatRole::ECR_Ranged };
	for (const EEnemyCombatRole role : roles)
	{
		const int32 bandCount = role == EEnemyCombatRole::ECR_Ranged ? 4 : 3;
		for (const EEnemyCard card : EnemyCardScore::GetRoleCards(role))
		{
			for (int32 band = 0; band < bandCount; ++band)
			{
				int32 found = 0;
				table->ForeachRow<FEnemyCardBaseRow>(TEXT("CardBaseTest"), [&](const FName& rowName, const FEnemyCardBaseRow& row)
					{
						if (row.role == role && row.card == card && static_cast<int32>(row.band) == band)
							++found;
					});
				TestEqual(FString::Printf(TEXT("%s %s %s 한 줄"), role == EEnemyCombatRole::ECR_Ranged ? TEXT("원거리") : TEXT("근거리"),
					EnemyCardScore::GetCardName(card), EnemyCardScore::GetBandName(static_cast<EEnemyRangeBand>(band))), found, 1);
			}
		}
	}

	/* 표 합계 확인 */
	float meleeNear = 0.f;
	for (const TPair<EEnemyCard, float>& pair : EnemyCardScore::CollectBaseScores(table, EEnemyCombatRole::ECR_Melee, EEnemyRangeBand::ERB_Near, EEnemyTemperament::ETMP_Normal))
		meleeNear += pair.Value;

	float rangedMid = 0.f;
	for (const TPair<EEnemyCard, float>& pair : EnemyCardScore::CollectBaseScores(table, EEnemyCombatRole::ECR_Ranged, EEnemyRangeBand::ERB_Mid, EEnemyTemperament::ETMP_Normal))
		rangedMid += pair.Value;

	TestEqual(TEXT("근거리 가까이: 기본 합 100"), meleeNear, 100.f, 0.001f);
	TestEqual(TEXT("원거리 중간: 기본 합 100"), rangedMid, 100.f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardModifierTest, "Windless.EnemyAI.Card.Modifiers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardModifierTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;

	/* 기본, 가까이, 헛방 후딜, 약공격: 약 67% */
	{
		TArray<FCardEntry> entries = CardTestNormalNear();
		ApplyModifiers(entries, CardTestWhiffMods());
		ApplyBlocks(entries, FCardBlockInput());
		const TArray<float> chances = GetPickChances(entries, FCardRatioInput());
		TestEqual(TEXT("헛방 후딜 약공격 약 67%"), CardTestChance(entries, chances, EEnemyCard::EC_LightAttack), 0.6709f, 0.001f);
	}

	/* *7.5와 *0.5 -> 5 * 0.5 = 2.5 */
	{
		TArray<FCardEntry> entries = CardTestNormalNear();
		ApplyModifiers(entries, { CardTestMod(EEnemyCard::EC_LightAttack, 2.5f), CardTestMod(EEnemyCard::EC_LightAttack, 3.f), CardTestMod(EEnemyCard::EC_LightAttack, 0.5f) });
		TestEqual(TEXT("*7.5, *0.5 -> 약공격 35 * 2.5"), CardTestFinal(entries, EEnemyCard::EC_LightAttack), 87.5f, 0.001f);
	}

	/* 3.5m 안 회복(2.5) + 헛방(3) -> 5에서 멈춤 */
	{
		TArray<FCardEntry> entries = CardTestNormalNear();
		ApplyModifiers(entries, { CardTestMod(EEnemyCard::EC_LightAttack, 2.5f), CardTestMod(EEnemyCard::EC_LightAttack, 3.f) });
		TestEqual(TEXT("회복 + 헛방: 약공격 35 * 5"), CardTestFinal(entries, EEnemyCard::EC_LightAttack), 175.f, 0.001f);
		const FCardEntry* light = entries.FindByPredicate([](const FCardEntry& entry) { return entry.card == EEnemyCard::EC_LightAttack; });
		TestTrue(TEXT("상한 표시"), light && light->bModifierCapped);
	}

	/* 0이 하나라도 있으면 0 */
	{
		TArray<FCardEntry> entries = CardTestNormalNear();
		ApplyModifiers(entries, { CardTestMod(EEnemyCard::EC_LightAttack, 3.f), CardTestMod(EEnemyCard::EC_LightAttack, 0.f) });
		TestEqual(TEXT("*3 , *0: 0"), CardTestFinal(entries, EEnemyCard::EC_LightAttack), 0.f, 0.001f);
	}

	/* 경직 회복 직후 */
	{
		TArray<FCardEntry> entries = CardTestNormalNear();
		ApplyModifiers(entries, { CardTestMod(EEnemyCard::EC_BackOff, 2.f), CardTestBundleMod(EEnemyCardBundle::ECB_Attack, 0.5f) });
		TestEqual(TEXT("약공격 35: 17.5"), CardTestFinal(entries, EEnemyCard::EC_LightAttack), 17.5f, 0.001f);
		TestEqual(TEXT("강공격 20: 10"), CardTestFinal(entries, EEnemyCard::EC_HeavyAttack), 10.f, 0.001f);
		TestEqual(TEXT("뒤로 빠지기 10: 20"), CardTestFinal(entries, EEnemyCard::EC_BackOff), 20.f, 0.001f);
		TestEqual(TEXT("숨 고르기 15 그대로"), CardTestFinal(entries, EEnemyCard::EC_Rest), 15.f, 0.001f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardBlockTest, "Windless.EnemyAI.Card.Blocks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardBlockTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;

	/* 쉬는 시간 및 플레이어 경직 */
	{
		TArray<FCardEntry> entries = CardTestNormalNear();
		FCardBlockInput rest;
		rest.bRestTime = true;
		ApplyBlocks(entries, rest);
		TestEqual(TEXT("쉬는 시간 약공격 0"), CardTestFinal(entries, EEnemyCard::EC_LightAttack), 0.f, 0.001f);
		TestEqual(TEXT("쉬는 시간 강공격 0"), CardTestFinal(entries, EEnemyCard::EC_HeavyAttack), 0.f, 0.001f);
		TestEqual(TEXT("쉬는 시간 옆걸음 20"), CardTestFinal(entries, EEnemyCard::EC_SideStep), 20.f, 0.001f);

		FCardBlockInput stagger;
		stagger.bPlayerStaggered = true;
		ApplyBlocks(entries, stagger);
		TestEqual(TEXT("플레이어 경직 약공격 0"), CardTestFinal(entries, EEnemyCard::EC_LightAttack), 0.f, 0.001f);
		const FCardEntry* light = entries.FindByPredicate([](const FCardEntry& entry) { return entry.card == EEnemyCard::EC_LightAttack; });
		TestTrue(TEXT("차단 이유 기록"), light && light->blockReason != nullptr);
	}

	/* 반복 억제 */
	{
		TArray<FCardEntry> entries = CardTestNormalNear();
		FCardBlockInput block;
		block.lastPick.bValid = true;
		block.lastPick.card = EEnemyCard::EC_HeavyAttack;
		ApplyBlocks(entries, block);
		TestEqual(TEXT("직전 강공격 20 -> 6"), CardTestFinal(entries, EEnemyCard::EC_HeavyAttack), 6.f, 0.001f);
		TestEqual(TEXT("약공격은 그대로 35"), CardTestFinal(entries, EEnemyCard::EC_LightAttack), 35.f, 0.001f);

		const FCardBaseScores scores = CardTestMeleeNear(EEnemyTemperament::ETMP_Normal);
		FCardBaseInput input;
		input.baseScores = &scores;
		input.attacks = { CardTestAttack(TEXT("A"), EEnemyCard::EC_LightAttack), CardTestAttack(TEXT("B"), EEnemyCard::EC_LightAttack) };
		FCardSpecialOption stomp;
		stomp.attackId = FName(TEXT("Stomp"));
		stomp.score = 40.f;
		FCardSpecialOption shake;
		shake.attackId = FName(TEXT("Shake"));
		shake.score = 80.f;
		input.specials = { stomp, shake };

		TArray<FCardEntry> lights = BuildBaseScores(input);
		FCardBlockInput lightBlock;
		lightBlock.lastPick.bValid = true;
		lightBlock.lastPick.card = EEnemyCard::EC_LightAttack;
		ApplyBlocks(lights, lightBlock);
		TestEqual(TEXT("약공격 2종 모두 억제 (17.5 * 0.3 * 2)"), CardTestFinal(lights, EEnemyCard::EC_LightAttack), 10.5f, 0.001f);

		FCardBlockInput specialBlock;
		specialBlock.lastPick.bValid = true;
		specialBlock.lastPick.card = EEnemyCard::EC_Special;
		specialBlock.lastPick.attackId = FName(TEXT("Stomp"));
		ApplyBlocks(lights, specialBlock);
		TestEqual(TEXT("특수 카드는 고른 것만 억제 (40 * 0.3 + 80)"), CardTestFinal(lights, EEnemyCard::EC_Special), 92.f, 0.001f);
	}

	/* 광증 */
	{
		const FCardBaseScores scores = CardTestMeleeNear(EEnemyTemperament::ETMP_Normal);
		const TArray<FEnemyCardScoreOverride> overrides = {
			CardTestOverride(EEnemyCard::EC_HeavyAttack, EEnemyRangeBand::ERB_Near, 60.f),
			CardTestOverride(EEnemyCard::EC_SideStep, EEnemyRangeBand::ERB_Near, 0.f),
			CardTestOverride(EEnemyCard::EC_BackOff, EEnemyRangeBand::ERB_Near, 0.f),
			CardTestOverride(EEnemyCard::EC_Rest, EEnemyRangeBand::ERB_Near, 15.f) };
		FCardBaseInput input;
		input.baseScores = &scores;
		input.overrides = &overrides;
		input.attacks = { CardTestAttack(TEXT("DA_Smash"), EEnemyCard::EC_HeavyAttack) };
		TArray<FCardEntry> entries = BuildBaseScores(input);

		FCardBlockInput block;
		block.bRestTime = true;
		block.bFrenzy = true;
		ApplyBlocks(entries, block);
		TestEqual(TEXT("광증 숨 고르기 0"), CardTestFinal(entries, EEnemyCard::EC_Rest), 0.f, 0.001f);

		FRandomStream random(1);
		TestEqual(TEXT("전부 0: INDEX_NONE"), PickEntry(entries, FCardRatioInput(), random), static_cast<int32>(INDEX_NONE));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardRatioTest, "Windless.EnemyAI.Card.Ratio",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardRatioTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;

	TArray<FCardEntry> entries = CardTestNormalNear();
	ApplyModifiers(entries, CardTestWhiffMods());
	ApplyBlocks(entries, FCardBlockInput());

	auto attackChance = [&entries](int32 others, bool bPack)
		{
			FCardRatioInput ratio;
			ratio.otherAttackers = others;
			ratio.bPackAttacker = bPack;
			return CardTestBundleChance(entries, GetPickChances(entries, ratio), EEnemyCardBundle::ECB_Attack);
		};

	/* 다른 공격자 */
	TestEqual(TEXT("공격자 0명: 조절 없음"), attackChance(0, false), 0.7987f, 0.001f);
	TestEqual(TEXT("공격자 1명: 조절 없음"), attackChance(1, false), 0.7987f, 0.001f);
	TestEqual(TEXT("공격자 2명: 30%"), attackChance(2, false), 0.30f, 0.001f);
	TestEqual(TEXT("공격자 3명: 20%"), attackChance(3, false), 0.20f, 0.001f);
	TestEqual(TEXT("공격자 5명: 20%"), attackChance(5, false), 0.20f, 0.001f);
	TestEqual(TEXT("무리 공격형 3명: 조절 없음"), attackChance(3, true), 0.7987f, 0.001f);

	/* 묶음 안 상대 비율 */
	FCardRatioInput two;
	two.otherAttackers = 2;
	const TArray<float> chances = GetPickChances(entries, two);
	TestEqual(TEXT("묶음 안 약 : 강 = 5.25"), CardTestChance(entries, chances, EEnemyCard::EC_LightAttack) / CardTestChance(entries, chances, EEnemyCard::EC_HeavyAttack), 5.25f, 0.001f);

	/* 한쪽 묶음이 0이면 조절하지 않음 */
	TArray<FCardEntry> resting = CardTestNormalNear();
	FCardBlockInput rest;
	rest.bRestTime = true;
	ApplyBlocks(resting, rest);
	TestEqual(TEXT("공격 묶음 0, 2명: 간 보기 100%"), CardTestBundleChance(resting, GetPickChances(resting, two), EEnemyCardBundle::ECB_Probe), 1.f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardPickTest, "Windless.EnemyAI.Card.Pick",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardPickTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;

	TArray<FCardEntry> entries = CardTestNormalNear();
	ApplyModifiers(entries, CardTestWhiffMods());
	ApplyBlocks(entries, FCardBlockInput());

	/* 같은 시드 = 같은 결과 */
	FRandomStream a(7);
	FRandomStream b(7);
	bool bSame = true;
	for (int32 i = 0; i < 20; ++i)
		bSame &= PickEntry(entries, FCardRatioInput(), a) == PickEntry(entries, FCardRatioInput(), b);
	TestTrue(TEXT("같은 시드 같은 뽑기"), bSame);

	/* 20000번 뽑으면 약공격 비율이 약 67% */
	FRandomStream random(12345);
	int32 lightCount = 0;
	const int32 drawCount = 20000;
	for (int32 i = 0; i < drawCount; ++i)
	{
		const int32 index = PickEntry(entries, FCardRatioInput(), random);
		if (entries.IsValidIndex(index) && entries[index].card == EEnemyCard::EC_LightAttack)
			++lightCount;
	}
	TestEqual(TEXT("실제 뽑기 약공격 약 67%"), static_cast<float>(lightCount) / drawCount, 0.6709f, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardCandidateTest, "Windless.EnemyAI.Card.Candidates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardCandidateTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;

	FEnemyAttackRow attack;
	attack.hitRanges = { 2.5f };
	attack.minRange = 0.f;
	attack.attackAngleUp = 25.f;
	attack.attackAngleDown = 20.f;

	/* 사거리, 수직 발동 각도 */
	TestTrue(TEXT("정면 2m 같은 높이: 후보"), IsAttackInReach(attack, 2.f, 0.f));
	TestFalse(TEXT("사거리 밖 3m: 후보 아님"), IsAttackInReach(attack, 3.f, 0.f));
	TestFalse(TEXT("지붕 위 (수평 1.5m, 위 3m): 후보 아님"), IsAttackInReach(attack, 1.5f, 3.f));
	TestTrue(TEXT("위 0.5m (수평 2m, 14도): 후보"), IsAttackInReach(attack, 2.f, 0.5f));
	TestFalse(TEXT("아래 1.5m (수평 2m, -36.9도): 후보 아님"), IsAttackInReach(attack, 2.f, -1.5f));

	/* 최소 사거리 */
	attack.minRange = 1.5f;
	TestFalse(TEXT("최소 사거리 안 1.0m: 후보 아님"), IsAttackInReach(attack, 1.f, 0.f));
	TestTrue(TEXT("최소 사거리 1.5m: 후보"), IsAttackInReach(attack, 1.5f, 0.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardPlayerStatesTest, "Windless.EnemyAI.Card.PlayerStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardPlayerStatesTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;

	auto has = [](const FPlayerStateQuery& query, EPlayerAIState state)
		{
			return GetActivePlayerStates(query).Contains(state);
		};

	FPlayerStateQuery query;
	query.attackReach = 2.5f;
	query.distance = 2.f;

	/* 공격 시작 */
	query.player.bAttackBegin = true;
	TestTrue(TEXT("공격 시작, 정면 2m: 걸림"), has(query, EPlayerAIState::EPS_AttackBegin));
	query.distance = 3.1f;
	TestFalse(TEXT("공격 시작, 3.1m: 안 걸림"), has(query, EPlayerAIState::EPS_AttackBegin));
	query.distance = 2.f;
	query.angleFromFront = 61.f;
	TestFalse(TEXT("공격 시작, 61도: 안 걸림"), has(query, EPlayerAIState::EPS_AttackBegin));
	query.angleFromFront = 0.f;
	query.bCounterReaction = false;
	TestFalse(TEXT("공격 시작, 근거리, 카운터 외형 아님: 안 걸림"), has(query, EPlayerAIState::EPS_AttackBegin));
	query.role = EEnemyCombatRole::ECR_Ranged;
	TestTrue(TEXT("공격 시작, 원거리: 걸림"), has(query, EPlayerAIState::EPS_AttackBegin));
	query.role = EEnemyCombatRole::ECR_Melee;
	query.bCounterReaction = true;
	query.player = FPlayerAISnapshot();

	/* 헛방 */
	query.player.bWhiff = true;
	query.distance = 2.5f;
	TestTrue(TEXT("헛방, 사거리 2.5m 안: 걸림"), has(query, EPlayerAIState::EPS_Whiff));
	query.distance = 2.6f;
	TestFalse(TEXT("헛방, 사거리 밖: 안 걸림"), has(query, EPlayerAIState::EPS_Whiff));
	query.player = FPlayerAISnapshot();

	/* 회복이나 아이템 사용 및 활 조준 */
	query.player.bUsingItem = true;
	query.distance = 3.5f;
	TestTrue(TEXT("아이템, 3.5m: 걸림"), has(query, EPlayerAIState::EPS_ItemUse));
	query.distance = 3.6f;
	TestFalse(TEXT("아이템, 3.6m: 안 걸림"), has(query, EPlayerAIState::EPS_ItemUse));
	query.player = FPlayerAISnapshot();
	query.player.bAiming = true;
	query.distance = 8.f;
	TestTrue(TEXT("활 조준, 8m: 걸림"), has(query, EPlayerAIState::EPS_BowAim));
	query.distance = 8.1f;
	TestFalse(TEXT("활 조준, 8.1m: 안 걸림"), has(query, EPlayerAIState::EPS_BowAim));
	query.player = FPlayerAISnapshot();

	/* 가드 2초 */
	query.distance = 2.f;
	query.player.guardHoldTime = 1.9f;
	TestFalse(TEXT("가드 1.9초: 안 걸림"), has(query, EPlayerAIState::EPS_GuardHold));
	query.player.guardHoldTime = 2.f;
	TestTrue(TEXT("가드 2.0초: 걸림"), has(query, EPlayerAIState::EPS_GuardHold));
	query.distance = 3.f;
	TestFalse(TEXT("가드 2.0초, 사거리 밖: 안 걸림"), has(query, EPlayerAIState::EPS_GuardHold));
	query.player = FPlayerAISnapshot();

	/* 뒤에서 접근 */
	query.distance = 2.f;
	query.angleFromFront = 121.f;
	TestTrue(TEXT("등 뒤, 121도, 2m: 걸림"), has(query, EPlayerAIState::EPS_Behind));
	query.angleFromFront = 119.f;
	TestFalse(TEXT("등 뒤, 119도: 안 걸림"), has(query, EPlayerAIState::EPS_Behind));
	query.angleFromFront = 150.f;
	query.distance = 3.1f;
	TestFalse(TEXT("등 뒤, 3.1m: 안 걸림"), has(query, EPlayerAIState::EPS_Behind));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyDataPlayerStateModsTest, "Windless.EnemyAI.Data.PlayerStateMods",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyDataPlayerStateModsTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const UDataTable* cardTable = settings ? settings->cardBaseTable.LoadSynchronous() : nullptr;
	const UDataTable* modTable = settings ? settings->playerStateModTable.LoadSynchronous() : nullptr;
	if (!TestNotNull(TEXT("DT_CardBase"), cardTable) || !TestNotNull(TEXT("DT_PlayerStateMods"), modTable))
		return false;

	/* 헛방, 근거리: 약공격 *3, 강공격 *1, 숨 고르기 *0.1 */
	TArray<FCardModifier> modifiers;
	CollectPlayerStateMods(modTable, EEnemyCombatRole::ECR_Melee, { EPlayerAIState::EPS_Whiff }, modifiers);
	TestEqual(TEXT("헛방, 근거리 보정 3개"), modifiers.Num(), 3);

	/* 기본 성향, 가까이, 헛방 후딜: 약공격 105 / 156.5 = 67.09% */
	const FCardBaseScores baseScores = CollectBaseScores(cardTable, EEnemyCombatRole::ECR_Melee, EEnemyRangeBand::ERB_Near, EEnemyTemperament::ETMP_Normal);
	FCardBaseInput input;
	input.role = EEnemyCombatRole::ECR_Melee;
	input.band = EEnemyRangeBand::ERB_Near;
	input.baseScores = &baseScores;

	FCardAttackOption light;
	light.attackId = TEXT("Light");
	light.card = EEnemyCard::EC_LightAttack;
	input.attacks.Add(light);

	FCardAttackOption heavy;
	heavy.attackId = TEXT("Heavy");
	heavy.card = EEnemyCard::EC_HeavyAttack;
	input.attacks.Add(heavy);

	TArray<FCardEntry> entries = BuildBaseScores(input);
	ApplyModifiers(entries, modifiers);
	ApplyBlocks(entries, FCardBlockInput());
	const TArray<float> chances = GetPickChances(entries, FCardRatioInput());

	const int32 lightIndex = entries.IndexOfByPredicate([](const FCardEntry& entry) { return entry.card == EEnemyCard::EC_LightAttack; });
	if (!TestTrue(TEXT("약공격 줄 있음"), chances.IsValidIndex(lightIndex)))
		return false;

	TestEqual(TEXT("헛방(약공격 약 67%)"), chances[lightIndex], 105.f / 156.5f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyDataLastActModsTest, "Windless.EnemyAI.Data.LastActMods",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyDataLastActModsTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardScore;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const UDataTable* cardTable = settings ? settings->cardBaseTable.LoadSynchronous() : nullptr;
	const UDataTable* lastActTable = settings ? settings->lastActModTable.LoadSynchronous() : nullptr;
	if (!TestNotNull(TEXT("DT_CardBase"), cardTable) || !TestNotNull(TEXT("DT_LastActMods"), lastActTable))
		return false;

	/* 테이블의 모든 직전 행동 검사 */
	const UEnum* actEnum = StaticEnum<EEnemyLastAct>();
	for (int32 i = 0; i < actEnum->NumEnums() - 1; ++i)
	{
		const EEnemyLastAct act = static_cast<EEnemyLastAct>(actEnum->GetValueByIndex(i));
		TArray<FCardModifier> modifiers;
		CollectLastActMods(lastActTable, act, modifiers);
		TestTrue(FString::Printf(TEXT("%s 줄 있음"), GetLastActName(act)), modifiers.Num() > 0);
	}

	/* 경직 회복 직후 */
	TArray<FCardModifier> modifiers;
	CollectLastActMods(lastActTable, EEnemyLastAct::ELA_StaggerRecovered, modifiers);
	TestEqual(TEXT("경직 회복 직후 보정 2개"), modifiers.Num(), 2);

	const FCardBaseScores baseScores = CollectBaseScores(cardTable, EEnemyCombatRole::ECR_Melee, EEnemyRangeBand::ERB_Near, EEnemyTemperament::ETMP_Normal);
	FCardBaseInput input;
	input.role = EEnemyCombatRole::ECR_Melee;
	input.band = EEnemyRangeBand::ERB_Near;
	input.baseScores = &baseScores;

	FCardAttackOption light;
	light.attackId = TEXT("Light");
	light.card = EEnemyCard::EC_LightAttack;
	input.attacks.Add(light);

	FCardAttackOption heavy;
	heavy.attackId = TEXT("Heavy");
	heavy.card = EEnemyCard::EC_HeavyAttack;
	input.attacks.Add(heavy);

	TArray<FCardEntry> entries = BuildBaseScores(input);
	ApplyModifiers(entries, modifiers);

	auto scoreOf = [&entries](EEnemyCard card)
		{
			const FCardEntry* entry = entries.FindByPredicate([card](const FCardEntry& e) { return e.card == card; });
			return entry ? entry->modifiedScore : -1.f;
		};
	TestEqual(TEXT("약공격 35 * 0.5"), scoreOf(EEnemyCard::EC_LightAttack), 17.5f, 0.001f);
	TestEqual(TEXT("강공격 20 * 0.5"), scoreOf(EEnemyCard::EC_HeavyAttack), 10.f, 0.001f);
	TestEqual(TEXT("뒤로 빠지기 10 * 2"), scoreOf(EEnemyCard::EC_BackOff), 20.f, 0.001f);
	TestEqual(TEXT("숨 고르기 그대로"), scoreOf(EEnemyCard::EC_Rest), 15.f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardMovePointsTest, "Windless.EnemyAI.Card.MovePoints",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardMovePointsTest::RunTest(const FString& Parameters)
{
	using namespace EnemyCardMove;

	/* 옆걸음 */
	const FVector center(0.f, 0.f, 0.f);
	const FVector from(300.f, 0.f, 50.f);
	const float quarter = PI * 300.f * 0.5f;
	const FVector clockwise = GetCirclePoint(center, from, 300.f, quarter, 1.f);
	const FVector counter = GetCirclePoint(center, from, 300.f, quarter, -1.f);
	TestEqual(TEXT("시계 방향 1/4 바퀴 X"), clockwise.X, 0.0, 0.5);
	TestEqual(TEXT("시계 방향 1/4 바퀴 Y"), clockwise.Y, 300.0, 0.5);
	TestEqual(TEXT("반시계 방향 1/4 바퀴 Y"), counter.Y, -300.0, 0.5);
	TestEqual(TEXT("높이 유지"), clockwise.Z, 50.0, 0.01);
	TestEqual(TEXT("반지름을 고정값으로 되돌림"),
		FVector::Dist2D(center, GetCirclePoint(center, FVector(200.f, 0.f, 0.f), 300.f, 10.f, 1.f)), 300.0, 0.5);

	/* 물러서기 */
	const FVector backOff = GetBackOffPoint(FVector(0.f, 0.f, 0.f), FVector(100.f, 0.f, 0.f), 250.f);
	TestEqual(TEXT("물러서기 X"), backOff.X, -250.0, 0.01);
	TestEqual(TEXT("물러서기 Y"), backOff.Y, 0.0, 0.01);

	/* 다가가기 */
	TestEqual(TEXT("6m: 달리기 4.2"), GetApproachSpeed(6.f, 5.f, 1.6f, 4.2f), 4.2f);
	TestEqual(TEXT("5m: 걷기 1.6"), GetApproachSpeed(5.f, 5.f, 1.6f, 4.2f), 1.6f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyCardReachTest, "Windless.EnemyAI.Card.Reach",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyCardReachTest::RunTest(const FString& Parameters)
{
	using namespace EnemyReach;

	/* 경로 확인 */
	TestFalse(TEXT("1.5m, 높이 0.5m: 확인 안 함"), ShouldCheckPath(1.5f, 0.5f, 1.8f, 3.f));
	TestTrue(TEXT("6m: 확인"), ShouldCheckPath(6.f, 0.f, 1.8f, 3.f));
	TestTrue(TEXT("바로 위 3.5m: 확인"), ShouldCheckPath(1.f, 3.5f, 1.8f, 3.f));
	TestTrue(TEXT("바로 아래 3.5m: 확인"), ShouldCheckPath(1.f, -3.5f, 1.8f, 3.f));
	TestFalse(TEXT("높이 3.0m는 기준 이하"), ShouldCheckPath(1.f, 3.f, 1.8f, 3.f));

	/* 도달 불가 */
	TestTrue(TEXT("경로 없음"), Judge(false, false, 0.f, 5.f, 3.f) == EEnemyReach::EER_NoPath);
	TestTrue(TEXT("일부 경로"), Judge(true, true, 4.f, 5.f, 3.f) == EEnemyReach::EER_Partial);
	TestTrue(TEXT("직선 3m, 경로 9m: 도달 가능"), Judge(true, false, 9.f, 3.f, 3.f) == EEnemyReach::EER_Reachable);
	TestTrue(TEXT("직선 3m, 경로 9.1m: 너무 돌아감"), Judge(true, false, 9.1f, 3.f, 3.f) == EEnemyReach::EER_TooLong);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS