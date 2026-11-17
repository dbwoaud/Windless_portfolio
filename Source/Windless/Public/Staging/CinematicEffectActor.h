#pragma once

#include "CoreMinimal.h"
#include "Staging/BaseStagingActor.h"
#include "CinematicEffectActor.generated.h"

class ALevelSequenceActor;

UCLASS()
class WINDLESS_API ACinematicEffectActor : public ABaseStagingActor
{
	GENERATED_BODY()
protected:
    /* 연출 제어 */
    virtual void HandlePlay() override; // 시네마틱 연출 시작을 처리하는 함수


    UPROPERTY(EditInstanceOnly, Category = "Cinematic")
    TObjectPtr<ALevelSequenceActor> sequenceActor; // 재생할 레벨 시퀀스 액터

    UPROPERTY(EditAnywhere, Category = "Cinematic")
    bool bDisableInputDuringPlay = true; // 재생 중 플레이어 입력을 잠글지 여부

private:
    /* 입력 제어 */
    UFUNCTION()
    void OnCinematicFinished(); // 시네마틱 종료 시 입력을 복구하는 함수
};
