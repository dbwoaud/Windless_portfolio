#include "Components/Enemy/EnemyInterruptComponent.h"
#include "Enemy/AI/EnemyAISettings.h"
#include "Characters/BaseCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Windless/DebugMacro.h"

namespace
{
	constexpr float InterruptMinPlayRate = 0.5f;
	constexpr float InterruptMaxPlayRate = 2.0f;
	constexpr float HitReactBlendOut = 0.15f;
	constexpr float KnockdownBlendOut = 0.25f;
}

UEnemyInterruptComponent::UEnemyInterruptComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UEnemyInterruptComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsInterrupted())
	{
		SetComponentTickEnabled(false);
		return;
	}

	elapsed += DeltaTime;
	if (current == EEnemyInterrupt::EIN_StaggerStrong)
		TickStrongPhase();

	DrawInterruptDebug();

	if (elapsed >= duration)
		EndInterrupt();
}

void UEnemyInterruptComponent::ApplyStagger(EStaggerResult result, const FVector& fromLocation)
{
	const EInterruptHitAction action = EnemyInterrupt::ResolveRehit(current, result);
	switch (action)
	{
	case EInterruptHitAction::EIHA_Begin:
		if (result == EStaggerResult::ESR_Strong)
			StartStrongStagger(action, fromLocation);
		else
			StartWeakStagger(action);
		break;

	case EInterruptHitAction::EIHA_Restart:
		StartWeakStagger(action);
		break;

	case EInterruptHitAction::EIHA_Promote:
	case EInterruptHitAction::EIHA_Relaunch:
		StartStrongStagger(action, fromLocation);
		break;

	case EInterruptHitAction::EIHA_HpOnly:
		if (bLogInterrupts)
			UE_LOG(LogWindless, Log, TEXT("[경직] %s 강한 경직 중 약한 결정"), *GetOwner()->GetName());
		break;

	default:
		break;
	}
}

void UEnemyInterruptComponent::CancelInterrupt()
{
	if (!IsInterrupted())
		return;

	RestoreRotation();
	ResetInterrupt();
	bStaggerRecovered = false;
}

bool UEnemyInterruptComponent::ConsumeStaggerRecovered()
{
	const bool bRecovered = bStaggerRecovered;
	bStaggerRecovered = false;
	return bRecovered;
}

void UEnemyInterruptComponent::StartWeakStagger(EInterruptHitAction action)
{
	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const float weakTime = settings ? settings->weakStaggerTime : 0.6f;
	const float remainingBefore = GetRemaining();

	current = EEnemyInterrupt::EIN_StaggerWeak;
	duration = weakTime;
	elapsed = 0.f;
	bStaggerRecovered = false;
	SetComponentTickEnabled(true);

	const float playRate = SyncHitReactMontage(weakTime);

	if (!bLogInterrupts)
		return;

	const FString remainingText = action == EInterruptHitAction::EIHA_Begin ? FString() : FString::Printf(TEXT(" (남은 %.2f초에서)"), remainingBefore);
	const FString montageText = playRate > 0.f ? FString::Printf(TEXT("피격 몽타주 x%.2f"), playRate) : FString(TEXT("피격 몽타주 없음"));

	UE_LOG(LogWindless, Log, TEXT("[경직] %s 끼어들기 %s: 약한 경직 %.2f초%s, %s"), *GetOwner()->GetName(), EnemyInterrupt::GetHitActionName(action),
		weakTime, *remainingText, *montageText);
}

void UEnemyInterruptComponent::EndInterrupt()
{
	const EEnemyInterrupt ended = current;
	const float endedTime = elapsed;

	StopHitReactMontage();
	StopKnockdownMontage();
	RestoreRotation();
	ResetInterrupt();
	bStaggerRecovered = true;

	if (bLogInterrupts)
		UE_LOG(LogWindless, Log, TEXT("[경직] %s %s 회복 (%.2f초) - 다음 선택 1회: 뒤로 빠지기 x2, 공격 묶음 x0.5"),
			*GetOwner()->GetName(), EnemyInterrupt::GetInterruptName(ended), endedTime);

	onInterruptEnded.ExecuteIfBound();
}

void UEnemyInterruptComponent::ResetInterrupt()
{
	current = EEnemyInterrupt::EIN_None;
	strongPhase = EStrongStaggerPhase::ESSP_Done;
	duration = 0.f;
	elapsed = 0.f;
	SetComponentTickEnabled(false);
}

void UEnemyInterruptComponent::StartStrongStagger(EInterruptHitAction action, const FVector& fromLocation)
{
	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	strongTimes = FStrongStaggerTimes();
	if (settings)
	{
		strongTimes.launch = settings->knockbackTime;
		strongTimes.down = settings->downTime;
		strongTimes.getUp = settings->getUpTime;
	}
	const float distance = settings ? settings->knockbackDistance : 3.f;
	const float remainingBefore = GetRemaining();

	if (current == EEnemyInterrupt::EIN_StaggerWeak)
		StopHitReactMontage();

	current = EEnemyInterrupt::EIN_StaggerStrong;
	duration = strongTimes.GetTotal();
	elapsed = 0.f;
	bStaggerRecovered = false;
	SetComponentTickEnabled(true);

	LockRotation();
	LaunchOwner(fromLocation, distance);

	if (bLogInterrupts)
	{
		const FString remainingText = action == EInterruptHitAction::EIHA_Begin ? FString() : FString::Printf(TEXT(" (남은 %.2f초에서)"), remainingBefore);
		UE_LOG(LogWindless, Log, TEXT("[경직] %s 끼어들기 %s: 강한 경직 %.2f초 (날아감 %.2f → 다운 %.2f → 기상 %.2f), %.1fm 날아감%s"),
			*GetOwner()->GetName(), EnemyInterrupt::GetHitActionName(action), duration,
			strongTimes.launch, strongTimes.down, strongTimes.getUp, distance, *remainingText);
	}

	EnterStrongPhase(EStrongStaggerPhase::ESSP_Launch);
}

void UEnemyInterruptComponent::TickStrongPhase()
{
	const EStrongStaggerPhase phase = EnemyInterrupt::GetStrongPhase(strongTimes, elapsed);
	if (phase != strongPhase && phase != EStrongStaggerPhase::ESSP_Done)
		EnterStrongPhase(phase);
}

void UEnemyInterruptComponent::EnterStrongPhase(EStrongStaggerPhase phase)
{
	strongPhase = phase;
	const float phaseTime = strongTimes.GetPhaseTime(phase);
	const float playRate = PlayKnockdownSection(phase, phaseTime);

	if (bLogInterrupts)
		UE_LOG(LogWindless, Log, TEXT("[경직] %s 강한 경직 %s (%.2f초 시점, %.2f초간, %s)"), *GetOwner()->GetName(), EnemyInterrupt::GetStrongPhaseName(phase),
			elapsed, phaseTime, playRate > 0.f ? *FString::Printf(TEXT("몽타주 x%.2f"), playRate) : TEXT("몽타주 없음"));
}

void UEnemyInterruptComponent::LaunchOwner(const FVector& fromLocation, float distance)
{
	ACharacter* character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* movement = character ? character->GetCharacterMovement() : nullptr;
	if (!movement)
		return;

	FVector direction = (character->GetActorLocation() - fromLocation).GetSafeNormal2D();
	if (direction.IsNearlyZero())
		direction = -character->GetActorForwardVector().GetSafeNormal2D();

	const FVector velocity = EnemyInterrupt::ComputeLaunchVelocity(direction, distance * 100.f, strongTimes.launch, movement->GetGravityZ());
	character->LaunchCharacter(velocity, true, true);
}

void UEnemyInterruptComponent::LockRotation()
{
	const ACharacter* character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* movement = character ? character->GetCharacterMovement() : nullptr;
	if (!movement || bRotationLocked)
		return;

	bSavedOrientToMovement = movement->bOrientRotationToMovement;
	movement->bOrientRotationToMovement = false;
	bRotationLocked = true;
}

void UEnemyInterruptComponent::RestoreRotation()
{
	if (!bRotationLocked)
		return;

	const ACharacter* character = Cast<ACharacter>(GetOwner());
	if (UCharacterMovementComponent* movement = character ? character->GetCharacterMovement() : nullptr)
		movement->bOrientRotationToMovement = bSavedOrientToMovement;

	bRotationLocked = false;
}

float UEnemyInterruptComponent::SyncHitReactMontage(float time)
{
	const ABaseCharacter* character = Cast<ABaseCharacter>(GetOwner());
	UAnimMontage* montage = character ? character->GetHitReactMontage() : nullptr;
	UAnimInstance* anim = GetAnimInstance();
	if (!montage || !anim || time <= 0.f || !anim->Montage_IsPlaying(montage))
		return 0.f;

	const int32 sectionIndex = montage->GetSectionIndex(anim->Montage_GetCurrentSection(montage));
	const float clipLength = sectionIndex != INDEX_NONE ? montage->GetSectionLength(sectionIndex) : montage->GetPlayLength();
	const float playRate = clipLength / time;
	anim->Montage_SetPlayRate(montage, playRate);

	if (!bWarnedMontageRate && (playRate < InterruptMinPlayRate || playRate > InterruptMaxPlayRate))
	{
		bWarnedMontageRate = true;
		UE_LOG(LogWindless, Warning, TEXT("[경직] %s 피격 몽타주 재생 속도 x%.2f (섹션 %.2f초 / 경직 %.2f초)"),
			*GetOwner()->GetName(), playRate, clipLength, time);
	}
	return playRate;
}

void UEnemyInterruptComponent::StopHitReactMontage()
{
	const ABaseCharacter* character = Cast<ABaseCharacter>(GetOwner());
	UAnimMontage* montage = character ? character->GetHitReactMontage() : nullptr;
	UAnimInstance* anim = GetAnimInstance();
	if (montage && anim && anim->Montage_IsPlaying(montage))
		anim->Montage_Stop(HitReactBlendOut, montage);
}

float UEnemyInterruptComponent::PlayKnockdownSection(EStrongStaggerPhase phase, float time)
{
	const ABaseCharacter* character = Cast<ABaseCharacter>(GetOwner());
	UAnimMontage* montage = character ? character->GetKnockdownMontage() : nullptr;
	UAnimInstance* anim = GetAnimInstance();
	const FName section = EnemyInterrupt::GetStrongSectionName(phase);
	const int32 sectionIndex = montage ? montage->GetSectionIndex(section) : INDEX_NONE;

	if (!anim || sectionIndex == INDEX_NONE || time <= 0.f)
	{
		if (!bWarnedKnockdown)
		{
			bWarnedKnockdown = true;
			UE_LOG(LogWindless, Warning, TEXT("[경직] %s 강한 경직 몽타주가 없거나 %s 섹션이 없음"),
				*GetOwner()->GetName(), *section.ToString());
		}
		return 0.f;
	}

	if (!anim->Montage_IsPlaying(montage))
		anim->Montage_Play(montage, 1.f);

	anim->Montage_JumpToSection(section, montage);

	const float clipLength = montage->GetSectionLength(sectionIndex);
	const float playRate = clipLength / time;
	anim->Montage_SetPlayRate(montage, playRate);

	if (!bWarnedMontageRate && (playRate < InterruptMinPlayRate || playRate > InterruptMaxPlayRate))
	{
		bWarnedMontageRate = true;
		UE_LOG(LogWindless, Warning, TEXT("[경직] %s 강한 경직 섹션 %s 재생 속도 x%.2f (섹션 %.2f초 / 데이터 %.2f초)"),
			*GetOwner()->GetName(), *section.ToString(), playRate, clipLength, time);
	}
	return playRate;
}

void UEnemyInterruptComponent::StopKnockdownMontage()
{
	const ABaseCharacter* character = Cast<ABaseCharacter>(GetOwner());
	UAnimMontage* montage = character ? character->GetKnockdownMontage() : nullptr;
	UAnimInstance* anim = GetAnimInstance();
	if (montage && anim && anim->Montage_IsPlaying(montage))
		anim->Montage_Stop(KnockdownBlendOut, montage);
}

UAnimInstance* UEnemyInterruptComponent::GetAnimInstance() const
{
	const ACharacter* character = Cast<ACharacter>(GetOwner());
	return character && character->GetMesh() ? character->GetMesh()->GetAnimInstance() : nullptr;
}
void UEnemyInterruptComponent::DrawInterruptDebug() const
{
	if (!bDrawInterrupt || !GEngine || !GetOwner())
		return;

	const uint64 key = 0x20000000ull + GetOwner()->GetUniqueID();
	GEngine->AddOnScreenDebugMessage(key, 0.f, FColor::Cyan, FString::Printf(TEXT("%s %s%s %.2f초"),
		*GetOwner()->GetName(), EnemyInterrupt::GetInterruptName(current),
		current == EEnemyInterrupt::EIN_StaggerStrong ? *FString::Printf(TEXT("(%s)"), EnemyInterrupt::GetStrongPhaseName(strongPhase)) : TEXT(""),
		GetRemaining()));
}
