#include "HUD/WindlessHUD.h"
#include "HUD/WindlessOverlay.h"

void AWindlessHUD::BeginPlay()
{
	Super::BeginPlay();
	UWorld* world = GetWorld();
	if (world)
	{
		APlayerController* controller = world->GetFirstPlayerController();
		if (controller && windlessOverlayClass)
		{
			windlessOverlay = CreateWidget<UWindlessOverlay>(controller, windlessOverlayClass); // 오버레이 위젯을 생성
			windlessOverlay->AddToViewport(); // 오버레이 위젯을 화면에 추가
		}
	}
}