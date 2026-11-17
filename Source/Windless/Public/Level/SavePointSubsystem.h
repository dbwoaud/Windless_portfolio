#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Save/WindlessSaveTypes.h"
#include "SavePointSubsystem.generated.h"

class ASavePoint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSavePointActivatedSignature, FName, savePointId);

UCLASS()
class WINDLESS_API USavePointSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/* 등록 및 해제 */
	void RegisterSavePoint(ASavePoint* savePoint);   // 로드된 세이브포인트를 등록하는 함수
	void UnregisterSavePoint(ASavePoint* savePoint); // 언로드되는 세이브포인트를 해제하는 함수


	/* 활성화 */
	bool ActivateSavePoint(const ASavePoint* savePoint); // 세이브포인트를 활성화하고 최초 활성화 여부를 반환하는 함수
	void TouchActivationOrder(FName savePointId); // 휴식 시 활성화 순번을 최신으로 갱신하는 함수

	UFUNCTION(BlueprintPure, Category = "SavePoint")
	bool IsActivated(FName savePointId) const; // 세이브포인트가 활성화 상태인지 확인하는 함수


	/* 조회 */
	bool FindActivated(FName savePointId, FSavePointRecord& outRecord) const; // 활성 세이브포인트 레코드를 찾는 함수
	bool FindLatestActivated(FSavePointRecord& outRecord) const; // 가장 최근 활성화된 세이브포인트를 찾는 함수

	UFUNCTION(BlueprintPure, Category = "SavePoint")
	bool IsInSafeZone(const FVector& location) const; // 위치가 안전지대 안인지 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "SavePoint")
	TArray<FSavePointRecord> GetActivatedSavePoints() const; // 활성 세이브포인트 목록을 반환하는 함수


	/* 세이브 */
	void CollectRecord(FSavePointProgressRecord& outRecord) const; // 저장용 레코드를 수집하는 함수
	void ApplyRecord(const FSavePointProgressRecord& record);      // 불러온 레코드를 적용하는 함수
	void ResetProgress(); // 활성화 상태를 전부 초기화하는 함수


	/* 디버그 */
	void PrintDebugInfo() const; // 현재 활성화 상태를 로그로 출력하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "SavePoint|Event")
	FOnSavePointActivatedSignature onSavePointActivated; // 세이브포인트 최초 활성화 시 실행되는 델리게이트

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type worldType) const override;

private:
	/* 등록 및 해제 */
	static void FillRecord(const ASavePoint* savePoint, FSavePointRecord& outRecord); // 액터 정보로 레코드를 채우는 함수

	TArray<TWeakObjectPtr<ASavePoint>> loadedSavePoints; // 현재 로드된 세이브포인트 액터


	/* 활성화 */
	void RefreshLoadedSavePoints(); // 로드된 세이브포인트들의 활성화 표시를 갱신하는 함수

	UPROPERTY()
	TMap<FName, FSavePointRecord> activatedSavePoints; // 활성화된 세이브포인트
	int32 nextActivationOrder = 1; // 다음 활성화에 부여할 순번
};