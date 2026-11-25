#include "Staging/SoundEffectActor.h"
#include "Windless/DebugMacro.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"

void ASoundEffectActor::HandlePlay()
{
	if (!sound)
	{
		UE_LOG(LogWindless, Warning, TEXT("재생할 소리가 없음: %s"), *GetName());
		return;
	}

	if (bIsLoop) // 반복 재생인 경우
		ScheduleSound();
	else // 한 번 재생인 경우
		PlayOneShot();
}

void ASoundEffectActor::HandleStop()
{
	GetWorldTimerManager().ClearTimer(randomLoopTimer);

	for (const TWeakObjectPtr<UAudioComponent>& playing : playingSounds) // 울리고 있는 소리도 멈춘다
	{
		if (playing.IsValid())
			playing->FadeOut(stopFadeTime, 0.f);
	}
	playingSounds.Reset();
}

void ASoundEffectActor::PlayOneShot()
{
	if (!sound)
		return;

	UAudioComponent* playing = bIsSpatialSound
		? UGameplayStatics::SpawnSoundAtLocation(this, sound, GetActorLocation(), FRotator::ZeroRotator, volumeMultiplier)
		: UGameplayStatics::SpawnSound2D(this, sound, volumeMultiplier);

	playingSounds.RemoveAll([](const TWeakObjectPtr<UAudioComponent>& entry) { return !entry.IsValid(); });
	if (playing)
		playingSounds.Add(playing);
}

void ASoundEffectActor::ScheduleSound()
{
	PlayOneShot();
	const float nextDelay = FMath::FRandRange(minInterval, maxInterval);
	GetWorldTimerManager().SetTimer(randomLoopTimer, this, &ASoundEffectActor::ScheduleSound, nextDelay, false);
}