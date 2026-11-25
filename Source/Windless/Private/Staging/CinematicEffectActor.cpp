#include "Staging/CinematicEffectActor.h"
#include "Windless/DebugMacro.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void ACinematicEffectActor::HandlePlay()
{
	if (!sequenceActor || !sequenceActor->GetSequencePlayer())
		return;
	

	if (bDisableInputDuringPlay)
	{
		APlayerController* pc = UGameplayStatics::GetPlayerController(this, 0);
		APawn* player = UGameplayStatics::GetPlayerPawn(this, 0);
		if (pc && player)
			player->DisableInput(pc);

		sequenceActor->GetSequencePlayer()->OnFinished.AddDynamic(this, &ACinematicEffectActor::OnCinematicFinished);
	}

	sequenceActor->GetSequencePlayer()->Play();
	UE_LOG(LogWindless, Log, TEXT("시네마틱 재생: %s"), *GetName());
}

void ACinematicEffectActor::OnCinematicFinished()
{
	APlayerController* pc = UGameplayStatics::GetPlayerController(this, 0);
	APawn* player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (pc && player)
		player->EnableInput(pc);

	if (sequenceActor && sequenceActor->GetSequencePlayer())
		sequenceActor->GetSequencePlayer()->OnFinished.RemoveDynamic(this, &ACinematicEffectActor::OnCinematicFinished);

	UE_LOG(LogWindless, Log, TEXT("시네마틱 종료 -> 입력 복구: %s"), *GetName());
}