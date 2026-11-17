#pragma once

#include "CoreMinimal.h"
#include "Level/TriggerVolumeBase.h"
#include "FallDeathVolume.generated.h"

UCLASS()
class WINDLESS_API AFallDeathVolume : public ATriggerVolumeBase
{
	GENERATED_BODY()

public:
	AFallDeathVolume();

protected:
	virtual void HandleTriggerEnter(AActor* activator) override; // 진입한 캐릭터를 낙사 처리하는 함수
};