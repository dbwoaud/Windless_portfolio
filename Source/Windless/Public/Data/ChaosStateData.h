#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ChaosStateData.generated.h"

USTRUCT(BlueprintType)
struct FChaosStateData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float staminaLockRatio = 0.f; // 스태미나 잠금 비율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float staminaRegenDelayOverride = -1.f; // 스태미나 회복 대기시간 오버라이드(음수면 기본 staminaRegenDelay 사용)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float castSpeedMultiplier = 1.f; // 아이템 시전 속도 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float injuryChanceReduction = 0.f; // 부상 발동 확률 감소량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float weaknessDamageMultiplier = 1.f; // 약점 공격 대미지 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    bool bLockOnInputBlocked = false; // 락온 입력 차단 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float lockOnBreakInterval = 0.f; // 락온 강제 해제 판정 주기

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float lockOnBreakChance = 0.f; // 락온 강제 해제 확률

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
    float lockOnShakeInterval = 0.f; // 락온 중 카메라 흔들림 주기
};