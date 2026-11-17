#pragma once

#include "CoreMinimal.h"
#include "Staging/BaseStagingActor.h"
#include "FogEffectActor.generated.h"

class AExponentialHeightFog;
class UExponentialHeightFogComponent;

UCLASS()
class WINDLESS_API AFogEffectActor : public ABaseStagingActor
{
	GENERATED_BODY()
	
public:	
	AFogEffectActor();
	virtual void Tick(float DeltaTime) override;

protected:
	/* 연출 제어 */
	virtual void HandlePlay() override;


	/* 대상 안개 */
	UPROPERTY(EditInstanceOnly, Category = "Fog")
	TObjectPtr<AExponentialHeightFog> targetFog; // 안개 액터


	/* 목표 설정 */
	UPROPERTY(EditAnywhere, Category = "Fog|Target Setting", meta = (ClampMin = "0.0"))
	float targetDensity = 0.05f; // 목표 안개 밀도

	UPROPERTY(EditAnywhere, Category = "Fog|Target Setting")
	FLinearColor targetInscatteringColor = FLinearColor(0.4f, 0.4f, 0.45f, 1.f); // 목표 안개 색

	UPROPERTY(EditAnywhere, Category = "Fog|Target Setting", meta = (ClampMin = "0.0"))
	float targetHeightFalloff = 0.2f; // 목표 높이 낙차

	UPROPERTY(EditAnywhere, Category = "Fog|Target Setting", meta = (ClampMin = "0.01"))
	float blendTime = 3.f; // 블렌딩에 걸리는 시간

private:	
	/* 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UExponentialHeightFogComponent> fogComponent; // 대상 안개 컴포넌트


	/* 상태 확인 */
	float currentDensity = 0.f; // 현재 안개 밀도
	FLinearColor currentColor = FLinearColor::Black; // 현재 안개 색
	float currentFalloff = 0.f; // 현재 높이 낙차
	bool bBlending = false;   // 블렌딩이 진행 중인지 여부
	float blendAlpha = 0.f;   // 현재 블렌딩 진행도
};