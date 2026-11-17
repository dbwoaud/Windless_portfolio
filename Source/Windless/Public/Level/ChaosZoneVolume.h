#pragma once

#include "CoreMinimal.h"
#include "Components/AttributeComponent.h"
#include "Level/TriggerVolumeBase.h"
#include "ChaosZoneVolume.generated.h"

// 혼돈 게이지의 빛 구역 또는 어둠 구역. 칠하지 않은 곳은 그늘(변화 없음)이다.
// 빛: 초당 감소, lightFloor까지만 내린다. 어둠: 초당 증가, darkCap까지만 올린다. 겹치면 빛이 이긴다.
UCLASS()
class WINDLESS_API AChaosZoneVolume : public ATriggerVolumeBase
{
	GENERATED_BODY()

public:
	AChaosZoneVolume();
	virtual void OnConstruction(const FTransform& transform) override;

	void InitAsLight(float floor, float radius); // 실행 중에 놓는 빛 (빛나는 돌)

	FORCEINLINE EChaosZone GetZone() const { return zone; }
	FORCEINLINE float GetLimit() const { return zone == EChaosZone::Light ? lightFloor : darkCap; }

protected:
	virtual void HandleTriggerEnter(AActor* activator) override;
	virtual void HandleTriggerExit(AActor* activator) override;

	UPROPERTY(EditAnywhere, Category = "Chaos")
	EChaosZone zone = EChaosZone::Dark; // 빛 구역인지 어둠 구역인지

	UPROPERTY(EditAnywhere, Category = "Chaos", meta = (EditCondition = "zone == EChaosZone::Light", ClampMin = "-100.0", ClampMax = "100.0"))
	float lightFloor = 0.f; // 빛의 하한. 세이브포인트 빛과 빛나는 돌 0, 횃불과 햇빛 -21

	UPROPERTY(EditAnywhere, Category = "Chaos", meta = (EditCondition = "zone == EChaosZone::Dark", ClampMin = "-100.0", ClampMax = "100.0"))
	float darkCap = 50.f; // 어둠의 상한. 블록아웃 때 어둠마다 정한다 (예: 얕은 어둠 50, 깊은 어둠 100)
};
