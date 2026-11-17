#pragma once

#include "CoreMinimal.h"
#include "Level/TriggerVolumeBase.h"
#include "DoorCloseTrigger.generated.h"

class ABaseDoor;

UCLASS()
class WINDLESS_API ADoorCloseTrigger : public ATriggerVolumeBase
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleClose(AActor* activator); // 진입 시 지정된 문을 닫는 함수

	UPROPERTY(EditInstanceOnly, Category = "DoorClose")
	TObjectPtr<ABaseDoor> targetDoor; // 닫을 문
};
