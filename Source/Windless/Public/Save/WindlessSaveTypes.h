#pragma once

#include "CoreMinimal.h"
#include "Items/ItemTypes.h"
#include "WindlessSaveTypes.generated.h"

/* 활성화 세이브포인트 레코드 */
USTRUCT(BlueprintType)
struct FSavePointRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FName savePointId = NAME_None; // 세이브포인트 고유 ID

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FText displayName; // 지도, 메뉴 표시용 이름

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FVector respawnLocation = FVector::ZeroVector; // 부활, 빠른 이동 위치

	UPROPERTY(SaveGame, BlueprintReadOnly)
	FRotator respawnRotation = FRotator::ZeroRotator; // 부활, 빠른 이동 회전

	UPROPERTY(SaveGame, BlueprintReadOnly)
	int32 activationOrder = 0; // 활성화 번호
};

/* 세이브포인트 진행 저장 레코드 */
USTRUCT()
struct FSavePointProgressRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FSavePointRecord> activatedSavePoints; // 활성화된 세이브포인트 목록
};

/* 인벤토리 저장 레코드 */
USTRUCT()
struct FInventorySaveRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FItemInstance> items; // 보유 아이템 인스턴스 목록

	UPROPERTY(SaveGame)
	int64 nextInstanceUID = 1; // 다음 발급할 인스턴스 UID

	UPROPERTY(SaveGame)
	int32 gridWidth = 0; // 저장 당시 그리드 가로 크기

	UPROPERTY(SaveGame)
	int32 gridHeight = 0; // 저장 당시 그리드 세로 크기
};

/* 캐릭터 스테이터스 저장 레코드 */
USTRUCT()
struct FPlayerStatSaveRecord
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int32 currentLevel = 1; // 현재 허물

	UPROPERTY(SaveGame)
	int32 moltStones = 0; // 보유 탈피석

	UPROPERTY(SaveGame)
	int32 moltCost = 200; // 다음 탈피 비용

	UPROPERTY(SaveGame)
	float bodyHeat = 36.f; // 체온
};