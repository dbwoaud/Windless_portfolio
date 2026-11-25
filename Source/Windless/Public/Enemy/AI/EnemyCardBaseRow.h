#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Enemy/AI/EnemyAITypes.h"
#include "EnemyCardBaseRow.generated.h"

USTRUCT(BlueprintType)
struct WINDLESS_API FEnemyCardBaseRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyCombatRole role = EEnemyCombatRole::ECR_Melee; // 근거리 / 원거리 카드 표

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyCard card = EEnemyCard::EC_Rest; // 카드

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyRangeBand band = EEnemyRangeBand::ERB_Near; // 거리 구간

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score", meta = (ClampMin = "0.0"))
	float scoreCalm = 0.f; // 차분 성향 점수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score", meta = (ClampMin = "0.0"))
	float scoreNormal = 0.f; // 기본 성향 점수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Score", meta = (ClampMin = "0.0"))
	float scoreAggressive = 0.f; // 공격적 성향 점수

	float GetScore(EEnemyTemperament temperament) const // 성향에 맞는 점수를 반환하는 함수
	{
		switch (temperament)
		{
		case EEnemyTemperament::ETMP_Calm:       return scoreCalm;
		case EEnemyTemperament::ETMP_Aggressive: return scoreAggressive;
		default:                                 return scoreNormal;
		}
	}
};