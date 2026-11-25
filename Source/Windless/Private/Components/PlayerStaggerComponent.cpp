#include "Components/PlayerStaggerComponent.h"
#include "Enemy/AI/EnemyAISettings.h"
#include "Characters/BaseCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Windless/DebugMacro.h"

namespace
{
	constexpr float StaggerMinPlayRate = 0.5f;
	constexpr float StaggerMaxPlayRate = 2.0f;
	constexpr float GetUpBlendOut = 0.25f;
	constexpr float RollGetUpBlendOut = 0.1f;
}

UPlayerStaggerComponent::UPlayerStaggerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UPlayerStaggerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsStrongStaggered())
	{
		SetComponentTickEnabled(false);
		return;
	}

	elapsed += DeltaTime;
	const EStrongStaggerPhase nextPhase = EnemyInterrupt::GetStrongPhase(times, elapsed);
	if (nextPhase == EStrongStaggerPhase::ESSP_Done)
	{
		FinishStagger(TEXT("기상 끝"), GetUpBlendOut);
		return;
	}

	if (nextPhase != phase)
		EnterPhase(nextPhase);

	DrawStaggerDebug();
}

void UPlayerStaggerComponent::BeginStrongStagger(const FVector& fromLocation)
{
	if (IsStrongStaggered())
		return;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	times = FStrongStaggerTimes();
	if (settings)
	{
		times.launch = settings->knockbackTime;
		times.down = settings->downTime;
		times.getUp = settings->getUpTime;
	}
	const float distance = settings ? settings->knockbackDistance : 3.f;

	elapsed = 0.f;
	SetComponentTickEnabled(true);
	LaunchOwner(fromLocation, distance);

	if (bLogStagger)
		UE_LOG(LogWindless, Log, TEXT("[경직] %s 강한 경직 시작: %.2f초 (날아감 %.2f → 다운 %.2f → 기상 %.2f), %.1fm 날아감, 전 구간 무적"),
			*GetOwner()->GetName(), times.GetTotal(), times.launch, times.down, times.getUp, distance);

	EnterPhase(EStrongStaggerPhase::ESSP_Launch);
}

bool UPlayerStaggerComponent::TryRollGetUp()
{
	if (phase != EStrongStaggerPhase::ESSP_Down)
		return false;

	if (bLogStagger)
		UE_LOG(LogWindless, Log, TEXT("[경직] %s 다운 중 구르기"),
			*GetOwner()->GetName());

	FinishStagger(TEXT("구르며 기상"), RollGetUpBlendOut);
	return true;
}

void UPlayerStaggerComponent::CancelStagger()
{
	if (!IsStrongStaggered())
		return;

	ResetStagger();
}

void UPlayerStaggerComponent::EnterPhase(EStrongStaggerPhase newPhase)
{
	phase = newPhase;
	const float phaseTime = times.GetPhaseTime(newPhase);
	const float playRate = PlayKnockdownSection(newPhase, phaseTime);

	if (bLogStagger)
		UE_LOG(LogWindless, Log, TEXT("[경직] %s 강한 경직 %s (%.2f초 시점, %.2f초간, %s)%s"), *GetOwner()->GetName(), EnemyInterrupt::GetStrongPhaseName(newPhase),
			elapsed, phaseTime, playRate > 0.f ? *FString::Printf(TEXT("몽타주 x%.2f"), playRate) : TEXT("몽타주 없음"),
			newPhase == EStrongStaggerPhase::ESSP_Down ? TEXT(" - 구르기 입력으로 기상 가능") : TEXT(""));
}

void UPlayerStaggerComponent::FinishStagger(const TCHAR* reason, float blendOut)
{
	const float endedTime = elapsed;

	StopKnockdownMontage(blendOut);
	ResetStagger();

	if (bLogStagger)
		UE_LOG(LogWindless, Log, TEXT("[경직] %s 강한 경직 끝 (%s, %.2f초)"), *GetOwner()->GetName(), reason, endedTime);

	onStrongStaggerEnded.ExecuteIfBound();
}

void UPlayerStaggerComponent::ResetStagger()
{
	phase = EStrongStaggerPhase::ESSP_Done;
	elapsed = 0.f;
	SetComponentTickEnabled(false);
}

void UPlayerStaggerComponent::LaunchOwner(const FVector& fromLocation, float distance)
{
	ACharacter* character = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* movement = character ? character->GetCharacterMovement() : nullptr;
	if (!movement)
		return;

	FVector direction = (character->GetActorLocation() - fromLocation).GetSafeNormal2D();
	if (direction.IsNearlyZero())
		direction = -character->GetActorForwardVector().GetSafeNormal2D();

	const FVector velocity = EnemyInterrupt::ComputeLaunchVelocity(direction, distance * 100.f, times.launch, movement->GetGravityZ());
	character->LaunchCharacter(velocity, true, true);
}

float UPlayerStaggerComponent::PlayKnockdownSection(EStrongStaggerPhase targetPhase, float time)
{
	const ABaseCharacter* character = Cast<ABaseCharacter>(GetOwner());
	UAnimMontage* montage = character ? character->GetKnockdownMontage() : nullptr;
	UAnimInstance* anim = GetAnimInstance();
	const FName section = EnemyInterrupt::GetStrongSectionName(targetPhase);
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

	if (!bWarnedMontageRate && (playRate < StaggerMinPlayRate || playRate > StaggerMaxPlayRate))
	{
		bWarnedMontageRate = true;
		UE_LOG(LogWindless, Warning, TEXT("[경직] %s 강한 경직 섹션 %s 재생 속도 x%.2f (섹션 %.2f초 / 데이터 %.2f초)"),
			*GetOwner()->GetName(), *section.ToString(), playRate, clipLength, time);
	}
	return playRate;
}

void UPlayerStaggerComponent::StopKnockdownMontage(float blendOut)
{
	const ABaseCharacter* character = Cast<ABaseCharacter>(GetOwner());
	UAnimMontage* montage = character ? character->GetKnockdownMontage() : nullptr;
	UAnimInstance* anim = GetAnimInstance();
	if (montage && anim && anim->Montage_IsPlaying(montage))
		anim->Montage_Stop(blendOut, montage);
}

UAnimInstance* UPlayerStaggerComponent::GetAnimInstance() const
{
	const ACharacter* character = Cast<ACharacter>(GetOwner());
	return character && character->GetMesh() ? character->GetMesh()->GetAnimInstance() : nullptr;
}

void UPlayerStaggerComponent::DrawStaggerDebug() const
{
	if (!bDrawStagger || !GEngine || !GetOwner())
		return;

	const uint64 key = 0x30000000ull + GetOwner()->GetUniqueID();
	GEngine->AddOnScreenDebugMessage(key, 0.f, FColor::Orange, FString::Printf(TEXT("%s 강한 경직(%s) %.2f초 - 무적"),
		*GetOwner()->GetName(), EnemyInterrupt::GetStrongPhaseName(phase), GetRemaining()));
}
