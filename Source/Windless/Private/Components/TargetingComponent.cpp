#include "Components/TargetingComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Characters/WindlessCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Windless/DebugMacro.h"

UTargetingComponent::UTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.f;

	TargetObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	VisibilityTraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
}

void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!PlayerController && WindlessCharacter)
		PlayerController = Cast<APlayerController>(WindlessCharacter->GetController());

	if (!bLockActive)
		return;

	if (!IsValidTargetActor(CurrentTarget)) // 락온 대상 사망 시 즉시 해제 
	{
		BreakLock(TEXT("대상 사망 또는 제거"));
		return;
	}

	if (!IsWithinRange(CurrentTarget, BreakLockDistance)) // 락온 범위 이탈 시 즉시 해제 
	{
		BreakLock(TEXT("거리 이탈"));
		return;
	}

	if (HasLineOfSight(CurrentTarget)) // 시야 차단 시 해제
		lostSightTime = 0.f;
	else
	{
		lostSightTime += DeltaTime;
		if (lostSightTime >= lineOfSightGraceTime)
		{
			BreakLock(TEXT("시야 차단"));
			return;
		}
	}

	UpdateCameraOnTarget(DeltaTime);
}

void UTargetingComponent::ToggleLockOn()
{
	if (bLockActive)
		BreakLock(TEXT("입력 해제"));
	else
		FindAndLockTarget();
}

void UTargetingComponent::ReleaseLockOn(const TCHAR* reason)
{
	if (bLockActive)
		BreakLock(reason);
}

void UTargetingComponent::SwitchTarget(float Direction)
{
	if (!bLockActive || FMath::IsNearlyZero(Direction)) // 락온 상태가 아니거나 입력이 약하면 무시
		return;

	if (GetWorld()->GetTimeSeconds() - LastSwitchTime < SwitchCoolTime) // 쿨타임 중이면 무시
		return;

	AActor* NewTarget = FindSwitchTarget(Direction);
	if (NewTarget)
	{
		LastSwitchTime = GetWorld()->GetTimeSeconds();
		SetLockTarget(NewTarget);
		UE_LOG(LogWindless, Log, TEXT("[락온] 대상 전환 -> %s"), *NewTarget->GetName());
	}
}

void UTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
	WindlessCharacter = Cast<ACharacter>(GetOwner());
	if (WindlessCharacter)
		PlayerController = Cast<APlayerController>(WindlessCharacter->GetController());

	CreateLockOnMarker();
}

void UTargetingComponent::FindAndLockTarget()
{
	if (!WindlessCharacter || !PlayerController)
		return;

	TArray<AActor*> Candidates;
	CollectCandidates(Candidates);

	AActor* Best = FindBestTarget(Candidates);
	if (!Best)
	{
		UE_LOG(LogWindless, Log, TEXT("[락온] 대상 없음 (후보 %d)"), Candidates.Num());
		return;
	}

	SetLockTarget(Best);
	UE_LOG(LogWindless, Log, TEXT("[락온] 대상 고정 -> %s"), *Best->GetName());
}

void UTargetingComponent::SetLockTarget(AActor* newTarget)
{
	CurrentTarget = newTarget;
	bLockActive = true;
	lostSightTime = 0.f;
	ShowLockOnMarker(newTarget);
	OnTargetChanged.Broadcast(CurrentTarget);
}

void UTargetingComponent::BreakLock(const TCHAR* reason)
{
	if (!bLockActive)
		return;

	CurrentTarget = nullptr;
	bLockActive = false;
	lostSightTime = 0.f;
	HideLockOnMarker();

	UE_LOG(LogWindless, Log, TEXT("[락온] 해제 (%s)"), reason);
	OnTargetChanged.Broadcast(nullptr);
}

void UTargetingComponent::CollectCandidates(TArray<AActor*>& OutCandidates) const
{
	if (!WindlessCharacter)
		return;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(WindlessCharacter); 
	TArray<AActor*> Found;
	UKismetSystemLibrary::SphereOverlapActors(
		this,
		WindlessCharacter->GetActorLocation(),
		SearchSphereRadius,
		TargetObjectTypes,
		APawn::StaticClass(),
		ActorsToIgnore,
		Found
	);

	for (AActor* A : Found)
	{
		if (IsCandidateEligible(A))
			OutCandidates.Add(A);
	}
}

bool UTargetingComponent::IsCandidateEligible(AActor* Candidate) const
{
	return IsValidTargetActor(Candidate) &&
		IsWithinRange(Candidate, MaxLockOnDistance) &&
		HasLineOfSight(Candidate);
}

bool UTargetingComponent::IsValidTargetActor(const AActor* candidate) const
{
	if (!IsValid(candidate) || candidate == WindlessCharacter.Get()) // 무효 액터와 자기 자신이면 제외
		return false;

	if (!RequiredTag.IsNone() && !candidate->ActorHasTag(RequiredTag)) // 적 태그가 없으면 제외
		return false;

	if (!DeadTag.IsNone() && candidate->ActorHasTag(DeadTag)) // 사망 태그가 있으면 제외
		return false;

	if (candidate->IsActorBeingDestroyed()) // 파괴 중인 액터는 제외
		return false;

	return true;
}

bool UTargetingComponent::IsWithinRange(const AActor* candidate, float baseRange) const
{
	if(!WindlessCharacter || !candidate)
		return false;

	float rangeMult = 1.0f;
	if (const AWindlessCharacter* windless = Cast<AWindlessCharacter>(WindlessCharacter))
		rangeMult = windless->GetLockOnRangeMultiplier();

	const float range = baseRange * rangeMult;
	return FVector::DistSquared(WindlessCharacter->GetActorLocation(), candidate->GetActorLocation()) <= FMath::Square(range);
}

bool UTargetingComponent::HasLineOfSight(const AActor* Target) const
{
	if (!WindlessCharacter || !Target)
		return false;

	FHitResult Hit;
	const FVector Start = WindlessCharacter->GetActorLocation();
	const FVector End = GetTargetingLocation(Target);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(WindlessCharacter);
	ActorsToIgnore.Add(const_cast<AActor*>(Target));

	const bool bHit = UKismetSystemLibrary::LineTraceSingle(
		this, Start, End,
		VisibilityTraceChannel,
		false, ActorsToIgnore,
		EDrawDebugTrace::None,
		Hit, true
	);

	return !bHit;
}

bool UTargetingComponent::GetScreenPosition(const AActor* Target, FVector2D& OutScreenPos) const
{
	if (!PlayerController || !Target)
		return false;

	const FVector targetLocation = GetTargetingLocation(Target);
	return PlayerController->ProjectWorldLocationToScreen(targetLocation, OutScreenPos, false);
}

AActor* UTargetingComponent::FindBestTarget(const TArray<AActor*>& Candidates) const
{
	if (Candidates.Num() == 0 || !PlayerController) 
		return nullptr;

	int32 ViewportSizeX = 0, ViewportSizeY = 0;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0) 
		return nullptr;

	const FVector2D ScreenCenter(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
	const float HalfDiag = ScreenCenter.Size();

	AActor* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (AActor* A : Candidates)
	{
		FVector2D ScreenPos;
		if (!GetScreenPosition(A, ScreenPos)) // 카메라 뒤편 대상이면 무시
			continue;  

		const float ScreenDist = FVector2D::Distance(ScreenPos, ScreenCenter) / HalfDiag;
		if (ScreenDist > MaxScreenDistance) // / 화면 가장자리의 대상이면 무시
			continue;

		const float WorldDist = FVector::Distance(WindlessCharacter->GetActorLocation(), A->GetActorLocation());
		const float NormalizedDist = WorldDist / MaxLockOnDistance;
		const float Score = (ScreenDist * ScreenCenterWeight) + (NormalizedDist * DistanceWeight);
		if (Score < BestScore)
		{
			BestScore = Score;
			Best = A;
		}
	}
	return Best;
}

FVector UTargetingComponent::GetTargetingLocation(const AActor* Target) const
{
	if (!Target)
		return FVector::ZeroVector;
	return Target->GetActorLocation();
}

AActor* UTargetingComponent::FindSwitchTarget(float Direction) const
{
	if (!CurrentTarget || !PlayerController) 
		return nullptr;

	TArray<AActor*> Candidates;
	CollectCandidates(Candidates);

	FVector2D CurrentScreenPos;
	if (!GetScreenPosition(CurrentTarget, CurrentScreenPos))// 현재 타겟의 스크린 좌표를 기준으로 좌우를 벗어난 대상이면 무시
		return nullptr;

	AActor* Best = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (AActor* A : Candidates)
	{
		if (A == CurrentTarget) // 현재 락온 대상이면 무시
			continue;

		FVector2D ScreenPos;
		if (!GetScreenPosition(A, ScreenPos)) // 카메라 뒤편 대상이면 무시
			continue;

		const float DeltaX = ScreenPos.X - CurrentScreenPos.X;
		if (FMath::Sign(DeltaX) != FMath::Sign(Direction)) 	// 입력 방향과 다른 쪽에 위치한 대상이면 무시 
			continue;

		const float HorizDist = FMath::Abs(DeltaX);
		const float VertDist = FMath::Abs(ScreenPos.Y - CurrentScreenPos.Y);
		const float Score = HorizDist + VertDist * 0.5f;
		if (Score < BestScore)
		{
			BestScore = Score;
			Best = A;
		}
	}

	return Best;
}

void UTargetingComponent::UpdateCameraOnTarget(float DeltaTime)
{
	if (!WindlessCharacter || !PlayerController || !CurrentTarget) 
		return;

	const FVector playerLocation = WindlessCharacter->GetActorLocation();
	const FVector targetLocation = GetTargetingLocation(CurrentTarget);

	FRotator lookAt = UKismetMathLibrary::FindLookAtRotation(playerLocation, targetLocation);
	lookAt.Pitch += TargetPitchOffset;
	lookAt.Roll = 0.f;

	const FRotator currentRotation = PlayerController->GetControlRotation();
	const FRotator newRotation = UKismetMathLibrary::RInterpTo(currentRotation, lookAt, DeltaTime, CameraInterpSpeed);
	PlayerController->SetControlRotation(newRotation);
}

void UTargetingComponent::CreateLockOnMarker()
{
	AActor* owner = GetOwner();
	if (!lockOnMarkerClass || !owner)
		return;

	lockOnMarker = NewObject<UWidgetComponent>(owner, TEXT("LockOnMarker"));
	lockOnMarker->SetWidgetSpace(EWidgetSpace::Screen);
	lockOnMarker->SetWidgetClass(lockOnMarkerClass);
	lockOnMarker->SetDrawSize(lockOnMarkerSize);
	lockOnMarker->SetPivot(FVector2D(0.5f, 0.5f));
	lockOnMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	lockOnMarker->SetupAttachment(owner->GetRootComponent());
	lockOnMarker->RegisterComponent();
	lockOnMarker->SetVisibility(false);
}

void UTargetingComponent::ShowLockOnMarker(AActor* target)
{
	if (!lockOnMarker || !target || !target->GetRootComponent())
		return;

	lockOnMarker->AttachToComponent(target->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	lockOnMarker->SetRelativeLocation(lockOnMarkerOffset);
	lockOnMarker->SetVisibility(true);
}

void UTargetingComponent::HideLockOnMarker()
{
	if (!lockOnMarker)
		return;

	lockOnMarker->SetVisibility(false);
	if (AActor* owner = GetOwner())
		lockOnMarker->AttachToComponent(owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}
