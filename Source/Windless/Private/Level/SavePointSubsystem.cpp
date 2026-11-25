#include "Level/SavePointSubsystem.h"
#include "Level/SavePoint.h"
#include "Windless/DebugMacro.h"
#include "Engine/Engine.h"

void USavePointSubsystem::RegisterSavePoint(ASavePoint* savePoint)
{
	if (!savePoint)
		return;

#if !UE_BUILD_SHIPPING
	for (const TWeakObjectPtr<ASavePoint>& other : loadedSavePoints)
	{
		if (other.IsValid() && other.Get() != savePoint && other->GetSavePointId() == savePoint->GetSavePointId())
			UE_LOG(LogWindless, Error, TEXT("[세이브포인트] ID 중복: %s (%s / %s)"),
				*savePoint->GetSavePointId().ToString(), *other->GetName(), *savePoint->GetName());
	}
#endif

	loadedSavePoints.AddUnique(savePoint);
	if (FSavePointRecord* record = activatedSavePoints.Find(savePoint->GetSavePointId()))
		FillRecord(savePoint, *record);
}

void USavePointSubsystem::UnregisterSavePoint(ASavePoint* savePoint)
{
	loadedSavePoints.RemoveAll([savePoint](const TWeakObjectPtr<ASavePoint>& entry)
		{
			return !entry.IsValid() || entry.Get() == savePoint;
		});
}

bool USavePointSubsystem::ActivateSavePoint(const ASavePoint* savePoint)
{
	if (!savePoint || savePoint->GetSavePointId().IsNone())
		return false;

	const FName id = savePoint->GetSavePointId();
	if (FSavePointRecord* existing = activatedSavePoints.Find(id))
	{
		FillRecord(savePoint, *existing);
		return false;
	}

	FSavePointRecord& record = activatedSavePoints.Add(id);
	FillRecord(savePoint, record);
	record.activationOrder = nextActivationOrder++;

	UE_LOG(LogWindless, Log, TEXT("[세이브포인트] 활성화: %s (총 %d곳)"), *id.ToString(), activatedSavePoints.Num());
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("[세이브포인트] %s 활성화"), *id.ToString()));

	onSavePointActivated.Broadcast(id);
	return true;
}

void USavePointSubsystem::TouchActivationOrder(FName savePointId)
{
	FSavePointRecord* record = activatedSavePoints.Find(savePointId);
	if (!record)
		return;

	for (const TPair<FName, FSavePointRecord>& pair : activatedSavePoints)
	{
		if (pair.Value.activationOrder > record->activationOrder)
		{
			record->activationOrder = nextActivationOrder++;
			return;
		}
	}
}

bool USavePointSubsystem::IsActivated(FName savePointId) const
{
	return activatedSavePoints.Contains(savePointId);
}

bool USavePointSubsystem::FindActivated(FName savePointId, FSavePointRecord& outRecord) const
{
	if (const FSavePointRecord* record = activatedSavePoints.Find(savePointId))
	{
		outRecord = *record;
		return true;
	}
	return false;
}

bool USavePointSubsystem::FindLatestActivated(FSavePointRecord& outRecord) const
{
	const FSavePointRecord* latest = nullptr;

	for (const TPair<FName, FSavePointRecord>& pair : activatedSavePoints)
	{
		if (!latest || pair.Value.activationOrder > latest->activationOrder)
			latest = &pair.Value;
	}

	if (!latest)
		return false;

	outRecord = *latest;
	return true;
}

bool USavePointSubsystem::IsInSafeZone(const FVector& location) const
{
	for (const TWeakObjectPtr<ASavePoint>& savePoint : loadedSavePoints)
	{
		if (savePoint.IsValid() && savePoint->IsLocationInSafeZone(location))
			return true;
	}
	return false;
}

TArray<FSavePointRecord> USavePointSubsystem::GetActivatedSavePoints() const
{
	TArray<FSavePointRecord> result;
	activatedSavePoints.GenerateValueArray(result);
	return result;
}

void USavePointSubsystem::CollectRecord(FSavePointProgressRecord& outRecord) const
{
	activatedSavePoints.GenerateValueArray(outRecord.activatedSavePoints);
}

void USavePointSubsystem::ApplyRecord(const FSavePointProgressRecord& record)
{
	activatedSavePoints.Reset();
	nextActivationOrder = 1;

	for (const FSavePointRecord& entry : record.activatedSavePoints)
	{
		if (!entry.savePointId.IsNone())
			activatedSavePoints.Add(entry.savePointId, entry);
	}
	RefreshLoadedSavePoints();
}

void USavePointSubsystem::ResetProgress()
{
	activatedSavePoints.Reset();
	nextActivationOrder = 1;
	RefreshLoadedSavePoints();
}

void USavePointSubsystem::PrintDebugInfo() const
{
	UE_LOG(LogWindless, Warning, TEXT("===== [세이브포인트] 로드 %d곳 / 활성 %d곳 ====="), loadedSavePoints.Num(), activatedSavePoints.Num());

	for (const TPair<FName, FSavePointRecord>& pair : activatedSavePoints)
		UE_LOG(LogWindless, Log, TEXT("  [활성 #%d] %-20s %s  %s"),
			pair.Value.activationOrder, *pair.Key.ToString(),
			*pair.Value.displayName.ToString(), *pair.Value.respawnLocation.ToCompactString());

	for (const TWeakObjectPtr<ASavePoint>& savePoint : loadedSavePoints)
	{
		if (savePoint.IsValid() && !activatedSavePoints.Contains(savePoint->GetSavePointId()))
			UE_LOG(LogWindless, Log, TEXT("  [비활성] %s"), *savePoint->GetSavePointId().ToString());
	}
}

bool USavePointSubsystem::DoesSupportWorldType(const EWorldType::Type worldType) const
{
	return worldType == EWorldType::Game || worldType == EWorldType::PIE;
}

void USavePointSubsystem::FillRecord(const ASavePoint* savePoint, FSavePointRecord& outRecord)
{
	outRecord.savePointId = savePoint->GetSavePointId();
	outRecord.displayName = savePoint->GetDisplayName();
	outRecord.respawnLocation = savePoint->GetRespawnLocation();
	outRecord.respawnRotation = savePoint->GetRespawnRotation();
}

void USavePointSubsystem::RefreshLoadedSavePoints()
{
	for (const TWeakObjectPtr<ASavePoint>& savePoint : loadedSavePoints)
	{
		if (!savePoint.IsValid())
			continue;

		if (FSavePointRecord* record = activatedSavePoints.Find(savePoint->GetSavePointId()))
			FillRecord(savePoint.Get(), *record);

		savePoint->RefreshActivationState();
	}
}
