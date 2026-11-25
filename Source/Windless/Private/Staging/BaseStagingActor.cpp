#include "Staging/BaseStagingActor.h"
#include "Level/TriggerVolumeBase.h"
#include "Windless/DebugMacro.h"
#include "TimerManager.h"

ABaseStagingActor::ABaseStagingActor()
{
	PrimaryActorTick.bCanEverTick = false;

	sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = sceneRoot;
}

void ABaseStagingActor::Play()
{
	if (bPlayOnce && bHasPlayed) // 일회성 연출인 경우 무시
		return;

	bHasPlayed = true;

	if (startDelay > 0.f)
		GetWorldTimerManager().SetTimer(startDelayTimer, this, &ABaseStagingActor::DoPlay, startDelay, false);
	else
		DoPlay();
}

void ABaseStagingActor::Stop()
{
	if (!bIsActive) // 현재 연출 진행 중이 아니면 무시
		return;

	bIsActive = false;
	GetWorldTimerManager().ClearTimer(startDelayTimer);

	UE_LOG(LogWindless, Log, TEXT("연출 정지: %s"), *GetName());
	HandleStop();
	OnStagingStopBP();
	onStagingStopped.Broadcast();
}

void ABaseStagingActor::ResetStaging()
{
	bHasPlayed = false;
	UE_LOG(LogWindless, Log, TEXT("연출 리셋: %s"), *GetName());
}

void ABaseStagingActor::BeginPlay()
{
	Super::BeginPlay();
	if (bBindToTrigger && triggerVolume) // 트리거에 자동 할당
		triggerVolume->onTriggerEnter.AddDynamic(this, &ABaseStagingActor::HandleTriggerEnter);

	if (stopTriggerVolume)
		stopTriggerVolume->onTriggerEnter.AddDynamic(this, &ABaseStagingActor::HandleStopTriggerEnter);

	if (bAutoPlayOnBeginPlay) 
		Play();
}

void ABaseStagingActor::HandlePlay()
{

}

void ABaseStagingActor::HandleStop()
{

}

void ABaseStagingActor::HandleTriggerEnter(AActor* activator)
{
	Play();
}

void ABaseStagingActor::HandleStopTriggerEnter(AActor* activator)
{
	Stop();
}

void ABaseStagingActor::DoPlay()
{
	bIsActive = true;

	UE_LOG(LogWindless, Log, TEXT("연출 발동: %s"), *GetName());

	HandlePlay();
	OnStagingPlayBP();
	onStagingPlayed.Broadcast();
	PlayChainedEffects();
}

void ABaseStagingActor::PlayChainedEffects()
{
	for (ABaseStagingActor* effect : chainedEffects)
	{
		if (effect && effect != this)
			effect->Play();
	}
}