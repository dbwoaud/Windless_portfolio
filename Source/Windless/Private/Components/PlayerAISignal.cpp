#include "Components/PlayerAISignal.h"
#include "Enemy/AI/EnemyAISettings.h"
#include "Enemy/AI/EnemyCardScore.h"
#include "TimerManager.h"
#include "Windless/DebugMacro.h"

UPlayerAISignal::UPlayerAISignal()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerAISignal::NotifyAttackBegin()
{
	bAttackBegin = true;
	bAttackActive = false;
	bHitThisSwing = false;
	bWhiff = false;
	PushEvent(EPlayerAIState::EPS_AttackBegin);
}

void UPlayerAISignal::NotifyAttackActive(bool bActive)
{
	if (bActive)
	{
		bAttackBegin = false;
		bAttackActive = true;
		bHitThisSwing = false;
		return;
	}

	if (!bAttackActive)
		return;

	bAttackActive = false;
	bWhiff = !bHitThisSwing;
	if (bWhiff)
		PushEvent(EPlayerAIState::EPS_Whiff);
}

void UPlayerAISignal::NotifyAttackHit()
{
	if (bAttackActive)
		bHitThisSwing = true;
}

void UPlayerAISignal::NotifyAttackEnd()
{
	bAttackBegin = false;
	bAttackActive = false;
	bHitThisSwing = false;
	bWhiff = false;
}

void UPlayerAISignal::NotifyItemUse(bool bUsing)
{
	if (bUsingItem == bUsing)
		return;

	bUsingItem = bUsing;
	if (bUsing)
		PushEvent(EPlayerAIState::EPS_ItemUse);
}

void UPlayerAISignal::NotifyAim(bool bInAiming)
{
	if (bAiming == bInAiming)
		return;

	bAiming = bInAiming;
	if (bInAiming)
		PushEvent(EPlayerAIState::EPS_BowAim);
}

void UPlayerAISignal::NotifyGuard(bool bGuarding)
{
	UWorld* world = GetWorld();
	if (!world)
		return;

	if (bGuarding)
	{
		if (guardStartTime >= 0.f)
			return;

		guardStartTime = GetNow();
		const UEnemyAISettings* settings = UEnemyAISettings::Get();
		const float threshold = settings ? settings->guardHoldThreshold : 2.f;
		world->GetTimerManager().SetTimer(guardHeldTimer, this, &UPlayerAISignal::HandleGuardHeld, FMath::Max(threshold, KINDA_SMALL_NUMBER), false);
		return;
	}

	guardStartTime = -1.f;
	world->GetTimerManager().ClearTimer(guardHeldTimer);
}

FPlayerAISnapshot UPlayerAISignal::GetSnapshot() const
{
	FPlayerAISnapshot snapshot;
	snapshot.bAttackBegin = bAttackBegin || IsFaking(EPlayerAIState::EPS_AttackBegin);
	snapshot.bWhiff = bWhiff || IsFaking(EPlayerAIState::EPS_Whiff);
	snapshot.bUsingItem = bUsingItem || IsFaking(EPlayerAIState::EPS_ItemUse);
	snapshot.bAiming = bAiming || IsFaking(EPlayerAIState::EPS_BowAim);
	snapshot.guardHoldTime = IsFaking(EPlayerAIState::EPS_GuardHold) ? 999.f : GetGuardHoldTime();
	return snapshot;
}

float UPlayerAISignal::GetGuardHoldTime() const
{
	return guardStartTime >= 0.f ? GetNow() - guardStartTime : 0.f;
}

void UPlayerAISignal::FakeSignal(EPlayerAIState state, float duration)
{
	fakeUntil.Add(static_cast<uint8>(state), GetNow() + duration);
	UE_LOG(LogWindless, Log, TEXT("[AI 신호] 가짜 신호 %s (%.1f초)"), EnemyCardScore::GetPlayerStateName(state), duration);
	PushEvent(state);
}

void UPlayerAISignal::PushEvent(EPlayerAIState state)
{
	++eventSerial;
	lastEvent = state;
}

void UPlayerAISignal::HandleGuardHeld()
{
	PushEvent(EPlayerAIState::EPS_GuardHold);
}

float UPlayerAISignal::GetNow() const
{
	return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}

bool UPlayerAISignal::IsFaking(EPlayerAIState state) const
{
	const float* until = fakeUntil.Find(static_cast<uint8>(state));
	return until && GetNow() < *until;
}
