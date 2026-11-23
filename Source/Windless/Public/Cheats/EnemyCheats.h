#pragma once

#include "CoreMinimal.h"
#include "Cheats/WindlessCheatExtension.h"
#include "EnemyCheats.generated.h"

/*
 * 적, 적 AI 콘솔 치트
 */
UCLASS()
class WINDLESS_API UEnemyCheats : public UWindlessCheatExtension
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void EnmSpawners(); // 적 스포너 상태를 출력하는 함수

	UFUNCTION(Exec)
	void EnmRespawnEnemies(bool bIncludeAlive = false); // 등록된 스포너의 적을 재생성하는 함수

	UFUNCTION(Exec)
	void EnmData(); // 적 AI 데이터 테이블을 검사하고 가장 가까운 적의 프로필을 출력하는 함수

	UFUNCTION(Exec)
	void EnmForceCard(const FString& cardName); // 모든 적의 다음 카드 선택을 강제하는 함수
};