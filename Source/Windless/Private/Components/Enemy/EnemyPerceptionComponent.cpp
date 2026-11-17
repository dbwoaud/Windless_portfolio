#include "Components/Enemy/EnemyPerceptionComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Windless/DebugMacro.h"

UEnemyPerceptionComponent::UEnemyPerceptionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyPerceptionComponent::SetPerceptionEnabled(bool bEnable)
{
	UWorld* world = GetWorld();
	if (!world)
		return;

	if (bEnable)
		world->GetTimerManager().SetTimer(checkTimer, this, &UEnemyPerceptionComponent::EvaluatePerception, checkInterval, true);
	else
	{
		world->GetTimerManager().ClearTimer(checkTimer);
		trackedTarget = nullptr;
		bHasLineOfSight = false;
	}
}

void UEnemyPerceptionComponent::ForceTrack(APawn* target)
{
	if (!target)
		return;

	trackedTarget = target;
	lastKnownLocation = target->GetActorLocation();
	timeSinceLastSeen = 0.f;
}

float UEnemyPerceptionComponent::GetEffectiveSightRadius(const APawn* target) const
{
	float radius = sightRadius;

	if (const AWindlessCharacter* player = Cast<AWindlessCharacter>(target))
	{
		if (!player->IsInLightZone()) // 어두울수록 감지 범위 증가
			radius *= darknessMultiplier;

		if (player->bIsCrouched) // 은신 시 감지 범위 감소
			radius *= stealthMultiplier;
	}

	return radius;
}

void UEnemyPerceptionComponent::BeginPlay()
{
	Super::BeginPlay();
	SetPerceptionEnabled(true);
}

void UEnemyPerceptionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetPerceptionEnabled(false);
	Super::EndPlay(EndPlayReason);
}

void UEnemyPerceptionComponent::EvaluatePerception()
{
	APawn* candidate = FindCandidate();
	const bool bSees = candidate && CanSee(candidate);

	if (bDrawDebug)
		DrawSightDebug(candidate, bSees);

	if (bSees)
	{
		if (trackedTarget.Get() != candidate)
		{
			UE_LOG(LogWindless, Log, TEXT("[AI] %s 감지 -> %s (거리 %.0f / 감지거리 %.0f)"), *GetNameSafe(GetOwner()), *candidate->GetName(),
				FVector::Dist(GetOwner()->GetActorLocation(), candidate->GetActorLocation()), GetEffectiveSightRadius(candidate));
		}

		trackedTarget = candidate;
		lastKnownLocation = candidate->GetActorLocation();
		timeSinceLastSeen = 0.f;
		bHasLineOfSight = true;
		return;
	}

	bHasLineOfSight = false;

	APawn* target = trackedTarget.Get();
	if (!target)
		return;

	if (target->ActorHasTag(FName("Dead"))) // 대상 사망 시 인지 해제
	{
		trackedTarget = nullptr;
		return;
	}

	/* 시야 상실 후 일정 시간 동안은 위치를 계속 파악 */
	timeSinceLastSeen += checkInterval;
	if (timeSinceLastSeen < memoryDuration)
	{
		lastKnownLocation = target->GetActorLocation();
		return;
	}

	UE_LOG(LogWindless, Log, TEXT("[AI] %s 시야 상실 %.1f초 경과 - 마지막 위치 탐색"), *GetNameSafe(GetOwner()), memoryDuration);
	trackedTarget = nullptr;
	onTargetLost.Broadcast(lastKnownLocation);
}

APawn* UEnemyPerceptionComponent::FindCandidate() const
{
	APawn* player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!player || !player->ActorHasTag(FName("EngagableTarget")) || player->ActorHasTag(FName("Dead")))
		return nullptr;

	return player;

}

bool UEnemyPerceptionComponent::CanSee(const APawn* target) const
{
	const AActor* owner = GetOwner();
	UWorld* world = GetWorld();
	if (!owner || !target || !world)
		return false;

	FVector eyeLocation;
	FRotator eyeRotation;
	owner->GetActorEyesViewPoint(eyeLocation, eyeRotation);

	// 1) 조도/은신 보정 거리 거리
	const FVector toTarget = target->GetActorLocation() - eyeLocation;
	if (toTarget.SizeSquared() > FMath::Square(GetEffectiveSightRadius(target)))
		return false;

	// 2) 시야각
	const FVector forward = owner->GetActorForwardVector().GetSafeNormal2D();
	if (FVector::DotProduct(forward, toTarget.GetSafeNormal2D()) < FMath::Cos(FMath::DegreesToRadians(sightHalfAngle)))
		return false;

	// 3) 장애물
	FCollisionQueryParams params(SCENE_QUERY_STAT(EnemySight), false, owner);
	params.AddIgnoredActor(target);

	FHitResult hit;
	return !world->LineTraceSingleByChannel(hit, eyeLocation, target->GetActorLocation(), sightTraceChannel, params);
}

void UEnemyPerceptionComponent::DrawSightDebug(const APawn* target, bool bSees) const
{
	const AActor* owner = GetOwner();
	UWorld* world = GetWorld();
	if (!owner || !world)
		return;

	FVector eyeLocation;
	FRotator eyeRotation;
	owner->GetActorEyesViewPoint(eyeLocation, eyeRotation);

	const float radius = target ? GetEffectiveSightRadius(target) : sightRadius;
	const float angle = FMath::DegreesToRadians(sightHalfAngle);
	DrawDebugCone(world, eyeLocation, owner->GetActorForwardVector(), radius, angle, angle, 12,
		bSees ? FColor::Red : FColor::Green, false, checkInterval + 0.05f);
}
