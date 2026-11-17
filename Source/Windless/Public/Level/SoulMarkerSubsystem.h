#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Level/SoulMarkerTypes.h"
#include "SoulMarkerSubsystem.generated.h"

class ASoulMarker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSoulMarkerChanged);

UCLASS()
class WINDLESS_API USoulMarkerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/* 마커 설정 */
	void CreateMarker(const FVector& deathLocation, int32 moltStones); // 사망 지점에 마커를 만드는 함수
	void ClearMarker(); // 마커를 제거하는 함수
	void SetMarkerClass(TSubclassOf<ASoulMarker> inClass); // 생성할 마커 클래스를 지정하는 함수

	UPROPERTY()
	TSubclassOf<ASoulMarker> markerClass; // 생성할 마커 클래스


	/* 세이브 */
	void CollectRecord(FSoulMarkerRecord& outRecord) const; // 저장용 레코드를 수집하는 함수
	void ApplyRecord(const FSoulMarkerRecord& record); // 불러온 레코드를 적용하는 함수
	void ResetProgress(); // 마커 상태를 초기화하는 함수
	void PrintDebugInfo() const; // 마커 상태를 로그로 출력하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "SoulMarker|Event")
	FOnSoulMarkerChanged onSoulMarkerChanged; // 마커 생성/제거 시 실행되는 델리게이트

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type worldType) const override;
	virtual void Deinitialize() override;

private:
	/* 마커 설정 */
	void SpawnMarkerActor(); // 마커 액터를 생성하는 함수
	void DestroyMarkerActor(); // 마커 액터를 제거하는 함수

	UPROPERTY()
	TObjectPtr<ASoulMarker> spawnedMarker = nullptr; // 현재 생성된 마커

	FSoulMarkerRecord currentRecord; // 현재 손실 지점 레코드

public:
	/* 변수 Getter 함수 */
	UFUNCTION(BlueprintPure, Category = "SoulMarker")
	FORCEINLINE bool HasMarker() const { return currentRecord.bValid; }

	UFUNCTION(BlueprintPure, Category = "SoulMarker")
	FORCEINLINE FSoulMarkerRecord GetRecord() const { return currentRecord; }

};