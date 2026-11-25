#include "Level/IntroSequenceDirector.h"
#include "Windless/DebugMacro.h"
#include "Characters/WindlessCharacter.h"
#include "Components/DialogueComponent.h"
#include "Staging/BaseStagingActor.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"

AIntroSequenceDirector::AIntroSequenceDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AIntroSequenceDirector::StartIntro()
{
	CachePlayer();

	if (!playerCharacter || !playerController)
	{
		UE_LOG(LogWindless, Warning, TEXT("플레이어를 찾지 못해 인트로 중단"));
		return;
	}

	UE_LOG(LogWindless, Log, TEXT("인트로 시작"));
	StartFadeIn();
	DisablePlayerControl();
	PlayStagingEffects();
	PlayGetUp();
}

void AIntroSequenceDirector::BeginPlay()
{
	Super::BeginPlay();
	if (bAutoStartOnBeginPlay)
		StartIntro();
}

void AIntroSequenceDirector::OnGetUpMontageEnded(UAnimMontage* montage, bool bInterrupted)
{
	if (montage != getUpMontage) // 다른 애니메이션 종료 무시
		return;

	if (cachedAnimInstance)
	{
		cachedAnimInstance->OnMontageEnded.RemoveDynamic(this, &AIntroSequenceDirector::OnGetUpMontageEnded);
		cachedAnimInstance = nullptr;
	}
	OnGetUpComplete();
}

void AIntroSequenceDirector::OnIntroDialogueFinished()
{
	if (playerCharacter)
	{
		if (UDialogueComponent* dialogue = playerCharacter->GetDialogueComponent())
			dialogue->onDialogueFinished.RemoveDynamic(this, &AIntroSequenceDirector::OnIntroDialogueFinished);
	}
	FinishIntro();
}

void AIntroSequenceDirector::CachePlayer()
{
	playerCharacter = Cast<AWindlessCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	playerController = UGameplayStatics::GetPlayerController(this, 0);
}

void AIntroSequenceDirector::FinishIntro()
{
	UE_LOG(LogWindless, Log, TEXT("인트로 종료"));
	onIntroFinished.Broadcast();
}

void AIntroSequenceDirector::StartFadeIn()
{
	if (playerController->PlayerCameraManager)
	{
		playerController->PlayerCameraManager->SetManualCameraFade(1.f, fadeColor, false);
		playerController->PlayerCameraManager->StartCameraFade(1.f, 0.f, fadeInDuration, fadeColor, false, false);
	}
}

void AIntroSequenceDirector::PlayGetUp()
{
	UAnimInstance* animInstance = playerCharacter->GetMesh() ? playerCharacter->GetMesh()->GetAnimInstance() : nullptr;
	if (getUpMontage && animInstance)
	{
		cachedAnimInstance = animInstance;
		animInstance->OnMontageEnded.AddDynamic(this, &AIntroSequenceDirector::OnGetUpMontageEnded);
		animInstance->Montage_Play(getUpMontage, getUpPlayRate);
	}
	else
	{
		UE_LOG(LogWindless, Log, TEXT("인트로 기상 애니메이션 미지정 -> 대체 타이머 사용"));
		GetWorldTimerManager().SetTimer(getUpFallbackTimer, this, &AIntroSequenceDirector::OnGetUpComplete, fallbackGetUpDuration, false);
	}
}

void AIntroSequenceDirector::OnGetUpComplete()
{
	EnablePlayerControl();
	UE_LOG(LogWindless, Log, TEXT("인트로 제어 활성화"));
	if (introDialogueTable)
	{
		if (UDialogueComponent* dialogue = playerCharacter->GetDialogueComponent())
		{
			dialogue->onDialogueFinished.AddDynamic(this, &AIntroSequenceDirector::OnIntroDialogueFinished);
			dialogue->StartDialogue(introDialogueTable);
			return;
		}
	}

	FinishIntro();
}

void AIntroSequenceDirector::EnablePlayerControl()
{
	playerCharacter->EnableInput(playerController);
}

void AIntroSequenceDirector::DisablePlayerControl()
{
	playerCharacter->DisableInput(playerController);
}

void AIntroSequenceDirector::PlayStagingEffects()
{
	for (ABaseStagingActor* effect : introStagingEffects)
	{
		if (effect)
			effect->Play();
	}
}