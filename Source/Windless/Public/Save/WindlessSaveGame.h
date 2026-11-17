#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Save/WindlessSaveTypes.h"
#include "Level/StorageTypes.h"
#include "Level/Landmarks/LandmarkTypes.h"
#include "Level/SoulMarkerTypes.h"
#include "WindlessSaveGame.generated.h"

UCLASS()
class WINDLESS_API UWindlessSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/* 메타 정보 */
	UPROPERTY(SaveGame)
	int32 saveVersion = 1; // 세이브 버전

	UPROPERTY(SaveGame)
	FDateTime saveTime; // 저장 시각

	UPROPERTY(SaveGame)
	FName levelName = NAME_None; // 저장 당시 레벨 이름

	UPROPERTY(SaveGame)
	FName savePointId = NAME_None; // 저장한 세이브 포인트 ID

	UPROPERTY(SaveGame)
	FVector savedLocation = FVector::ZeroVector; // 저장 당시 부활 위치

	UPROPERTY(SaveGame)
	FRotator savedRotation = FRotator::ZeroRotator; // 저장 당시 부활 회전


	/* 게임 데이터 */
	UPROPERTY(SaveGame)
	FPlayerStatSaveRecord stats; // 캐릭터 스테이터스

	UPROPERTY(SaveGame)
	FInventorySaveRecord inventory; // 인벤토리 상태

	UPROPERTY(SaveGame)
	FStorageSaveRecord storage; // 창고 상태

	UPROPERTY(SaveGame)
	FSavePointProgressRecord savePoints; // 활성화된 세이브포인트 목록

	UPROPERTY(SaveGame)
	FLandmarkProgressRecord landmarks; // 발견된 랜드마크

	UPROPERTY(SaveGame)
	TArray<FName> openedDoors; // 열어 둔 지름길 문 ID (ABaseDoor::doorId)

	UPROPERTY(SaveGame)
	FSoulMarkerRecord soulMarker; // 손실 탈피석 정보 
};