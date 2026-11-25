#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Enemy/AI/EnemyAITypes.h"  
#include "EnemySpawner.generated.h"

class AEnemy;
class ATriggerVolumeBase;

UCLASS()
class WINDLESS_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();


	/* 적 설정 */
	void RespawnEnemy(bool bIncludeAlive = false); // 적을 재생성하는 함수

	UFUNCTION(BlueprintPure, Category = "Spawner")
	bool IsEnemyAlive() const; // 현재 적이 살아 있는지 확인하는 함수

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	USceneComponent* sceneRoot; // 루트 컴포넌트 


	/* 적 설정 */
	UPROPERTY(EditAnywhere, Category = "Spawner")
	TSubclassOf<AEnemy> enemyClass; // 생성할 적 클래스

	UPROPERTY(EditAnywhere, Category = "Spawner")
	bool bBossSpawner = false; // 보스 스포너인지 여부

	UPROPERTY(EditAnywhere, Category = "Spawner")
	bool bSpawnOnBeginPlay = true; // 시작 시 즉시 생성할지 여부

	UPROPERTY(EditInstanceOnly, Category = "Spawner|Trigger")
	TObjectPtr<ATriggerVolumeBase> triggerVolume; // 채우면 시작할 때 만들지 않고, 이 트리거에 들어설 때 적을 한 번 생성

	UPROPERTY(EditInstanceOnly, Category = "Spawner|AI")
	TArray<AActor*> patrolPoints; // 순찰형 적의 순찰 경로

	UPROPERTY(EditAnywhere, Category = "Spawner|Placement", meta = (RowType = "/Script/Windless.MonsterArchetypeRow"))
	FDataTableRowHandle archetypeOverride; // 배치 원형

	UPROPERTY(EditAnywhere, Category = "Spawner|Placement")
	EEnemyIdleType idleTypeOverride = EEnemyIdleType::EIT_None; // 배치 대기 유형 덮어쓰기

private:
	/* 적 설정 */
	void SpawnEnemy(); // 적을 생성하는 함수

	UFUNCTION()
	void HandleTriggerEnter(AActor* activator); // 생성 트리거 진입 시 실행되는 함수

	UFUNCTION()
	void HandleEnemyDeath(AActor* destroyedActor); // 적 사망을 처리하는 함수

	UPROPERTY()
	TObjectPtr<AEnemy> spawnedEnemy = nullptr; // 현재 생성된 적

	bool bEnemyDead = false; // 적 사망 여부

public:
	/* 변수 Getter 함수 */
	UFUNCTION(BlueprintPure, Category = "Spawner")
	FORCEINLINE bool IsBossSpawner() const { return bBossSpawner; }
};