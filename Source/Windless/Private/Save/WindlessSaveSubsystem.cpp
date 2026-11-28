#include "Save/WindlessSaveSubsystem.h"
#include "Save/WindlessSaveGame.h"
#include "Components/PlayerSaveComponent.h"
#include "Windless/DebugMacro.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Level/SavePointSubsystem.h"
#include "Level/Landmarks/LandmarkSubsystem.h"
#include "Level/SoulMarkerSubsystem.h"
#include "Items/Storage/SharedStorageSubsystem.h"

const FString UWindlessSaveSubsystem::saveSlotName = TEXT("WindlessSlot0");

bool UWindlessSaveSubsystem::SaveGameData(AActor* player, FName savePointId, FVector respawnLocation, FRotator respawnRotation)
{
	if (!player)
		return false;

	UWindlessSaveGame* save = Cast<UWindlessSaveGame>(UGameplayStatics::CreateSaveGameObject(UWindlessSaveGame::StaticClass()));
	if (!save)
		return false;

	save->saveVersion = currentSaveVersion;
	save->saveTime = FDateTime::Now();
	save->savePointId = savePointId;
	save->savedLocation = respawnLocation;
	save->savedRotation = respawnRotation;

	if (const UWorld* world = GetWorld())
		save->levelName = FName(*world->GetMapName());

	if (const UPlayerSaveComponent* saveTarget = player->FindComponentByClass<UPlayerSaveComponent>())
		saveTarget->CollectSaveData(save->inventory, save->stats);

	save->openedDoors = openedDoors.Array();

	if (const USavePointSubsystem* savePoints = GetSavePointSubsystem(player))
		savePoints->CollectRecord(save->savePoints);

	if (USharedStorageSubsystem* storage = GetStorageSubsystem(player))
		storage->CollectRecord(save->storage);

	if (const ULandmarkSubsystem* landmarks = GetLandmarkSubsystem(player))
		landmarks->CollectRecord(save->landmarks);

	if (const USoulMarkerSubsystem* markers = GetSoulMarkerSubsystem(player))
		markers->CollectRecord(save->soulMarker);

	if (!UGameplayStatics::SaveGameToSlot(save, saveSlotName, saveUserIndex))
		return false;
	
	loadedSave = save;
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("[세이브] 저장되었습니다"));

	onGameSaved.Broadcast();
	return true;
}

bool UWindlessSaveSubsystem::LoadGameData()
{
	if (!HasSaveData())
		return false;

	UWindlessSaveGame* save = Cast<UWindlessSaveGame>(UGameplayStatics::LoadGameFromSlot(saveSlotName, saveUserIndex));

	if (!save)
		return false;
	
	if (save->saveVersion > currentSaveVersion)
	{
		UE_LOG(LogWindless, Error, TEXT("[세이브] 지원하지 않는 세이브 버전 %d (현재 %d)"), save->saveVersion, currentSaveVersion);
		return false;
	}
	
	if (save->saveVersion < currentSaveVersion)
		MigrateSave(*save);

	loadedSave = save;
	UE_LOG(LogWindless, Log, TEXT("[세이브] 불러오기 완료 (%s)"), *save->saveTime.ToString());
	return true;
}

bool UWindlessSaveSubsystem::ApplyToPlayer(AActor* player)
{
	if (!player || !loadedSave)
		return false;

	if (USavePointSubsystem* savePoints = GetSavePointSubsystem(player))
		savePoints->ApplyRecord(loadedSave->savePoints);

	if (USharedStorageSubsystem* storage = GetStorageSubsystem(player))
		storage->ApplyRecord(loadedSave->storage);

	if (ULandmarkSubsystem* landmarks = GetLandmarkSubsystem(player))
		landmarks->ApplyRecord(loadedSave->landmarks);

	if (USoulMarkerSubsystem* markers = GetSoulMarkerSubsystem(player))
		markers->ApplyRecord(loadedSave->soulMarker);

	if (UPlayerSaveComponent* saveTarget = player->FindComponentByClass<UPlayerSaveComponent>())
		saveTarget->ApplySaveData(loadedSave->inventory, loadedSave->stats);

	openedDoors = TSet<FName>(loadedSave->openedDoors);

	onGameLoaded.Broadcast();
	return true;
}

bool UWindlessSaveSubsystem::TryContinue(AActor* player)
{
	if (!LoadGameData())
		return false;

	return ApplyToPlayer(player);
}

bool UWindlessSaveSubsystem::GetLoadedRespawnTransform(FVector& outLocation, FRotator& outRotation) const
{
	if (!loadedSave || loadedSave->savePointId.IsNone())
		return false;

	outLocation = loadedSave->savedLocation;
	outRotation = loadedSave->savedRotation;
	return true;
}

bool UWindlessSaveSubsystem::HasSaveData() const
{
	return UGameplayStatics::DoesSaveGameExist(saveSlotName, saveUserIndex);
}

bool UWindlessSaveSubsystem::DeleteSaveData()
{
	loadedSave = nullptr;
	openedDoors.Empty();

	if (USavePointSubsystem* savePoints = GetSavePointSubsystem(this))
		savePoints->ResetProgress();

	if (USharedStorageSubsystem* storage = GetStorageSubsystem(this))
		storage->ResetStorage();
	
	if (ULandmarkSubsystem* landmarks = GetLandmarkSubsystem(this))
		landmarks->ResetProgress();

	if (USoulMarkerSubsystem* markers = GetSoulMarkerSubsystem(this))
		markers->ResetProgress();

	if (!HasSaveData())
		return false;

	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(saveSlotName, saveUserIndex);
	return bDeleted;
}

void UWindlessSaveSubsystem::MarkDoorOpened(FName doorId)
{
	if (doorId.IsNone() || openedDoors.Contains(doorId))
		return;

	openedDoors.Add(doorId);

	// 쉬기 전에 죽거나 꺼도 지름길이 남게, 세이브 파일이 있으면 문만 더해 바로 적는다
	if (!loadedSave && HasSaveData())
		LoadGameData();
	const bool bWritten = loadedSave && loadedSave->openedDoors.AddUnique(doorId) >= 0
		&& UGameplayStatics::SaveGameToSlot(loadedSave, saveSlotName, saveUserIndex);
	UE_LOG(LogWindless, Log, TEXT("[세이브] 문 열림 기록: %s (%s)"), *doorId.ToString(),
		bWritten ? TEXT("파일에 적음") : TEXT("세이브 파일이 없어 다음 저장 때 적힘"));
}

FName UWindlessSaveSubsystem::GetLoadedSavePointId() const
{
	return loadedSave ? loadedSave->savePointId : NAME_None;
}

USharedStorageSubsystem* UWindlessSaveSubsystem::GetStorageSubsystem(const UObject* worldContext)
{
	UWorld* world = worldContext ? worldContext->GetWorld() : nullptr;
	return world ? world->GetSubsystem<USharedStorageSubsystem>() : nullptr;
}

void UWindlessSaveSubsystem::PrintSaveInfo()
{
	if (!HasSaveData())
	{
		UE_LOG(LogWindless, Warning, TEXT("[세이브] 저장된 슬롯이 없습니다"));
		return;
	}

	UWindlessSaveGame* save = Cast<UWindlessSaveGame>(UGameplayStatics::LoadGameFromSlot(saveSlotName, saveUserIndex));
	if (!save)
	{
		UE_LOG(LogWindless, Error, TEXT("[세이브] 슬롯 읽기 실패 - 파일 손상 가능성"));
		return;
	}

	UE_LOG(LogWindless, Warning, TEXT("========== [세이브 슬롯 정보] =========="));
	UE_LOG(LogWindless, Log, TEXT("  버전 %d / 저장 시각 %s"), save->saveVersion, *save->saveTime.ToString());
	UE_LOG(LogWindless, Log, TEXT("  레벨 %s / 세이브포인트 %s %s"),
		*save->levelName.ToString(), *save->savePointId.ToString(), *save->savedLocation.ToCompactString());
	UE_LOG(LogWindless, Log, TEXT("  허물 %d / 탈피석 %d (다음 탈피 %d)"),
		save->stats.currentLevel, save->stats.moltStones, save->stats.moltCost);
	UE_LOG(LogWindless, Log, TEXT("  다음 UID %lld / 그리드 %dx%d"),
		save->inventory.nextInstanceUID, save->inventory.gridWidth, save->inventory.gridHeight);
	UE_LOG(LogWindless, Log, TEXT("  --- 인스턴스 %d개 ---"), save->inventory.items.Num());

	for (const FItemInstance& inst : save->inventory.items)
	{
		const FString placeInfo = inst.bIsEquipped
			? FString::Printf(TEXT("[장착 슬롯%d]"), (int32)inst.equipSlot)
			: FString::Printf(TEXT("(%d,%d)%s"), inst.gridPosX, inst.gridPosY, inst.bIsRotated ? TEXT(" R") : TEXT(""));

		UE_LOG(LogWindless, Log, TEXT("   UID %-4lld %-28s x%-3d 내구 %-6.1f %s"),
			inst.instanceUID, *inst.itemId.ToString(), inst.currentStackCount, inst.currentDurability, *placeInfo);
	}

	UE_LOG(LogWindless, Log, TEXT("  --- 활성 세이브포인트 %d곳 ---"), save->savePoints.activatedSavePoints.Num());
	for (const FSavePointRecord& record : save->savePoints.activatedSavePoints)
		UE_LOG(LogWindless, Log, TEXT("   %s %s"), *record.savePointId.ToString(), *record.respawnLocation.ToCompactString());

	UE_LOG(LogWindless, Log, TEXT("  --- 창고 %s : 아이템 %d개 ---"),
		*GetStorageGradeName(save->storage.grade).ToString(), save->storage.inventory.items.Num());

	UE_LOG(LogWindless, Log, TEXT("  --- 발견 랜드마크 %d곳 ---"), save->landmarks.discoveredLandmarks.Num());

	UE_LOG(LogWindless, Log, TEXT("  --- 열어 둔 문 %d곳 ---"), save->openedDoors.Num());

	if (save->soulMarker.bValid)
		UE_LOG(LogWindless, Log, TEXT("  --- 손실 지점: 탈피석 %d ---"), save->soulMarker.moltStones);

	UE_LOG(LogWindless, Warning, TEXT("======================================"));
}

void UWindlessSaveSubsystem::MigrateSave(UWindlessSaveGame& save)
{
	if (save.saveVersion < 3 && save.savePoints.activatedSavePoints.IsEmpty()
		&& !save.savePointId.IsNone() && save.savePointId != FName(TEXT("Debug")))
	{
		FSavePointRecord record;
		record.savePointId = save.savePointId;
		record.respawnLocation = save.savedLocation;
		record.respawnRotation = save.savedRotation;
		save.savePoints.activatedSavePoints.Add(record);
	}

	if (save.saveVersion < 3 && save.storage.inventory.gridWidth == 0)
	{
		save.storage.grade = EStorageGrade::ESG_Small;
		save.storage.inventory.gridWidth = StorageGridWidth;
		save.storage.inventory.gridHeight = GetStorageGradeHeight(EStorageGrade::ESG_Small);
	}

	save.saveVersion = currentSaveVersion;
}

USavePointSubsystem* UWindlessSaveSubsystem::GetSavePointSubsystem(const UObject* worldContext)
{
	UWorld* world = worldContext ? worldContext->GetWorld() : nullptr;
	return world ? world->GetSubsystem<USavePointSubsystem>() : nullptr;
}

USoulMarkerSubsystem* UWindlessSaveSubsystem::GetSoulMarkerSubsystem(const UObject* worldContext)
{
	UWorld* world = worldContext ? worldContext->GetWorld() : nullptr;
	return world ? world->GetSubsystem<USoulMarkerSubsystem>() : nullptr;
}

ULandmarkSubsystem* UWindlessSaveSubsystem::GetLandmarkSubsystem(const UObject* worldContext)
{
	UWorld* world = worldContext ? worldContext->GetWorld() : nullptr;
	return world ? world->GetSubsystem<ULandmarkSubsystem>() : nullptr;
}
