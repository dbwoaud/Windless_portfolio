#pragma once

#include "CoreMinimal.h"

struct FMonsterArchetypeRow;

/* 약점 지정 규칙 */
struct FWeakPointRule
{
	bool bFixed = false;          // 고정 약점 여부
	TArray<FName> fixedNames;     // 고정 약점 부위 이름
	TArray<FName> candidateNames; // 무작위 후보 부위 이름
	int32 minCount = 1;           // 무작위 약점 최소 개수
	int32 maxCount = 2;           // 무작위 약점 최대 개수
};

/* 약점 지정 순수 함수 */
namespace WindlessWeakPoint
{
	// 원형 데이터를 약점 규칙으로 바꾸는 함수
	WINDLESS_API FWeakPointRule MakeArchetypeRule(const FMonsterArchetypeRow& archetype);

	// 규칙대로 약점 부위 인덱스를 고르는 함수
	WINDLESS_API TArray<int32> PickRegions(const TArray<FName>& regionNames, const FWeakPointRule& rule, FRandomStream& random, TArray<FName>* outMissing = nullptr);
}