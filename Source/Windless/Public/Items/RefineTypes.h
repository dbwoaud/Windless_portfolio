#pragma once

#include "CoreMinimal.h"
#include "Items/ItemTypes.h"

/* 등급별 탈피석 환산값 */
inline int32 GetRefineValueForGrade(EItemGrade grade)
{
	switch (grade)
	{
	case EItemGrade::EIG_Uncommon:  return 15;
	case EItemGrade::EIG_Rare:      return 40;
	case EItemGrade::EIG_Unique:    return 100;
	case EItemGrade::EIG_Legendary: return 250;
	default:                        return 5;
	}
}
