#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyRespawnSubsystem.generated.h"

class AEnemySpawner;

UCLASS()
class WINDLESS_API UEnemyRespawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/* 스포너 설정 */
	void RegisterSpawner(AEnemySpawner* spawner); // 로드된 스포너를 등록하는 함수
	void UnregisterSpawner(AEnemySpawner* spawner); // 언로드되는 스포너를 해제하는 함수

	UFUNCTION(BlueprintCallable, Category = "Enemy|Respawn")
	int32 RespawnAll(bool bIncludeAlive = false); // 등록된 스포너의 적을 모두 재생성하는 함수

	void PrintDebugInfo() const; // 스포너 상태를 로그로 출력하는 함수

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type worldType) const override;

private:
	TArray<TWeakObjectPtr<AEnemySpawner>> spawners; // 현재 로드된 스포너

};