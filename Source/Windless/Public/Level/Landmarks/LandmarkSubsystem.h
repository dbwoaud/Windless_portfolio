#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Level/Landmarks/LandmarkTypes.h"
#include "LandmarkSubsystem.generated.h"

class ALandmark;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLandmarkDiscovered, FName, landmarkId);

UCLASS()
class WINDLESS_API ULandmarkSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/* 랜드마크 조회*/
	bool DiscoverLandmark(const ALandmark* landmark); // 랜드마크 발견 시 실행되는 함수

	UFUNCTION(BlueprintPure, Category = "Landmark")
	bool IsDiscovered(FName landmarkId) const; // 이미 발견한 랜드마크인지 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "Landmark")
	TArray<FLandmarkRecord> GetDiscoveredLandmarks() const; // 발견한 랜드마크 목록을 반환하는 함수


	/* 세이브 */
	void CollectRecord(FLandmarkProgressRecord& outRecord) const; // 저장용 레코드를 수집하는 함수
	void ApplyRecord(const FLandmarkProgressRecord& record); // 불러온 레코드를 적용하는 함수
	void ResetProgress(); // 발견 상태를 전부 초기화하는 함수


	/* 디버그 */
	void PrintDebugInfo() const; // 발견 상태를 로그로 출력하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Landmark|Event")
	FOnLandmarkDiscovered onLandmarkDiscovered; // 랜드마크 최초 발견 시 실행되는 델리게이트

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type worldType) const override;

private:
	UPROPERTY()
	TMap<FName, FLandmarkRecord> discoveredLandmarks; // 발견한 랜드마크
};