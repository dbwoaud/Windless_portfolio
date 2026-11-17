#include "Level/Landmarks/LandmarkSubsystem.h"
#include "Level/Landmarks/Landmark.h"
#include "Windless/DebugMacro.h"
#include "Engine/Engine.h"
#include "Level/Landmarks/LandmarkSubsystem.h"

bool ULandmarkSubsystem::DiscoverLandmark(const ALandmark* landmark)
{
	if (!landmark || landmark->GetLandmarkId().IsNone())
		return false;

	const FName id = landmark->GetLandmarkId();
	if (discoveredLandmarks.Contains(id))
		return false;

	FLandmarkRecord& record = discoveredLandmarks.Add(id);
	record.landmarkId = id;
	record.displayName = landmark->GetDisplayName();
	record.location = landmark->GetActorLocation();

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
			FString::Printf(TEXT("%s 발견"), *record.displayName.ToString()));

	onLandmarkDiscovered.Broadcast(id);
	return true;
}

bool ULandmarkSubsystem::IsDiscovered(FName landmarkId) const
{
	return discoveredLandmarks.Contains(landmarkId);
}

TArray<FLandmarkRecord> ULandmarkSubsystem::GetDiscoveredLandmarks() const
{
	TArray<FLandmarkRecord> result;
	discoveredLandmarks.GenerateValueArray(result);
	return result;
}

void ULandmarkSubsystem::CollectRecord(FLandmarkProgressRecord& outRecord) const
{
	discoveredLandmarks.GenerateValueArray(outRecord.discoveredLandmarks);
}

void ULandmarkSubsystem::ApplyRecord(const FLandmarkProgressRecord& record)
{
	discoveredLandmarks.Reset();
	for (const FLandmarkRecord& entry : record.discoveredLandmarks)
	{
		if (!entry.landmarkId.IsNone())
			discoveredLandmarks.Add(entry.landmarkId, entry);
	}
}

void ULandmarkSubsystem::ResetProgress()
{
	discoveredLandmarks.Reset();
}

void ULandmarkSubsystem::PrintDebugInfo() const
{
	UE_LOG(LogWindless, Warning, TEXT("===== [랜드마크] 발견 %d곳 ====="), discoveredLandmarks.Num());
	for (const TPair<FName, FLandmarkRecord>& pair : discoveredLandmarks)
		UE_LOG(LogWindless, Log, TEXT("  %-20s %s"), *pair.Key.ToString(), *pair.Value.displayName.ToString());
}

bool ULandmarkSubsystem::DoesSupportWorldType(const EWorldType::Type worldType) const
{
	return worldType == EWorldType::Game || worldType == EWorldType::PIE;
}
