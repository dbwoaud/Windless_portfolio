#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Save/WindlessSaveTypes.h"
#include "PlayerSaveComponent.generated.h"

class UWindlessSaveSubsystem;

DECLARE_MULTICAST_DELEGATE(FOnPlayerSaveApplied); // 세이브 데이터 적용이 끝났을 때 호출되는 델리게이트

/*
 * 플레이어 세이브 컴포넌트
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UPlayerSaveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerSaveComponent();


	/* 세이브 데이터 */
	void CollectSaveData(FInventorySaveRecord& outInventory, FPlayerStatSaveRecord& outStats) const; // 플레이어 상태를 세이브 데이터로 수집하는 함수
	void ApplySaveData(const FInventorySaveRecord& inventoryRecord, const FPlayerStatSaveRecord& statRecord); // 세이브 데이터를 플레이어에게 적용하는 함수


	/* 세이브 서브시스템 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool RequestSave(FName savePointId, FVector respawnLocation, FRotator respawnRotation); // 세이브 서브시스템에 저장을 요청하는 함수


	/* 델리게이트 */
	FOnPlayerSaveApplied onSaveApplied; // 세이브 데이터 적용 완료 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;

private:
	/* 세이브 데이터 */
	void HandleAutoLoad(); // 게임 시작 시 세이브를 자동으로 불러오는 함수
	void TeleportToLoadedSavePoint(); // 불러온 세이브포인트 위치로 이동하는 함수

	UPROPERTY(EditAnywhere, Category = "Save")
	bool bAutoLoadOnStart = false; // 게임 시작 시 자동으로 세이브를 불러올지 여부

	UPROPERTY(EditAnywhere, Category = "Save")
	bool bTeleportToSavePointOnLoad = true; // 불러오기 시 저장된 세이브포인트로 이동할지 여부


	/* 세이브 서브 시스템 */
	UWindlessSaveSubsystem* GetSaveSubsystem() const; // 세이브 서브시스템을 반환하는 함수
};