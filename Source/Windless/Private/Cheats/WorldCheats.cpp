#include "Cheats/WorldCheats.h"
#include "Components/PlayerSaveComponent.h" 
#include "Save/WindlessSaveSubsystem.h"
#include "Level/SavePointSubsystem.h"
#include "Level/Landmarks/LandmarkSubsystem.h"
#include "Level/SoulMarkerSubsystem.h"
#include "Items/Storage/SharedStorageSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

void UWorldCheats::WLSave()
{
	APawn* pawn = GetPlayerPawn();
	if (UPlayerSaveComponent* saveComponent = GetPlayerComponent<UPlayerSaveComponent>())
		saveComponent->RequestSave(FName(TEXT("Debug")), pawn->GetActorLocation(), pawn->GetActorRotation());
}

void UWorldCheats::WLLoad()
{
	UWindlessSaveSubsystem* saveSubsystem = GetSaveSubsystem();
	if (APawn* pawn = GetPlayerPawn(); saveSubsystem && pawn)
		saveSubsystem->TryContinue(pawn);
}

void UWorldCheats::WLDeleteSave()
{
	if (UWindlessSaveSubsystem* saveSubsystem = GetSaveSubsystem())
		saveSubsystem->DeleteSaveData();
}

void UWorldCheats::WLSaveInfo()
{
	if (UWindlessSaveSubsystem* saveSubsystem = GetSaveSubsystem())
		saveSubsystem->PrintSaveInfo();
}

void UWorldCheats::WLSavePoints()
{
	if (UWorld* world = GetWorld())
	{
		if (USavePointSubsystem* savePoints = world->GetSubsystem<USavePointSubsystem>())
			savePoints->PrintDebugInfo();
	}
}

void UWorldCheats::WLStorage()
{
	if (UWorld* world = GetWorld())
	{
		if (USharedStorageSubsystem* storage = world->GetSubsystem<USharedStorageSubsystem>())
			storage->PrintDebugInfo();
	}
}

void UWorldCheats::WLLandmarks()
{
	if (UWorld* world = GetWorld())
	{
		if (ULandmarkSubsystem* landmarks = world->GetSubsystem<ULandmarkSubsystem>())
			landmarks->PrintDebugInfo();
	}
}

void UWorldCheats::WLSoulMarker()
{
	if (UWorld* world = GetWorld())
	{
		if (USoulMarkerSubsystem* markers = world->GetSubsystem<USoulMarkerSubsystem>())
			markers->PrintDebugInfo();
	}
}

UWindlessSaveSubsystem* UWorldCheats::GetSaveSubsystem() const
{
	const UWorld* world = GetWorld();
	const UGameInstance* gameInstance = world ? world->GetGameInstance() : nullptr;
	return gameInstance ? gameInstance->GetSubsystem<UWindlessSaveSubsystem>() : nullptr;
}