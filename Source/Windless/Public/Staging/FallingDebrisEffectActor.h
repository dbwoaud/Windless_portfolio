#pragma once

#include "CoreMinimal.h"
#include "Staging/BaseStagingActor.h"
#include "FallingDebrisEffectActor.generated.h"

class UBoxComponent;
class UStaticMesh;
class UNiagaraSystem;
class USoundBase;
class AStaticMeshActor;

UCLASS()
class WINDLESS_API AFallingDebrisEffectActor : public ABaseStagingActor
{
	GENERATED_BODY()

public:
	AFallingDebrisEffectActor();

protected:
	virtual void HandlePlay() override;
	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Debris")
	TObjectPtr<UBoxComponent> spawnVolume; // 낙석 생성 영역(


	/* 낙석 설정 */
	UPROPERTY(EditAnywhere, Category = "Debris")
	TObjectPtr<UStaticMesh> debrisMesh; // 돌 부스러기 메시

	UPROPERTY(EditAnywhere, Category = "Debris", meta = (ClampMin = "1"))
	int32 debrisCount = 8; // 생성할 낙석 개수

	UPROPERTY(EditAnywhere, Category = "Debris", meta = (ClampMin = "0.0"))
	float spawnInterval = 0.15f; // 낙석 생성 간격

	UPROPERTY(EditAnywhere, Category = "Debris")
	FVector2D debrisScaleRange = FVector2D(0.3f, 0.8f); // 낙석 크기 범위

	UPROPERTY(EditAnywhere, Category = "Debris", meta = (ClampMin = "0.0"))
	float downwardImpulse = 200.f; // 아래 방향 추가 힘

	UPROPERTY(EditAnywhere, Category = "Debris", meta = (ClampMin = "0.1"))
	float debrisLifespan = 6.f; // 낙석 자동 소멸 시간


	/* 연출 */
	UPROPERTY(EditAnywhere, Category = "Debris|FX")
	TObjectPtr<UNiagaraSystem> dustEffect; // 먼지 VFX

	UPROPERTY(EditAnywhere, Category = "Debris|FX")
	TObjectPtr<USoundBase> impactSound; // 낙석 사운드

private:
	/* 낙석 생성 */
	void SpawnSingleDebris(); // 낙석 1개를 생성하는 함수
	FVector GetRandomSpawnLocation() const; // 낙석 생성 영역 내 랜덤 위치를 반환하는 함수
	AStaticMeshActor* SpawnDebrisActor(UWorld* world, const FVector& location); // 낙석 액터를 생성하는 함수
	void SetupDebris(AStaticMeshActor* rock); // 낙석 1개를 설정하는 함수
	int32 spawnedCount = 0; // 현재 낙석 수
	FTimerHandle spawnTimer; // 낙석 순차 생성 타이머 핸들

	
	/* 연출 */
	void PlayDebrisVFX(); // 낙석 연출을 수행하는 함수
};
