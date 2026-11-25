#include "Components/WeakPointRoll.h"
#include "Enemy/AI/MonsterArchetypeRow.h"

FWeakPointRule WindlessWeakPoint::MakeArchetypeRule(const FMonsterArchetypeRow& archetype)
{
	FWeakPointRule rule;
	rule.bFixed = archetype.grade == EMonsterGrade::EGRD_FieldBoss || archetype.grade == EMonsterGrade::EGRD_MainBoss;
	rule.fixedNames = archetype.fixedWeakPoints;
	rule.candidateNames = archetype.weakPointCandidates;
	rule.minCount = archetype.weakPointCount;
	rule.maxCount = archetype.weakPointCount;
	return rule;
}

TArray<int32> WindlessWeakPoint::PickRegions(const TArray<FName>& regionNames, const FWeakPointRule& rule, FRandomStream& random, TArray<FName>* outMissing)
{
	TArray<int32> picked;
	auto addByName = [&regionNames, outMissing](const FName& name, TArray<int32>& outIndices)
		{
			const int32 index = regionNames.IndexOfByKey(name);
			if (index != INDEX_NONE)
				outIndices.AddUnique(index);
			else if (outMissing)
				outMissing->AddUnique(name);
		};

	/* 고정 약점 */
	if (rule.bFixed)
	{
		for (const FName& name : rule.fixedNames)
			addByName(name, picked);
		return picked;
	}

	/* 무작위 약점 */
	TArray<int32> pool;
	if (rule.candidateNames.Num() == 0)
	{
		for (int32 i = 0; i < regionNames.Num(); ++i)
			pool.Add(i);
	}
	else
	{
		for (const FName& name : rule.candidateNames)
			addByName(name, pool);
	}

	const int32 maxCount = FMath::Max(0, rule.maxCount);
	const int32 minCount = FMath::Clamp(rule.minCount, 0, maxCount);
	const int32 count = FMath::Min(random.RandRange(minCount, maxCount), pool.Num());

	for (int32 n = 0; n < count; ++n)
	{
		const int32 pick = random.RandRange(0, pool.Num() - 1);
		picked.Add(pool[pick]);
		pool.RemoveAtSwap(pick);
	}
	return picked;
}