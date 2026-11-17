#include "Enemy/AI/EnemyAttackTiming.h"
#include "Enemy/AI/EnemyAttackRow.h"

TArray<FEnemyAttackPhaseSpan> EnemyAttackTiming::BuildTimeline(const FEnemyAttackRow& attack, int32 hitIndex, float windupScale)
{
	TArray<FEnemyAttackPhaseSpan> timeline;
	float time = 0.f;

	auto addPhase = [&timeline, &time](EEnemyAttackPhase phase, float duration)
		{
			if (duration <= 0.f)
				return;

			FEnemyAttackPhaseSpan span;
			span.phase = phase;
			span.startTime = time;
			span.duration = duration;
			timeline.Add(span);
			time += duration;
		};

	const float windup = attack.GetWindup(hitIndex) * windupScale;

	if (hitIndex == 0 && attack.HasHold())
	{
		const float rise = FMath::Max(0.f, windup - attack.windupHoldTime - attack.holdReleaseTime);
		addPhase(EEnemyAttackPhase::EAP_Windup, rise);
		addPhase(EEnemyAttackPhase::EAP_Hold, attack.windupHoldTime);
		addPhase(EEnemyAttackPhase::EAP_Release, attack.holdReleaseTime);
	}
	else
	{
		addPhase(EEnemyAttackPhase::EAP_Windup, windup);
	}

	addPhase(EEnemyAttackPhase::EAP_Active, attack.GetActiveTime(hitIndex));
	if (hitIndex >= attack.GetHitCount() - 1)
		addPhase(EEnemyAttackPhase::EAP_Recovery, attack.recovery);

	return timeline;
}

float EnemyAttackTiming::GetTotalDuration(const TArray<FEnemyAttackPhaseSpan>& timeline)
{
	return timeline.Num() > 0 ? timeline.Last().GetEndTime() : 0.f;
}

FFloatInterval EnemyAttackTiming::GetRollWindow(const FEnemyAttackRow& attack, float invincibleTime, int32 hitIndex)
{
	const float windupEnd = attack.GetWindup(hitIndex);
	const float active = attack.GetActiveTime(hitIndex);
	return FFloatInterval(windupEnd - FMath::Max(0.f, invincibleTime - active), windupEnd);
}

float EnemyAttackTiming::GetTrackScale(EEnemyTrackProfile profile, float windup, float timeInWindup)
{
	const float lockTime = windup - TrackLockLeadTime;
	if (timeInWindup >= lockTime)
		return 0.f;

	switch (profile)
	{
	case EEnemyTrackProfile::ETP_Strong:
		return 1.f;

	case EEnemyTrackProfile::ETP_None:
		return 0.f;

	case EEnemyTrackProfile::ETP_Decay:
	default:
	{
		const float half = windup * 0.5f;
		if (timeInWindup < half || lockTime <= half)
			return 1.f;

		return FMath::Clamp((lockTime - timeInWindup) / (lockTime - half), 0.f, 1.f);
	}
	}
}

float EnemyAttackTiming::GetRecoveryTurnScale(float timeInRecovery)
{
	return FMath::Clamp(timeInRecovery / RecoveryTurnRestoreTime, 0.f, 1.f);
}

float EnemyAttackTiming::GetTurnScale(const FEnemyAttackRow& attack, const TArray<FEnemyAttackPhaseSpan>& timeline, float elapsed, int32 hitIndex)
{
	if (timeline.Num() == 0)
		return 1.f;

	const float windupStart = timeline[0].startTime;
	float activeStart = -1.f;
	float recoveryStart = -1.f;
	for (const FEnemyAttackPhaseSpan& span : timeline)
	{
		if (span.phase == EEnemyAttackPhase::EAP_Active && activeStart < 0.f)
			activeStart = span.startTime;
		else if (span.phase == EEnemyAttackPhase::EAP_Recovery)
			recoveryStart = span.startTime;
	}

	if (activeStart < 0.f)
		return 1.f;

	if (elapsed < activeStart)
		return GetTrackScale(attack.GetTrackProfile(hitIndex), activeStart - windupStart, elapsed - windupStart);

	if (recoveryStart < 0.f || elapsed < recoveryStart)
		return 0.f;

	return GetRecoveryTurnScale(elapsed - recoveryStart);
}

FName EnemyAttackTiming::GetPhaseSectionName(EEnemyAttackPhase phase, FName prefix)
{
	const TCHAR* baseName = TEXT("None");
	switch (phase)
	{
	case EEnemyAttackPhase::EAP_Windup:   baseName = TEXT("Windup"); break;
	case EEnemyAttackPhase::EAP_Hold:     baseName = TEXT("Hold"); break;
	case EEnemyAttackPhase::EAP_Release:  baseName = TEXT("Release"); break;
	case EEnemyAttackPhase::EAP_Active:   baseName = TEXT("Active"); break;
	case EEnemyAttackPhase::EAP_Recovery: baseName = TEXT("Recovery"); break;
	default: break;
	}

	return prefix.IsNone() ? FName(baseName) : FName(*FString::Printf(TEXT("%s_%s"), *prefix.ToString(), baseName));
}

const TCHAR* EnemyAttackTiming::GetPhaseName(EEnemyAttackPhase phase)
{
	switch (phase)
	{
	case EEnemyAttackPhase::EAP_Windup:   return TEXT("예고");
	case EEnemyAttackPhase::EAP_PreTurn:  return TEXT("선회");
	case EEnemyAttackPhase::EAP_Hold:     return TEXT("멈춤");
	case EEnemyAttackPhase::EAP_Release:  return TEXT("내려옴");
	case EEnemyAttackPhase::EAP_Active:   return TEXT("판정");
	case EEnemyAttackPhase::EAP_Recovery: return TEXT("후딜");
	default:                              return TEXT("없음");
	}
}

const TCHAR* EnemyAttackTiming::GetTrackProfileName(EEnemyTrackProfile profile)
{
	switch (profile)
	{
	case EEnemyTrackProfile::ETP_Strong: return TEXT("끝까지");
	case EEnemyTrackProfile::ETP_None:   return TEXT("없음");
	default:                             return TEXT("점점 덜");
	}
}