#pragma once

#include "CoreMinimal.h"
#include "SavePointMenuTypes.generated.h"

/* 세이브포인트 메뉴 페이지 */
UENUM(BlueprintType)
enum class ESavePointMenuPage : uint8
{
	ESMP_None     UMETA(DisplayName = "FieldMenu"),  // 필드 메뉴
	ESMP_Storage  UMETA(DisplayName = "Storage"),    // 창고
	ESMP_Craft    UMETA(DisplayName = "Craft"),      // 제작
	ESMP_Repair   UMETA(DisplayName = "Repair"),     // 수리
	ESMP_Travel   UMETA(DisplayName = "FastTravel"), // 빠른 이동
	ESMP_LevelUp  UMETA(DisplayName = "LevelUp"),    // 레벨업
	ESMP_MAX      UMETA(Hidden)
};