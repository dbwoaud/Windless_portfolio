#include "Level/SoulMarkerSubsystem.h"
#include "Level/SoulMarker.h"
#include "Windless/DebugMacro.h"
#include "Engine/World.h"

void USoulMarkerSubsystem::CreateMarker(const FVector& deathLocation, int32 moltStones)
{
	DestroyMarkerActor();

	currentRecord.bValid = true;
	currentRecord.moltStones = FMath::Max(0, moltStones);
	currentRecord.location = deathLocation;

	SpawnMarkerActor();

	UE_LOG(LogWindless, Log, TEXT("[손실 회수] 사망 지점에 탈피석 %d 남김 (%s)"),
		currentRecord.moltStones, *deathLocation.ToCompactString());

	onSoulMarkerChanged.Broadcast();
}

void USoulMarkerSubsystem::ClearMarker()
{
	DestroyMarkerActor();
	currentRecord = FSoulMarkerRecord();
	onSoulMarkerChanged.Broadcast();
}

void USoulMarkerSubsystem::SetMarkerClass(TSubclassOf<ASoulMarker> inClass)
{
	if (inClass)
		markerClass = inClass;
}

void USoulMarkerSubsystem::CollectRecord(FSoulMarkerRecord& outRecord) const
{
	outRecord = currentRecord;
}

void USoulMarkerSubsystem::ApplyRecord(const FSoulMarkerRecord& record)
{
	DestroyMarkerActor();
	currentRecord = record;

	if (currentRecord.bValid)
		SpawnMarkerActor();

	onSoulMarkerChanged.Broadcast();
}

void USoulMarkerSubsystem::ResetProgress()
{
	ClearMarker();
}

void USoulMarkerSubsystem::PrintDebugInfo() const
{
	if (currentRecord.bValid)
	{
		UE_LOG(LogWindless, Warning, TEXT("[손실 회수] 탈피석 %d @ %s"),
			currentRecord.moltStones, *currentRecord.location.ToCompactString());
	}
	else
		UE_LOG(LogWindless, Warning, TEXT("[손실 회수] 대기 중인 손실 지점 없음"));
}

bool USoulMarkerSubsystem::DoesSupportWorldType(const EWorldType::Type worldType) const
{
	return worldType == EWorldType::Game || worldType == EWorldType::PIE;
}

void USoulMarkerSubsystem::Deinitialize()
{
	DestroyMarkerActor();
	Super::Deinitialize();
}

void USoulMarkerSubsystem::SpawnMarkerActor()
{
	UWorld* world = GetWorld();
	if (!world || !currentRecord.bValid)
		return;

	const TSubclassOf<ASoulMarker> spawnClass = markerClass ? markerClass : TSubclassOf<ASoulMarker>(ASoulMarker::StaticClass());

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	params.ObjectFlags |= RF_Transient;

	spawnedMarker = world->SpawnActor<ASoulMarker>(spawnClass, currentRecord.location, FRotator::ZeroRotator, params);
	if (spawnedMarker)
		spawnedMarker->Initialize(currentRecord.moltStones);
}

void USoulMarkerSubsystem::DestroyMarkerActor()
{
	if (IsValid(spawnedMarker))
		spawnedMarker->Destroy();

	spawnedMarker = nullptr;
}
