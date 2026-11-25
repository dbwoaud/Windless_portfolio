#pragma once

#include "CoreMinimal.h"

/* 레벨당 스탯 증가치 */
namespace WindlessLevelUp
{
	static constexpr float HeadHealthPerLevel = 2.f; // 생명력(머리)
	static constexpr float ChestHealthPerLevel = 5.f; // 생명력(가슴)
	static constexpr float LimbHealthPerLevel = 3.f; // 생명력(양팔/양다리)
	static constexpr float StaminaPerLevel = 5.f; // 스태미나
	static constexpr float AttackPowerPerLevel = 2.f; // 공격력
	static constexpr float WillPowerPerLevel = 2.f; // 정신력 

	/* 레벨에 따른 개인 인벤토리 크기 */
	inline void GetInventorySizeForLevel(int32 level, int32& outWidth, int32& outHeight)
	{
		outWidth = 6 + FMath::Max(0, level - 1) / 15;  // 15레벨마다 +1
		outHeight = 6 + FMath::Max(0, level - 1) / 5; // 5레벨마다 +1
	}
}