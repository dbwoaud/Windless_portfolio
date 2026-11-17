#include "Staging/CameraShakeEffectActor.h"
#include "Windless/DebugMacro.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"

void ACameraShakeEffectActor::HandlePlay()
{
	if (!shakeClass)
	{
		UE_LOG(LogWindless, Warning, TEXT("쉐이크 클래스가 없음: %s"), *GetName());
		return;
	}

	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	if (playerController && playerController->PlayerCameraManager)
		playerController->PlayerCameraManager->StartCameraShake(shakeClass, shakeScale);
}

void ACameraShakeEffectActor::HandleStop()
{
	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	if (playerController && playerController->PlayerCameraManager)
		playerController->PlayerCameraManager->StopAllInstancesOfCameraShake(shakeClass, true);
}