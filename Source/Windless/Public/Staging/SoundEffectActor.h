#pragma once

#include "CoreMinimal.h"
#include "Staging/BaseStagingActor.h"
#include "SoundEffectActor.generated.h"

UCLASS()
class WINDLESS_API ASoundEffectActor : public ABaseStagingActor
{
	GENERATED_BODY()
protected:
	/* 연출 제어 */
	virtual void HandlePlay() override; // 연출 시작을 처리하는 함수
	virtual void HandleStop() override; // 연출 중지를 처리하는 함수


	/* 소리 설정 */
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> sound; // 재생할 소리

	UPROPERTY(EditAnywhere, Category = "Sound")
	bool bIsSpatialSound = false; // 공간 음향 여부 

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (ClampMin = "0.0"))
	float stopFadeTime = 0.5f; // 정지할 때 소리가 잦아드는 시간

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (ClampMin = "0.0"))
	float volumeMultiplier = 1.f; // 볼륨 배율

	UPROPERTY(EditAnywhere, Category = "Sound")
	bool bIsLoop = false; // 소리 반복 재생 여부

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (EditCondition = "bIsLoop", ClampMin = "0.0"))
	float minInterval = 20.f; // 최소 반복 재생 간격

	UPROPERTY(EditAnywhere, Category = "Sound", meta = (EditCondition = "bIsLoop", ClampMin = "0.0"))
	float maxInterval = 30.f; // 최대 반복 재생 간격

private:
	/* 소리 재생 */
	void PlayOneShot(); // 소리를 한 번 재생하는 함수
	void ScheduleSound(); // 다음 소리 재생을 예약하는 함수

	TArray<TWeakObjectPtr<UAudioComponent>> playingSounds; // 지금 울리는 소리들 (정지할 때 멈춘다)

	FTimerHandle randomLoopTimer; // 반복 재생 타이머 핸들
};