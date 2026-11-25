#include "Enemy/AI/EnemyInterrupt.h"

EEnemyInterrupt EnemyInterrupt::ToInterrupt(EStaggerResult result)
{
	switch (result)
	{
	case EStaggerResult::ESR_Weak:   return EEnemyInterrupt::EIN_StaggerWeak;
	case EStaggerResult::ESR_Strong: return EEnemyInterrupt::EIN_StaggerStrong;
	default:                         return EEnemyInterrupt::EIN_None;
	}
}

EInterruptHitAction EnemyInterrupt::ResolveRehit(EEnemyInterrupt current, EStaggerResult result)
{
	const EEnemyInterrupt incoming = ToInterrupt(result);
	if (incoming == EEnemyInterrupt::EIN_None)
		return EInterruptHitAction::EIHA_None;

	switch (current)
	{
	case EEnemyInterrupt::EIN_StaggerWeak:
		return incoming == EEnemyInterrupt::EIN_StaggerStrong ? EInterruptHitAction::EIHA_Promote : EInterruptHitAction::EIHA_Restart;

	case EEnemyInterrupt::EIN_StaggerStrong:
		return incoming == EEnemyInterrupt::EIN_StaggerStrong ? EInterruptHitAction::EIHA_Relaunch : EInterruptHitAction::EIHA_HpOnly;

	default:
		return EInterruptHitAction::EIHA_Begin;
	}
}

EStrongStaggerPhase EnemyInterrupt::GetStrongPhase(const FStrongStaggerTimes& times, float elapsed)
{
	if (elapsed < times.launch)
		return EStrongStaggerPhase::ESSP_Launch;

	if (elapsed < times.launch + times.down)
		return EStrongStaggerPhase::ESSP_Down;

	if (elapsed < times.GetTotal())
		return EStrongStaggerPhase::ESSP_GetUp;

	return EStrongStaggerPhase::ESSP_Done;
}

FVector EnemyInterrupt::ComputeLaunchVelocity(const FVector& direction, float distance, float time, float gravityZ)
{
	if (time <= 0.f)
		return FVector::ZeroVector;

	const FVector horizontal = direction.GetSafeNormal2D() * (distance / time);
	const float vertical = FMath::Abs(gravityZ) * time * 0.5f;
	return FVector(horizontal.X, horizontal.Y, vertical);
}

FName EnemyInterrupt::GetStrongSectionName(EStrongStaggerPhase phase)
{
	switch (phase)
	{
	case EStrongStaggerPhase::ESSP_Launch: return FName("Launch");
	case EStrongStaggerPhase::ESSP_Down:   return FName("Down");
	case EStrongStaggerPhase::ESSP_GetUp:  return FName("GetUp");
	default:                               return NAME_None;
	}
}

const TCHAR* EnemyInterrupt::GetInterruptName(EEnemyInterrupt interrupt)
{
	switch (interrupt)
	{
	case EEnemyInterrupt::EIN_StaggerWeak:   return TEXT("약한 경직");
	case EEnemyInterrupt::EIN_StaggerStrong: return TEXT("강한 경직");
	default:                                 return TEXT("없음");
	}
}

const TCHAR* EnemyInterrupt::GetHitActionName(EInterruptHitAction action)
{
	switch (action)
	{
	case EInterruptHitAction::EIHA_Begin:    return TEXT("시작");
	case EInterruptHitAction::EIHA_Restart:  return TEXT("다시 시작");
	case EInterruptHitAction::EIHA_Promote:  return TEXT("강한 경직으로 승격");
	case EInterruptHitAction::EIHA_Relaunch: return TEXT("다시 날림");
	case EInterruptHitAction::EIHA_HpOnly:   return TEXT("HP만 감소");
	default:                                 return TEXT("변화 없음");
	}
}

const TCHAR* EnemyInterrupt::GetStrongPhaseName(EStrongStaggerPhase phase)
{
	switch (phase)
	{
	case EStrongStaggerPhase::ESSP_Launch: return TEXT("날아감");
	case EStrongStaggerPhase::ESSP_Down:   return TEXT("다운");
	case EStrongStaggerPhase::ESSP_GetUp:  return TEXT("기상");
	default:                               return TEXT("끝");
	}
}
