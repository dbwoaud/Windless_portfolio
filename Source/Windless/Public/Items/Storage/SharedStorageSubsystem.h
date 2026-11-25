#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Level/StorageTypes.h"
#include "SharedStorageSubsystem.generated.h"

class UInventoryComponent;
class AStorageHolder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStorageGradeChanged, EStorageGrade, newGrade);

UCLASS()
class WINDLESS_API USharedStorageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/* 창고 시스템 */
	UFUNCTION(BlueprintPure, Category = "Storage")
	UInventoryComponent* GetStorageInventory(); // 통합 창고 인벤토리를 반환하는 함수

	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool UpgradeTo(EStorageGrade newGrade); // 창고 등급을 올리는 함수


	/* 세이브 */
	void CollectRecord(FStorageSaveRecord& outRecord); // 저장용 레코드를 수집하는 함수
	void ApplyRecord(const FStorageSaveRecord& record); // 불러온 레코드를 적용하는 함수
	void ResetStorage(); // 창고를 초기 상태로 되돌리는 함수


	/* 디버그 */
	void PrintDebugInfo(); // 창고 상태를 로그로 출력하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Storage|Event")
	FOnStorageGradeChanged onStorageGradeChanged; // 창고 등급 변경 시 실행되는 델리게이트

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type worldType) const override;
	virtual void Deinitialize() override;

private:
	/* 창고 설정 */
	void EnsureHolder(); // 창고 액터를 준비하는 함수
	void ApplyGradeToGrid(); // 현재 등급을 그리드 크기에 반영하는 함수

	UPROPERTY()
	TObjectPtr<AStorageHolder> holder = nullptr; // 창고 인벤토리를 담은 액터

	EStorageGrade currentGrade = EStorageGrade::ESG_Small; // 현재 창고 등급

public:
	UFUNCTION(BlueprintPure, Category = "Storage")
	FORCEINLINE EStorageGrade GetGrade() const { return currentGrade; }
};