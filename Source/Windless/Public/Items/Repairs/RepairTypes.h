#pragma once

#include "CoreMinimal.h"
#include "Items/ItemTypes.h"
#include "RepairTypes.generated.h"

/* 수리도구 등급별 최대 충전량 */
inline float GetRepairToolMaxCharge(EItemGrade grade)
{
	switch (grade)
	{
	case EItemGrade::EIG_Uncommon:  return 100.f;
	case EItemGrade::EIG_Rare:      return 200.f;
	case EItemGrade::EIG_Unique:    return 400.f;
	case EItemGrade::EIG_Legendary: return 800.f;
	default:                        return 50.f;
	}
}

/* 수리 도구 */
USTRUCT(BlueprintType)
struct FRepairToolEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	int64 instanceUID = 0; // 수리 도구 인스턴스

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	FText toolName; // 표시 이름

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	float currentCharge = 0.f; // 현재 충전량

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	float maxCharge = 0.f; // 최대 충전량
};

/*  장착 장비 */
USTRUCT(BlueprintType)
struct FRepairTargetEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	int64 instanceUID = 0; // 장비 인스턴스

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	EEquipSlot slot = EEquipSlot::EES_None; // 장착 슬롯

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	FText equipmentName; // 표시 이름

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	float currentDurability = 0.f; // 현재 내구도

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	float maxDurability = 0.f; // 최대 내구도

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	float requiredCharge = 0.f; // 완전 수리에 필요한 충전량

	UPROPERTY(BlueprintReadOnly, Category = "Repair")
	bool bBroken = false; // 비활성화 여부
};