#pragma once

#include "CoreMinimal.h"
#include "Cheats/WindlessCheatExtension.h"
#include "CombatCheats.generated.h"

/*
 * 플레이어 전투 콘솔 치트
 */

UCLASS()
class WINDLESS_API UCombatCheats : public UWindlessCheatExtension
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void WLCombat(); // 전투 상태와 무기별 공격력을 출력하는 함수

	UFUNCTION(Exec)
	void WLStagger(); // 플레이어에게 강한 경직을 적용하는 함수

	UFUNCTION(Exec)
	void WLSignal(const FString& signalName, float duration); // 적 AI에게 플레이어 행동 신호를 보내는 함수
};