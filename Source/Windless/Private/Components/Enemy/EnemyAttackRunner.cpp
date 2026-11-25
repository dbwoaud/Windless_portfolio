#include "Components/Enemy/EnemyAttackRunner.h"
#include "Enemy/AI/EnemyAttackRow.h"
#include "Enemy/AI/EnemyAISettings.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "Windless/DebugMacro.h"
#include "DrawDebugHelpers.h"

namespace
{
	constexpr float RunnerMinPlayRate = 0.5f;
	constexpr float RunnerMaxPlayRate = 2.0f;

	constexpr float PhaseBarCmPerSecond = 80.f; // 구간 막대 길이
	bool bWarnedNoWindupAttenuation = false;    // 감쇠 미지정 경고

	FColor GetPhaseColor(EEnemyAttackPhase phase)
	{
		switch (phase)
		{
		case EEnemyAttackPhase::EAP_Windup:   return FColor::Yellow;
		case EEnemyAttackPhase::EAP_Hold:     return FColor::Orange;
		case EEnemyAttackPhase::EAP_Release:  return FColor::Magenta;
		case EEnemyAttackPhase::EAP_Active:   return FColor::Red;
		case EEnemyAttackPhase::EAP_Recovery: return FColor(80, 140, 255);
		default:                              return FColor::White;
		}
	}
}

UEnemyAttackRunner::UEnemyAttackRunner()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UEnemyAttackRunner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsRunning())
		return;

	if (bPreTurning)
	{
		TickPreTurn(DeltaTime);
		DrawPhaseBarDebug();
		return;
	}

	elapsed += DeltaTime;
	TickTracking(DeltaTime);
	while (IsRunning() && timeline.IsValidIndex(phaseIndex) && elapsed >= timeline[phaseIndex].GetEndTime())
	{
		ExitCurrentPhase();

		if (phaseIndex + 1 < timeline.Num())
			EnterPhase(phaseIndex + 1);
		else
			FinishAttack(EEnemyAttackEnd::EAE_Completed);
	}

	DrawPhaseBarDebug();
}

bool UEnemyAttackRunner::StartAttack(const FEnemyAttackRow& attack, FName attackId, AActor* target, float inTurnRate)
{
	if (IsRunning())
		return false;

	timeline = EnemyAttackTiming::BuildTimeline(attack);
	if (timeline.Num() == 0)
		return false;

	currentAttack = &attack;
	currentAttackId = attackId;
	currentTarget = target;
	turnRate = FMath::Max(0.f, inTurnRate);
	elapsed = 0.f;
	phaseIndex = INDEX_NONE;
	preTurnElapsed = 0.f;
	bLockLogged = false;
	SetComponentTickEnabled(true);

	/* 선회 */
	const float yawToTarget = GetYawToTarget();
	if (target && yawToTarget > attack.attackAngleH)
	{
		bPreTurning = true;
		if (bLogPhases)
			UE_LOG(LogWindless, Log, TEXT("[적 공격] %s %s 선회 시작 - 대상 %.0f° (발동 ±%.0f° · 제한 %.1f초 · 회전 %.0f°/초)"),
				*GetOwner()->GetName(), *attackId.ToString(), yawToTarget, attack.attackAngleH, attack.preTurnTime, turnRate);
		return true;
	}

	CommitAttack();
	return true;
}

void UEnemyAttackRunner::CancelAttack(const TCHAR* reason)
{
	if (!IsRunning())
		return;

	if (bLogPhases)
		UE_LOG(LogWindless, Log, TEXT("[적 공격] %s %s 취소 (%s, %.2f초 시점 %s 구간)"), *GetOwner()->GetName(), *currentAttackId.ToString(),
			reason, bPreTurning ? preTurnElapsed : elapsed, EnemyAttackTiming::GetPhaseName(GetPhase()));

	FinishAttack(EEnemyAttackEnd::EAE_Cancelled);
}

void UEnemyAttackRunner::CommitAttack()
{
	const FEnemyAttackRow& attack = *currentAttack;

	if (attack.GetHitCount() > 1)
		UE_LOG(LogWindless, Warning, TEXT("[적 공격] %s %s 연타 %d타"), *GetOwner()->GetName(), *currentAttackId.ToString(), attack.GetHitCount());

	PlayWindupSound();
	StartMontage();

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	rollWindow = EnemyAttackTiming::GetRollWindow(attack, settings ? settings->playerRollInvincibleTime : 0.35f);

	if (bLogPhases)
	{
		FString phaseText;
		for (const FEnemyAttackPhaseSpan& span : timeline)
			phaseText += FString::Printf(TEXT("%s%s %.2f"), phaseText.IsEmpty() ? TEXT("") : TEXT(" → "), EnemyAttackTiming::GetPhaseName(span.phase), span.duration);

		UE_LOG(LogWindless, Log, TEXT("[적 공격] %s %s 시작: %s (합계 %.2f초), 구르기 성공 %.2f~%.2f초, 추적 %s, 몽타주 %s"),
			*GetOwner()->GetName(), *currentAttackId.ToString(), *phaseText, EnemyAttackTiming::GetTotalDuration(timeline),
			rollWindow.Min, rollWindow.Max, EnemyAttackTiming::GetTrackProfileName(attack.GetTrackProfile(0)),
			!currentMontage ? TEXT("없음") : (bPhaseSections ? TEXT("구간 섹션") : TEXT("섹션 하나를 전체 시간에 맞춤")));
		UE_LOG(LogWindless, Log, TEXT("[적 공격] %s 공격자 등록 (%s)"), *GetOwner()->GetName(), *currentAttackId.ToString());
	}

	onAttackerChanged.Broadcast(this, true);
	EnterPhase(0);
}

void UEnemyAttackRunner::EnterPhase(int32 index)
{
	phaseIndex = index;
	const FEnemyAttackPhaseSpan& span = timeline[index];

	if (bLogPhases)
		UE_LOG(LogWindless, Log, TEXT("[적 공격] %s %s %s 시작 (%.2f초 시점, %.2f초간)"), *GetOwner()->GetName(), *currentAttackId.ToString(),
			EnemyAttackTiming::GetPhaseName(span.phase), span.startTime, span.duration);

	SyncPhaseMontage(index);

	if (span.phase == EEnemyAttackPhase::EAP_Active)
		onHitboxChanged.ExecuteIfBound(true);
}

void UEnemyAttackRunner::ExitCurrentPhase()
{
	if (GetPhase() == EEnemyAttackPhase::EAP_Active)
		onHitboxChanged.ExecuteIfBound(false);
}

void UEnemyAttackRunner::FinishAttack(EEnemyAttackEnd endReason)
{
	if (!IsRunning())
		return;

	const bool bWasAttacker = IsCommitted();

	if (GetPhase() == EEnemyAttackPhase::EAP_Active)
		onHitboxChanged.ExecuteIfBound(false);

	UAnimInstance* anim = GetAnimInstance();
	if (anim && currentMontage && anim->Montage_IsPlaying(currentMontage))
		anim->Montage_Stop(endReason == EEnemyAttackEnd::EAE_Completed ? 0.25f : 0.1f, currentMontage);

	if (bLogPhases && endReason == EEnemyAttackEnd::EAE_Completed)
		UE_LOG(LogWindless, Log, TEXT("[적 공격] %s %s 끝 (%.2f초)"), *GetOwner()->GetName(), *currentAttackId.ToString(), elapsed);

	currentAttack = nullptr;
	currentAttackId = NAME_None;
	currentTarget.Reset();
	currentMontage = nullptr;
	timeline.Reset();
	phaseIndex = INDEX_NONE;
	bPreTurning = false;
	preTurnElapsed = 0.f;
	SetComponentTickEnabled(false);

	if (bWasAttacker)
	{
		if (bLogPhases)
			UE_LOG(LogWindless, Log, TEXT("[적 공격] %s 공격자 해제"), *GetOwner()->GetName());

		onAttackerChanged.Broadcast(this, false);
	}
	onAttackFinished.ExecuteIfBound(endReason);
}

void UEnemyAttackRunner::TickPreTurn(float DeltaTime)
{
	preTurnElapsed += DeltaTime;
	TurnOwnerTowardTarget(turnRate * DeltaTime);
	DrawFacingDebug(-1.f);

	const float yawToTarget = GetYawToTarget();
	if (!currentTarget.IsValid() || yawToTarget <= currentAttack->attackAngleH) // 선회 발동 각도 안
	{
		if (bLogPhases)
			UE_LOG(LogWindless, Log, TEXT("[적 공격] %s %s 선회 끝 (%.2f초, 남은 각도 %.0f)"),
				*GetOwner()->GetName(), *currentAttackId.ToString(), preTurnElapsed, yawToTarget);

		bPreTurning = false;
		CommitAttack();
		return;
	}

	if (preTurnElapsed >= currentAttack->preTurnTime) // 선회 제한 시간 초과
	{
		if (bLogPhases)
			UE_LOG(LogWindless, Log, TEXT("[적 공격] %s %s 선회 실패: %.1f초 안에 못 돌아섬 (남은 각도 %.0f)"),
				*GetOwner()->GetName(), *currentAttackId.ToString(), currentAttack->preTurnTime, yawToTarget);

		FinishAttack(EEnemyAttackEnd::EAE_TurnFailed);
	}
}

void UEnemyAttackRunner::TickTracking(float DeltaTime)
{
	const float turnScale = EnemyAttackTiming::GetTurnScale(*currentAttack, timeline, elapsed);
	TurnOwnerTowardTarget(turnRate * turnScale * DeltaTime);
	DrawFacingDebug(turnScale);

	const EEnemyAttackPhase phase = GetPhase();
	const bool bWindupGroup = phase == EEnemyAttackPhase::EAP_Windup || phase == EEnemyAttackPhase::EAP_Hold || phase == EEnemyAttackPhase::EAP_Release;
	if (bLogPhases && !bLockLogged && bWindupGroup && turnScale <= 0.f)
	{
		bLockLogged = true;
		UE_LOG(LogWindless, Log, TEXT("[적 공격] %s %s 방향 고정 (%.2f초 시점, 판정 %.2f초 전)"),
			*GetOwner()->GetName(), *currentAttackId.ToString(), elapsed, GetActiveStartTime() - elapsed);
	}
}

void UEnemyAttackRunner::TurnOwnerTowardTarget(float maxDegrees)
{
	AActor* owner = GetOwner();
	const AActor* target = currentTarget.Get();
	if (!owner || !target || maxDegrees <= 0.f)
		return;

	const FVector toTarget = (target->GetActorLocation() - owner->GetActorLocation()).GetSafeNormal2D();
	if (toTarget.IsNearlyZero())
		return;

	FRotator rotation = owner->GetActorRotation();
	rotation.Yaw = FMath::FixedTurn(rotation.Yaw, toTarget.Rotation().Yaw, maxDegrees);
	owner->SetActorRotation(rotation);
}

float UEnemyAttackRunner::GetYawToTarget() const
{
	const AActor* owner = GetOwner();
	const AActor* target = currentTarget.Get();
	if (!owner || !target)
		return 0.f;

	const FVector toTarget = (target->GetActorLocation() - owner->GetActorLocation()).GetSafeNormal2D();
	if (toTarget.IsNearlyZero())
		return 0.f;

	return FMath::Abs(FMath::FindDeltaAngleDegrees(owner->GetActorRotation().Yaw, toTarget.Rotation().Yaw));
}

float UEnemyAttackRunner::GetActiveStartTime() const
{
	for (const FEnemyAttackPhaseSpan& span : timeline)
	{
		if (span.phase == EEnemyAttackPhase::EAP_Active)
			return span.startTime;
	}
	return 0.f;
}

void UEnemyAttackRunner::StartMontage()
{
	currentMontage = currentAttack->montage.LoadSynchronous();
	bPhaseSections = false;

	UAnimInstance* anim = GetAnimInstance();
	if (!currentMontage || !anim)
		return;

	const FName windupSection = EnemyAttackTiming::GetPhaseSectionName(EEnemyAttackPhase::EAP_Windup, currentAttack->montageSection);
	bPhaseSections = currentMontage->GetSectionIndex(windupSection) != INDEX_NONE;

	anim->Montage_Play(currentMontage, 1.f);
	if (bPhaseSections) // 구간 섹션 방식
		return;

	/* 구간 섹션이 없으면 섹션 하나 또는 몽타주 전체를 공격 전체 시간에 맞추기 */
	float clipLength = currentMontage->GetPlayLength();
	if (!currentAttack->montageSection.IsNone())
	{
		const int32 sectionIndex = currentMontage->GetSectionIndex(currentAttack->montageSection);
		if (sectionIndex != INDEX_NONE)
		{
			anim->Montage_JumpToSection(currentAttack->montageSection, currentMontage);
			clipLength = currentMontage->GetSectionLength(sectionIndex);
		}
	}

	const float total = EnemyAttackTiming::GetTotalDuration(timeline);
	anim->Montage_SetPlayRate(currentMontage, total > 0.f ? clipLength / total : 1.f);
}

void UEnemyAttackRunner::SyncPhaseMontage(int32 index)
{
	UAnimInstance* anim = GetAnimInstance();
	if (!bPhaseSections || !anim || !currentMontage)
		return;

	const FName prefix = currentAttack->montageSection;
	const FName section = EnemyAttackTiming::GetPhaseSectionName(timeline[index].phase, prefix);
	const int32 sectionIndex = currentMontage->GetSectionIndex(section);
	if (sectionIndex == INDEX_NONE)
		return;

	float duration = timeline[index].duration;
	for (int32 next = index + 1; next < timeline.Num(); ++next)
	{
		if (currentMontage->GetSectionIndex(EnemyAttackTiming::GetPhaseSectionName(timeline[next].phase, prefix)) != INDEX_NONE)
			break;
		duration += timeline[next].duration;
	}

	const float sectionLength = currentMontage->GetSectionLength(sectionIndex);
	const float playRate = duration > 0.f ? sectionLength / duration : 1.f;

	anim->Montage_JumpToSection(section, currentMontage);
	anim->Montage_SetPlayRate(currentMontage, playRate);

	if (playRate < RunnerMinPlayRate || playRate > RunnerMaxPlayRate)
		UE_LOG(LogWindless, Warning, TEXT("[적 공격] %s %s 섹션 %s 재생 속도 x%.2f (섹션 %.2f초 / 데이터 %.2f초) - 애니메이션 길이를 데이터에 가깝게"),
			*GetOwner()->GetName(), *currentAttackId.ToString(), *section.ToString(), playRate, sectionLength, duration);
}

UAnimInstance* UEnemyAttackRunner::GetAnimInstance() const
{
	const ACharacter* character = Cast<ACharacter>(GetOwner());
	return character && character->GetMesh() ? character->GetMesh()->GetAnimInstance() : nullptr;
}

void UEnemyAttackRunner::PlayWindupSound()
{
	USoundBase* windupSound = currentAttack->windupSound.LoadSynchronous();
	if (!windupSound)
		return;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	USoundAttenuation* attenuation = settings ? settings->windupSoundAttenuation.LoadSynchronous() : nullptr;
	if (!attenuation && !bWarnedNoWindupAttenuation)
	{
		bWarnedNoWindupAttenuation = true;
		UE_LOG(LogWindless, Warning, TEXT("[적 공격] 예고 효과음 감쇠 미지정"));
	}

	UGameplayStatics::SpawnSoundAttached(windupSound, GetOwner()->GetRootComponent(), NAME_None, FVector::ZeroVector,
		EAttachLocation::KeepRelativeOffset, true, 1.f, 1.f, 0.f, attenuation);
}

void UEnemyAttackRunner::DrawFacingDebug(float turnScale) const
{
#if ENABLE_DRAW_DEBUG
	const AActor* owner = GetOwner();
	if (!bDrawFacing || !owner || !GetWorld())
		return;

	const FColor color = turnScale < 0.f ? FColor::Orange
		: (turnScale >= 1.f ? FColor::Green
			: (turnScale > 0.f ? FColor::Yellow : FColor::Red));
	const FVector start = owner->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	DrawDebugDirectionalArrow(GetWorld(), start, start + owner->GetActorForwardVector() * 200.f, 40.f, color, false, -1.f, 0, 3.f);
#endif
}

void UEnemyAttackRunner::DrawPhaseBarDebug() const
{
#if ENABLE_DRAW_DEBUG
	const AActor* owner = GetOwner();
	UWorld* world = GetWorld();
	if (!bDrawPhaseBar || !IsRunning() || !owner || !world)
		return;

	FVector right = owner->GetActorRightVector();
	if (const APlayerCameraManager* camera = UGameplayStatics::GetPlayerCameraManager(world, 0))
		right = FRotationMatrix(camera->GetCameraRotation()).GetScaledAxis(EAxis::Y);
	right.Z = 0.f;
	right = right.GetSafeNormal();
	if (right.IsNearlyZero())
		right = owner->GetActorRightVector();

	const float total = EnemyAttackTiming::GetTotalDuration(timeline);
	const FVector origin = owner->GetActorLocation() + FVector(0.f, 0.f, owner->GetSimpleCollisionHalfHeight() + 50.f);
	const FVector barStart = origin - right * (total * PhaseBarCmPerSecond * 0.5f);
	auto pointAt = [&barStart, &right](float time) { return barStart + right * (time * PhaseBarCmPerSecond); };

	/* 구간 막대(노랑 예고, 주황 멈춤, 보라 내려옴, 빨강 판정, 파랑 후딜) */
	for (const FEnemyAttackPhaseSpan& span : timeline)
		DrawDebugLine(world, pointAt(span.startTime), pointAt(span.GetEndTime()), GetPhaseColor(span.phase), false, -1.f, 0, 6.f);

	/* 구르기 성공 구간(막대 아래 초록) */
	const FVector below(0.f, 0.f, -12.f);
	DrawDebugLine(world, pointAt(rollWindow.Min) + below, pointAt(rollWindow.Max) + below, FColor::Green, false, -1.f, 0, 4.f);

	/* 현재 시각(흰 세로선) */
	if (!bPreTurning)
		DrawDebugLine(world, pointAt(elapsed) + FVector(0.f, 0.f, 14.f), pointAt(elapsed) + FVector(0.f, 0.f, -22.f), FColor::White, false, -1.f, 0, 2.f);

	/* 화면 글자 */
	if (GEngine)
	{
		const uint64 messageKey = 0x10000000ull + owner->GetUniqueID();
		FString text;
		FColor color = FColor::White;
		if (bPreTurning)
		{
			text = FString::Printf(TEXT("%s %s  선회 %.2f / %.1f초"), *owner->GetName(), *currentAttackId.ToString(), preTurnElapsed, currentAttack->preTurnTime);
			color = FColor::Orange;
		}
		else
		{
			const TCHAR* rollState = elapsed < rollWindow.Min ? TEXT("지금 구르면 빠름") : (elapsed <= rollWindow.Max ? TEXT("지금 구르면 성공") : TEXT("구르기 늦음"));
			color = elapsed < rollWindow.Min ? FColor::Yellow : (elapsed <= rollWindow.Max ? FColor::Green : FColor::Silver);
			text = FString::Printf(TEXT("%s %s  %s %.2f / %.2f초  · 구르기 %.2f~%.2f초 · %s"), *owner->GetName(), *currentAttackId.ToString(),
				EnemyAttackTiming::GetPhaseName(GetPhase()), elapsed, total, rollWindow.Min, rollWindow.Max, rollState);
		}
		GEngine->AddOnScreenDebugMessage(messageKey, 0.f, color, text);
	}
#endif
}

EEnemyAttackPhase UEnemyAttackRunner::GetPhase() const
{
	if (bPreTurning)
		return EEnemyAttackPhase::EAP_PreTurn;

	return timeline.IsValidIndex(phaseIndex) ? timeline[phaseIndex].phase : EEnemyAttackPhase::EAP_None;
}
