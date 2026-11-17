#pragma once

#include "CoreMinimal.h"
#include "WeaponType.generated.h"

/* 무기 타입 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_OneHanded UMETA(DisplayName = "One-Handed"), // 한손무기
    EWT_Shield    UMETA(DisplayName = "Shield"),     // 방패
    EWT_TwoHanded UMETA(DisplayName = "Two-Handed"), // 양손무기
    EWT_Bow       UMETA(DisplayName = "Bow")         // 활
};