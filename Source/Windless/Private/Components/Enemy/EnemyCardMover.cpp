#include "Components/Enemy/EnemyCardMover.h"
#include "Enemy/AI/EnemyCardMove.h"
#include "Enemy/AI/EnemyCardScore.h"
#include "Enemy/AI/EnemyAISettings.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Windless/DebugMacro.h"
#include "Components/Enemy/EnemyCardMover.h"

namespace
{
	constexpr float SideStepLookAhead = 0.5f; // 옆걸음(몇 초 앞 지점을 목표로 잡을지)
	constexpr float SideStepRetargetInterval = 0.25f; // 옆걸음(목표 갱신 간격)
	constexpr float MoveAcceptRadius = 20.f; // 지점 이동 도착 반경
	constexpr float RunWalkHysteresis = 0.5f; // 달리기, 걷기 전환 완충
	const FVector NavProjectExtent(50.f, 50.f, 200.f); // 내비 투영 범위
}

UEnemyCardMover::UEnemyCardMover()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyCardMover::Setup(const FEnemyCardMoveParams& inParams)
{
	params = inParams;
}

bool UEnemyCardMover::CanSideStep(const AActor* inTarget) const
{
	const AActor* owner = GetOwner();
	if (!inTarget || !owner)
		return false;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const float arc = params.walkSpeed * 100.f * (settings ? settings->sideStepTimeMin : 1.5f);
	const FVector center = inTarget->GetActorLocation();
	const FVector from = owner->GetActorLocation();
	const float radius = FVector::Dist2D(center, from);

	return IsMoveClear(EnemyCardMove::GetCirclePoint(center, from, radius, arc, 1.f))
		|| IsMoveClear(EnemyCardMove::GetCirclePoint(center, from, radius, arc, -1.f));
}

bool UEnemyCardMover::CanBackOff(const AActor* inTarget) const
{
	const AActor* owner = GetOwner();
	if (!inTarget || !owner)
		return false;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const float distance = (settings ? settings->backOffDistance : 2.5f) * 100.f;
	return IsMoveClear(EnemyCardMove::GetBackOffPoint(owner->GetActorLocation(), inTarget->GetActorLocation(), distance));
}

bool UEnemyCardMover::StartCard(EEnemyCard inCard, AActor* inTarget)
{
	StopCard();
	if (!inTarget)
		return false;

	target = inTarget;
	card = inCard;

	bool bStarted = false;
	switch (card)
	{
	case EEnemyCard::EC_SideStep: bStarted = StartSideStep(); break;
	case EEnemyCard::EC_BackOff:  bStarted = StartBackOff();  break;
	case EEnemyCard::EC_Approach: bStarted = StartApproach(); break;
	default:
		card = EEnemyCard::EC_Rest;
		bStarted = StartRest();
		break;
	}

	bRunning = bStarted;
	return bStarted;
}

bool UEnemyCardMover::UpdateCard(float DeltaTime)
{
	if (!bRunning)
		return false;

	if (!target.IsValid())
	{
		Finish(TEXT("대상 없음"));
		return false;
	}

	if (bGlare)
		return UpdateGlare(DeltaTime);

	if (card == EEnemyCard::EC_Approach)
		return UpdateApproach();

	FaceTarget(DeltaTime);
	if (GetNow() >= endTime)
	{
		Finish(TEXT("시간"));
		return false;
	}

	if (card == EEnemyCard::EC_SideStep && GetNow() >= nextRetargetTime && !RetargetSideStep())
	{
		Finish(TEXT("갈 곳 없음"));
		return false;
	}

	if (card == EEnemyCard::EC_BackOff && !IsMoving())
	{
		Finish(TEXT("도착"));
		return false;
	}

	return true;
}

void UEnemyCardMover::StopCard()
{
	if (!bRunning)
		return;

	bRunning = false;
	if (card != EEnemyCard::EC_Rest || bGlare)
	{
		if (AAIController* controller = GetAIController())
			controller->StopMovement();
	}
	bGlare = false;
	bGlareMoving = false;
	SetStrafe(false);
}

void UEnemyCardMover::StartGlare(AActor* inTarget, const FVector& standPoint, bool bMoveFirst, float glareTime)
{
	StopCard();
	target = inTarget;
	card = EEnemyCard::EC_Rest;
	bGlare = true;
	bRunning = true;
	glareDuration = glareTime;

	const float distance = FVector::Dist2D(GetOwner()->GetActorLocation(), standPoint) / 100.f;
	const float speed = EnemyCardMove::GetApproachSpeed(distance, params.runDistance, params.walkSpeed, params.runSpeed) * 100.f;
	bGlareMoving = bMoveFirst && MoveToPoint(standPoint, speed);
	if (!bGlareMoving)
	{
		if (AAIController* controller = GetAIController())
			controller->StopMovement();

		endTime = GetNow() + glareDuration;
	}
}

bool UEnemyCardMover::IsMoveClear(const FVector& to) const
{
	UWorld* world = GetWorld();
	const AActor* owner = GetOwner();
	const UNavigationSystemV1* navSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(world);
	if (!owner || !navSystem)
		return false;

	FNavLocation navFrom;
	FNavLocation navTo;
	if (!navSystem->ProjectPointToNavigation(owner->GetActorLocation(), navFrom, NavProjectExtent))
		return false;

	if (!navSystem->ProjectPointToNavigation(to, navTo, NavProjectExtent))
		return false;

	FVector hitLocation;
	return !UNavigationSystemV1::NavigationRaycast(world, navFrom.Location, navTo.Location, hitLocation, nullptr, GetAIController());
}

bool UEnemyCardMover::StartRest()
{
	if (AAIController* controller = GetAIController())
		controller->StopMovement();

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const float restTime = settings ? FMath::FRandRange(settings->restTimeMin, settings->restTimeMax) : FMath::FRandRange(0.5f, 1.2f);
	endTime = GetNow() + restTime;
	UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s 숨 고르기 %.1f초"), *GetNameSafe(GetOwner()), restTime);
	return true;
}

bool UEnemyCardMover::StartSideStep()
{
	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const float duration = settings ? FMath::FRandRange(settings->sideStepTimeMin, settings->sideStepTimeMax) : FMath::FRandRange(1.5f, 2.5f);
	sideRadius = GetTargetDistance() * 100.f;
	sideDirection = FMath::RandBool() ? 1.f : -1.f;
	SetStrafe(true);

	if (!RetargetSideStep())
	{
		sideDirection = -sideDirection;
		if (!RetargetSideStep())
		{
			SetStrafe(false);
			UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s 옆걸음 불가(양옆에 갈 곳 없음)"), *GetNameSafe(GetOwner()));
			return false;
		}
	}

	endTime = GetNow() + duration;
	UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s 옆걸음 %s %.1f초 (반지름 %.1fm)"), *GetNameSafe(GetOwner()),
		sideDirection > 0.f ? TEXT("시계 방향") : TEXT("반시계 방향"), duration, sideRadius / 100.f);
	return true;
}

bool UEnemyCardMover::RetargetSideStep()
{
	const AActor* targetActor = target.Get();
	if (!targetActor)
		return false;

	const float speed = params.walkSpeed * 100.f;
	const FVector point = EnemyCardMove::GetCirclePoint(targetActor->GetActorLocation(), GetOwner()->GetActorLocation(), sideRadius, speed * SideStepLookAhead, sideDirection);
	nextRetargetTime = GetNow() + SideStepRetargetInterval;
	return IsMoveClear(point) && MoveToPoint(point, speed);
}

void UEnemyCardMover::SetStrafe(bool bStrafe)
{
	if (ACharacter* character = GetCharacter())
		character->GetCharacterMovement()->bOrientRotationToMovement = !bStrafe;
}

bool UEnemyCardMover::StartBackOff()
{
	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const float distance = settings ? settings->backOffDistance : 2.5f;
	const FVector point = EnemyCardMove::GetBackOffPoint(GetOwner()->GetActorLocation(), target->GetActorLocation(), distance * 100.f);

	SetStrafe(true);
	if (!IsMoveClear(point) || !MoveToPoint(point, params.walkSpeed * 100.f))
	{
		SetStrafe(false);
		UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s 뒤로 빠지기 불가(뒤에 갈 곳 없음)"), *GetNameSafe(GetOwner()));
		return false;
	}

	endTime = GetNow() + distance / FMath::Max(params.walkSpeed, 0.1f) + 1.f;
	UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s 뒤로 빠지기 %.1fm"), *GetNameSafe(GetOwner()), distance);
	return true;
}

bool UEnemyCardMover::StartApproach()
{
	AAIController* controller = GetAIController();
	ACharacter* character = GetCharacter();
	if (!controller || !character)
		return false;

	const float distance = GetTargetDistance();
	bApproachRunning = distance > params.runDistance;
	character->GetCharacterMovement()->MaxWalkSpeed = EnemyCardMove::GetApproachSpeed(distance, params.runDistance, params.walkSpeed, params.runSpeed) * 100.f;
	SetStrafe(false);

	FAIMoveRequest request;
	request.SetGoalActor(target.Get());
	request.SetAcceptanceRadius(params.preferredRange * 100.f);
	request.SetReachTestIncludesAgentRadius(false);
	request.SetReachTestIncludesGoalRadius(false);
	if (controller->MoveTo(request).Code == EPathFollowingRequestResult::Failed)
	{
		UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s 다가가기 불가(경로 없음)"), *GetNameSafe(GetOwner()));
		return false;
	}

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	endTime = GetNow() + (settings ? settings->approachMaxTime : 3.f);
	UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s 다가가기 %s (%.1fm, 선호 거리 %.1fm)"), *GetNameSafe(GetOwner()),
		bApproachRunning ? TEXT("달리기") : TEXT("걷기"), distance, params.preferredRange);
	return true;
}

bool UEnemyCardMover::UpdateApproach()
{
	const float distance = GetTargetDistance();
	if (distance <= params.preferredRange)
	{
		Finish(TEXT("선호 거리"));
		return false;
	}

	if (GetNow() >= endTime)
	{
		Finish(TEXT("시간"));
		return false;
	}

	if (!IsMoving())
	{
		Finish(TEXT("이동 끝"));
		return false;
	}

	/* 달리기 기준 거리를 넘나들면 속도 전환, 경계에서 떨지 않게 달리는 중에는 0.5m 완충 */
	const bool bRun = bApproachRunning ? distance > params.runDistance - RunWalkHysteresis : distance > params.runDistance;
	if (bRun != bApproachRunning)
	{
		bApproachRunning = bRun;
		if (ACharacter* character = GetCharacter())
			character->GetCharacterMovement()->MaxWalkSpeed = (bRun ? params.runSpeed : params.walkSpeed) * 100.f;

		UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s 다가가기 %s로 전환 (%.1fm)"), *GetNameSafe(GetOwner()), bRun ? TEXT("달리기") : TEXT("걷기"), distance);
	}
	return true;
}

bool UEnemyCardMover::UpdateGlare(float DeltaTime)
{
	if (bGlareMoving)
	{
		if (IsMoving())
			return true;

		bGlareMoving = false;
		endTime = GetNow() + glareDuration;
		UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s 가장 가까운 지점 도착(노려보기 %.0f초)"), *GetNameSafe(GetOwner()), glareDuration);
	}

	FaceTarget(DeltaTime);
	if (GetNow() >= endTime)
	{
		Finish(TEXT("시간"));
		return false;
	}
	return true;
}

void UEnemyCardMover::FaceTarget(float DeltaTime)
{
	AActor* owner = GetOwner();
	const AActor* targetActor = target.Get();
	if (!owner || !targetActor)
		return;

	const FVector toTarget = (targetActor->GetActorLocation() - owner->GetActorLocation()).GetSafeNormal2D();
	if (toTarget.IsNearlyZero())
		return;

	FRotator rotation = owner->GetActorRotation();
	rotation.Yaw = FMath::FixedTurn(rotation.Yaw, toTarget.Rotation().Yaw, params.turnRate * DeltaTime);
	owner->SetActorRotation(rotation);
}

bool UEnemyCardMover::MoveToPoint(const FVector& point, float speed)
{
	AAIController* controller = GetAIController();
	ACharacter* character = GetCharacter();
	const UNavigationSystemV1* navSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!controller || !character || !navSystem)
		return false;

	FNavLocation navPoint;
	if (!navSystem->ProjectPointToNavigation(point, navPoint, NavProjectExtent))
		return false;

	character->GetCharacterMovement()->MaxWalkSpeed = speed;

	FAIMoveRequest request;
	request.SetGoalLocation(navPoint.Location);
	request.SetAcceptanceRadius(MoveAcceptRadius);
	return controller->MoveTo(request).Code != EPathFollowingRequestResult::Failed;
}

bool UEnemyCardMover::IsMoving() const
{
	const AAIController* controller = GetAIController();
	return controller && controller->GetMoveStatus() == EPathFollowingStatus::Moving;
}

void UEnemyCardMover::Finish(const TCHAR* reason)
{
	UE_LOG(LogWindless, Log, TEXT("[카드 동작] %s %s 끝 (%s)"), *GetNameSafe(GetOwner()), bGlare ? TEXT("노려보기") : EnemyCardScore::GetCardName(card), reason);
	StopCard();
}

float UEnemyCardMover::GetTargetDistance() const
{
	const AActor* targetActor = target.Get();
	return (targetActor && GetOwner()) ? FVector::Dist2D(GetOwner()->GetActorLocation(), targetActor->GetActorLocation()) / 100.f : 0.f;
}

AAIController* UEnemyCardMover::GetAIController() const
{
	const APawn* pawn = Cast<APawn>(GetOwner());
	return pawn ? Cast<AAIController>(pawn->GetController()) : nullptr;
}

ACharacter* UEnemyCardMover::GetCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

float UEnemyCardMover::GetNow() const
{
	return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}
