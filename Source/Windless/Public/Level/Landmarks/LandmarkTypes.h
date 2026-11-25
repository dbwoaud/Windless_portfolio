#pragma once

#include "CoreMinimal.h"
#include "LandmarkTypes.generated.h"

/* 랜드마크 */
USTRUCT(BlueprintType)
struct FLandmarkRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FName landmarkId = NAME_None; // 랜드마크 고유 ID

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FText displayName; // 지도 표시용 이름

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FVector location = FVector::ZeroVector; // 지도 좌표 계산용 위치
};

/* 랜드마크 진행 저장 */
USTRUCT()
struct FLandmarkProgressRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FLandmarkRecord> discoveredLandmarks; // 발견한 랜드마크 목록
};