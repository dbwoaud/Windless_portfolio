#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/WeakPointRoll.h"
#include "Enemy/AI/MonsterArchetypeRow.h"

namespace
{
	/* 시험용 부위 목록 */
	const TArray<FName>& WeakPointTestRegions()
	{
		static const TArray<FName> regions = { TEXT("Head"), TEXT("Torso"), TEXT("LeftArm"), TEXT("RightArm"), TEXT("LeftLeg"), TEXT("RightLeg") };
		return regions;
	}

	/* 시험용 원형을 만드는 함수 */
	FMonsterArchetypeRow WeakPointTestArchetype(EMonsterGrade grade, int32 count, const TArray<FName>& candidates, const TArray<FName>& fixedNames)
	{
		FMonsterArchetypeRow row;
		row.grade = grade;
		row.weakPointCount = count;
		row.weakPointCandidates = candidates;
		row.fixedWeakPoints = fixedNames;
		return row;
	}

	/* 고른 결과를 정렬된 문자열로 바꾸는 함수 */
	FString WeakPointTestKey(const TArray<int32>& picked)
	{
		TArray<int32> sorted = picked;
		sorted.Sort();

		FString key;
		for (const int32 index : sorted)
			key += WeakPointTestRegions()[index].ToString() + TEXT("|");
		return key;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyWeakPointRollTest, "Windless.EnemyAI.Hit.WeakPoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	bool FEnemyWeakPointRollTest::RunTest(const FString& Parameters)
{
	const TArray<FName>& regions = WeakPointTestRegions();

	/* 일반(후보 3개 중 2개, 후보 밖 없음, 중복 없음, 재지정시 변경) */
	{
		const FWeakPointRule rule = WindlessWeakPoint::MakeArchetypeRule(
			WeakPointTestArchetype(EMonsterGrade::EGRD_Normal, 2, { TEXT("Head"), TEXT("Torso"), TEXT("LeftArm") }, {}));
		TestFalse(TEXT("일반 = 무작위"), rule.bFixed);

		TSet<FString> distinct;
		bool bAllValid = true;
		for (int32 seed = 1; seed <= 50; ++seed)
		{
			FRandomStream random(seed);
			const TArray<int32> picked = WindlessWeakPoint::PickRegions(regions, rule, random);
			bAllValid &= picked.Num() == 2 && picked[0] != picked[1];
			for (const int32 index : picked)
				bAllValid &= index >= 0 && index <= 2;
			distinct.Add(WeakPointTestKey(picked));
		}
		TestTrue(TEXT("일반: 매번 후보 안에서 서로 다른 2개"), bAllValid);
		TestTrue(TEXT("다시 지정하면 약점이 바뀐다 (조합 2가지 이상)"), distinct.Num() >= 2);
	}

	/* 같은 시드 = 같은 결과 */
	{
		const FWeakPointRule rule = WindlessWeakPoint::MakeArchetypeRule(WeakPointTestArchetype(EMonsterGrade::EGRD_Elite, 3, {}, {}));
		FRandomStream a(7);
		FRandomStream b(7);
		TestEqual(TEXT("같은 시드 같은 약점"), WeakPointTestKey(WindlessWeakPoint::PickRegions(regions, rule, a)),
			WeakPointTestKey(WindlessWeakPoint::PickRegions(regions, rule, b)));
	}

	/* 후보가 비면 모든 부위, 개수가 후보보다 많으면 후보 수까지, 0개 */
	{
		FRandomStream random(3);
		const TArray<int32> all = WindlessWeakPoint::PickRegions(regions,
			WindlessWeakPoint::MakeArchetypeRule(WeakPointTestArchetype(EMonsterGrade::EGRD_Normal, 2, {}, {})), random);
		TestEqual(TEXT("후보 없음 → 전체에서 2개"), all.Num(), 2);

		const TArray<int32> clamped = WindlessWeakPoint::PickRegions(regions,
			WindlessWeakPoint::MakeArchetypeRule(WeakPointTestArchetype(EMonsterGrade::EGRD_Normal, 5, { TEXT("Head"), TEXT("Torso"), TEXT("LeftLeg") }, {})), random);
		TestEqual(TEXT("개수 5 · 후보 3 → 3개"), clamped.Num(), 3);

		const TArray<int32> none = WindlessWeakPoint::PickRegions(regions,
			WindlessWeakPoint::MakeArchetypeRule(WeakPointTestArchetype(EMonsterGrade::EGRD_Normal, 0, {}, {})), random);
		TestEqual(TEXT("개수 0 → 없음"), none.Num(), 0);
	}

	/* 보스(고정 약점 그대로, 없는 이름 보고) */
	{
		const FWeakPointRule rule = WindlessWeakPoint::MakeArchetypeRule(
			WeakPointTestArchetype(EMonsterGrade::EGRD_FieldBoss, 2, { TEXT("Head") }, { TEXT("Torso"), TEXT("Tail") }));
		TestTrue(TEXT("보스 = 고정"), rule.bFixed);

		bool bSameEverySeed = true;
		TArray<FName> missing;
		for (int32 seed = 1; seed <= 10; ++seed)
		{
			FRandomStream random(seed);
			const TArray<int32> picked = WindlessWeakPoint::PickRegions(regions, rule, random, &missing);
			bSameEverySeed &= picked.Num() == 1 && picked[0] == 1; // Torso
		}
		TestTrue(TEXT("보스: 시드와 상관없이 Torso"), bSameEverySeed);
		TestTrue(TEXT("없는 고정 부위 Tail 보고"), missing.Contains(FName(TEXT("Tail"))));
	}

	/* 후보에 없는 이름은 빼고 보고 */
	{
		FRandomStream random(5);
		TArray<FName> missing;
		const TArray<int32> picked = WindlessWeakPoint::PickRegions(regions,
			WindlessWeakPoint::MakeArchetypeRule(WeakPointTestArchetype(EMonsterGrade::EGRD_Normal, 2, { TEXT("Head"), TEXT("Wing") }, {})), random, &missing);
		TestEqual(TEXT("있는 후보 Head 1개만"), picked.Num(), 1);
		TestTrue(TEXT("없는 후보 Wing 보고"), missing.Contains(FName(TEXT("Wing"))));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS