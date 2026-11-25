#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyPerceptionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyTargetLost, FVector, lastKnownLocation);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UEnemyPerceptionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyPerceptionComponent();

	/* 감지(제어) */
	void SetPerceptionEnabled(bool bEnable); // 감지 판정을 켜고 끄는 함수
	void ForceTrack(APawn* target); // 즉시 인지 상태로 만드는 함수


	/* 감지(조회) */
	float GetEffectiveSightRadius(const APawn* target) const; // 조도/은신 보정이 적용된 감지 거리를 반환하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Perception")
	FOnEnemyTargetLost onTargetLost; // 시야 상실 후 기억 시간이 지나면 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/* 캐시 변수 */
	TWeakObjectPtr<APawn> trackedTarget; // 인지 중인 대상
	FVector lastKnownLocation = FVector::ZeroVector; // 마지막으로 파악한 대상 위치


	/* 감지(설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Perception", meta = (ClampMin = "0.0"))
	float sightRadius = 2000.f; // 기본 감지 거리

	UPROPERTY(EditDefaultsOnly, Category = "Perception", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float sightHalfAngle = 60.f; // 시야 반각 (정면 기준 좌우)

	UPROPERTY(EditDefaultsOnly, Category = "Perception", meta = (ClampMin = "0.0"))
	float darknessMultiplier = 1.25f; // 대상이 빛 구역 밖일 때 감지 거리 배율

	UPROPERTY(EditDefaultsOnly, Category = "Perception", meta = (ClampMin = "0.0"))
	float stealthMultiplier = 0.6f; // 대상이 웅크리기 중일 때 감지 거리 배율

	UPROPERTY(EditDefaultsOnly, Category = "Perception", meta = (ClampMin = "0.0"))
	float memoryDuration = 3.f; // 시야 상실 후 위치를 계속 파악하는 시간

	UPROPERTY(EditDefaultsOnly, Category = "Perception", meta = (ClampMin = "0.05"))
	float checkInterval = 0.2f; // 감지 판정 주기

	UPROPERTY(EditDefaultsOnly, Category = "Perception")
	TEnumAsByte<ECollisionChannel> sightTraceChannel = ECC_Visibility; // 장애물 판정 채널

	UPROPERTY(EditAnywhere, Category = "Perception|Debug")
	bool bDrawDebug = false; // 시야 디버그 여부

	bool bHasLineOfSight = false; // 현재 대상을 직접 보고 있는지 여부
	float timeSinceLastSeen = 0.f; // 시야 상실 후 경과 시간
	FTimerHandle checkTimer; // 감지 판정 타이머


	/* 감지(판정) */
	void EvaluatePerception(); // 주기적으로 감지를 판정하는 함수


	/* 감지(조회) */
	APawn* FindCandidate() const; // 감지 후보를 반환하는 함수
	bool CanSee(const APawn* target) const; // 거리, 시야각, 장애물로 대상을 볼 수 있는지 확인하는 함수


	/* 디버그용 */
	void DrawSightDebug(const APawn* target, bool bSees) const; // 시야 범위를 디버그로 그리는 함수


public:
	/* 변수 Getter 함수 */
	FORCEINLINE APawn* GetTrackedTarget() const { return trackedTarget.Get(); }
	FORCEINLINE bool HasLineOfSight() const { return bHasLineOfSight; }
	FORCEINLINE const FVector& GetLastKnownLocation() const { return lastKnownLocation; }
};
