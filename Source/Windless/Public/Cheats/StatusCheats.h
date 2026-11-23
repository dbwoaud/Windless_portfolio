#pragma once

#include "CoreMinimal.h"
#include "Cheats/WindlessCheatExtension.h"
#include "StatusCheats.generated.h"

/*
 * 스테이터스, 부위, 혼돈, 탈피 콘솔 치트
 */

UCLASS()
class WINDLESS_API UStatusCheats : public UWindlessCheatExtension
{
	GENERATED_BODY()

public:
	/* 스테이터스 */
	UFUNCTION(Exec)
	void StaAttribute(); // 스테이터스와 부위별 체력을 출력하는 함수


	/* 부위 시스템 */
	UFUNCTION(Exec)
	void StaHurt(int32 part, float amount); // 부위에 대미지를 입히는 함수

	UFUNCTION(Exec)
	void StaInjure(int32 part, int32 injury); // 부위에 부상을 발동시키는 함수

	UFUNCTION(Exec)
	void StaSaveZone(bool bActive); // 세이브 포인트 구역 상태를 토글하는 함수


	/* 혼돈 시스템 */
	UFUNCTION(Exec)
	void WLChaos(float value); // 혼돈 수치를 지정하는 함수


	/* 탈피(레벨업) */
	UFUNCTION(Exec)
	void WLMolt(); // 탈피를 시도하는 함수

	UFUNCTION(Exec)
	void WLAddStone(int32 amount); // 탈피석을 획득하는 함수
};