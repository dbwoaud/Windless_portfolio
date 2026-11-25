#pragma once

#include "CoreMinimal.h"
#include "Staging/BaseStagingActor.h"
#include "CameraShakeEffectActor.generated.h"

class UCameraShakeBase;

UCLASS()
class WINDLESS_API ACameraShakeEffectActor : public ABaseStagingActor
{
	GENERATED_BODY()
	
protected:
	/* 연출 제어 */
	virtual void HandlePlay() override; // 연출 시작을 처리하는 함수
	virtual void HandleStop() override; // 연출 중지를 처리하는 함수


	/* 카메라 설정 */
	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> shakeClass; // 재생할 카메라 쉐이크 클래스

	UPROPERTY(EditAnywhere, Category = "CameraShake", meta = (ClampMin = "0.0"))
	float shakeScale = 1.f; // 쉐이크 강도 배율
};
