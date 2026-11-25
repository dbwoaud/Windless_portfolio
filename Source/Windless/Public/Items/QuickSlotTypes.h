#pragma once

#include "CoreMinimal.h"
#include "QuickSlotTypes.generated.h"

UENUM(BlueprintType)
enum class EQuickSlot : uint8
{
	EQS_None      UMETA(Hidden),
	EQS_Recovery  UMETA(DisplayName = "Recovery"),  // 회복 아이템 전용 슬롯
	EQS_Consume1  UMETA(DisplayName = "Consume1"),  // 소모품 슬롯 1
	EQS_Consume2  UMETA(DisplayName = "Consume2"),  // 소모품 슬롯 2
	EQS_Consume3  UMETA(DisplayName = "Consume3"),  // 소모품 슬롯 3
	EQS_Consume4  UMETA(DisplayName = "Consume4"),  // 소모품 슬롯 4
	EQS_MAX       UMETA(Hidden)
};
