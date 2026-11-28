#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WindlessSaveSubsystem.generated.h"

class UWindlessSaveGame;
class USavePointSubsystem;
class USharedStorageSubsystem;
class ULandmarkSubsystem;
class USoulMarkerSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameSavedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameLoadedSignature);

UCLASS()
class WINDLESS_API UWindlessSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/* 저장 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveGameData(AActor* player, FName savePointId, FVector respawnLocation, FRotator respawnRotation); // 현재 게임 세이브 데이터를 저장하는 함수


	/* 불러오기 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGameData(); // 게임 세이브 데이터를 로딩하는 함수

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool ApplyToPlayer(AActor* player); // 게임 세이브 데이터를 캐릭터에 적용하는 함수

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool TryContinue(AActor* player); // 게임 세이브 데이터가 있으면 즉시 적용하는 함수


	/* 조회/삭제 */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool GetLoadedRespawnTransform(FVector& outLocation, FRotator& outRotation) const; // 저장된 부활 좌표를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Save")
	bool HasSaveData() const; // 저장된 게임 세이브 데이터가 존재하는지 확인하는 함수

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteSaveData(); // 저장된 게임 세이브 데이터를 삭제하는 함수

	UFUNCTION(BlueprintPure, Category = "Save")
	FName GetLoadedSavePointId() const; // 마지막으로 읽어온 게임 세이브 포인트 ID를 반환하는 함수


	/* 지름길 문 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void MarkDoorOpened(FName doorId); // 연 문을 기억하고, 세이브 파일이 있으면 그 파일에 바로 적는 함수

	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsDoorOpened(FName doorId) const { return openedDoors.Contains(doorId); } // 열어 둔 문인지 확인하는 함수


	/* 창고 시스템 */
	static USharedStorageSubsystem* GetStorageSubsystem(const UObject* worldContext); // 창고 서브 시스템을 반환하는 함수


	/* 디버그 */
	UFUNCTION(BlueprintCallable, Category = "Save|Debug")
	void PrintSaveInfo(); // 저장된 슬롯의 내용을 로그로 출력하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Save|Event")
	FOnGameSavedSignature onGameSaved; // 저장 완료 시 실행되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Save|Event")
	FOnGameLoadedSignature onGameLoaded; // 불러오기 완료 시 실행되는 델리게이트

private:
	TSet<FName> openedDoors; // 열어 둔 지름길 문 ID

	/* 슬롯 설정 */
	static const FString saveSlotName; // 세이브 슬롯 이름
	static const int32 saveUserIndex = 0; // 세이브 사용자 인덱스
	static const int32 currentSaveVersion = 6; // 현재 지원하는 세이브 버전


	/* 버전 이전 / 유틸 */
	static void MigrateSave(UWindlessSaveGame& save); // 이전 버전 세이브를 현재 버전으로 올리는 함수
	static USavePointSubsystem* GetSavePointSubsystem(const UObject* worldContext); // 세이브포인트 서브시스템을 반환하는 함수
	static USoulMarkerSubsystem* GetSoulMarkerSubsystem(const UObject* worldContext); // 손실 탈피석 서브시스템을 반환하는 함수


	/* 캐시 */
	UPROPERTY()
	TObjectPtr<UWindlessSaveGame> loadedSave = nullptr; // 읽어온 세이브 데이터

	static ULandmarkSubsystem* GetLandmarkSubsystem(const UObject* worldContext); // 랜드마크 서브 시스템을 반환하는 함수
};
