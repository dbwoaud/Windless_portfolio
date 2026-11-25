#include "Level/BossRoom.h"
#include "Level/TriggerVolumeBase.h"
#include "Enemy/Boss.h"
#include "Characters/WindlessCharacter.h"
#include "Windless/DebugMacro.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

ABossRoom::ABossRoom()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABossRoom::SealRoom()
{
	if (bSealed || bBossDefeated) // 이미 봉쇄 중이거나 보스가 죽었으면 무시
		return;

	bSealed = true;
	SetBarriersActive(true);
	BindCombatEndEvents();

	UE_LOG(LogWindless, Warning, TEXT("보스방 봉쇄: %s"), *GetName());
	OnRoomSealedBP();
	onRoomSealed.Broadcast();
}

void ABossRoom::UnsealRoom()
{
	if (!bSealed) // 봉쇄 중이 아니면 무시
		return;

	bSealed = false;
	SetBarriersActive(false);
	UnbindCombatEndEvents();

	if (!bBossDefeated && entryTrigger)
		entryTrigger->ResetTrigger();

	UE_LOG(LogWindless, Warning, TEXT("보스방 해제: %s"), *GetName());
	OnRoomUnsealedBP();
	onRoomUnsealed.Broadcast();
}

void ABossRoom::BeginPlay()
{
	Super::BeginPlay();

	if (bStartUnsealed)
		SetBarriersActive(false);

	if (entryTrigger)
		entryTrigger->onTriggerEnter.AddDynamic(this, &ABossRoom::HandleEntryTriggerEnter);
}

void ABossRoom::HandleEntryTriggerEnter(AActor* activator)
{
	cachedPlayer = Cast<AWindlessCharacter>(activator);
	SealRoom();
	if (boss)
		boss->ShowBossHealthBar();
}

void ABossRoom::HandleBossDeath(ABoss* deadBoss)
{
	bBossDefeated = true;
	UnsealRoom();
	if (deadBoss)
		deadBoss->HideBossHealthBar();
}

void ABossRoom::HandlePlayerDeath()
{
	UnsealRoom();
	if (boss)
		boss->HideBossHealthBar();
}

void ABossRoom::SetBarriersActive(bool bActive)
{
	for (AActor* barrier : barrierActors)
	{
		if (!barrier)
			continue;

		barrier->SetActorHiddenInGame(!bActive);
		barrier->SetActorEnableCollision(bActive);
	}
}

void ABossRoom::BindCombatEndEvents()
{
	if (boss)
		boss->onBossDeath.AddDynamic(this, &ABossRoom::HandleBossDeath);

	if (!cachedPlayer)
		cachedPlayer = Cast<AWindlessCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (cachedPlayer)
		cachedPlayer->onPlayerDeath.AddDynamic(this, &ABossRoom::HandlePlayerDeath);
}

void ABossRoom::UnbindCombatEndEvents()
{
	if (boss)
		boss->onBossDeath.RemoveDynamic(this, &ABossRoom::HandleBossDeath);

	if (cachedPlayer)
		cachedPlayer->onPlayerDeath.RemoveDynamic(this, &ABossRoom::HandlePlayerDeath);
}