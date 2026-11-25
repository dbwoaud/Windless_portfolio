#pragma once

#include "CoreMinimal.h"
#include "SoulMarkerTypes.generated.h"

/* 사망 지점에 남은 손실 탈피석 */
USTRUCT(BlueprintType)
struct FSoulMarkerRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly)
	bool bValid = false; // 유효한 손실 지점 여부

	UPROPERTY(SaveGame, BlueprintReadOnly)
	int32 moltStones = 0; // 사망 시점에 잃은 탈피석 양

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FVector location = FVector::ZeroVector; // 사망 지점
};