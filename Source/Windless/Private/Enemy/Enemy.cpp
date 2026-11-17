#include "Enemy/Enemy.h"
#include "Enemy/AI/EnemyAITypes.h"
#include "Enemy/AI/MonsterArchetypeRow.h"
#include "Enemy/AI/EnemyAttackRow.h"
#include "Enemy/AI/EnemyAISettings.h"
#include "Enemy/AI/EnemyAttackHit.h"
#include "Enemy/AI/EnemyReach.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Navigation/PathFollowingComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/WeakPointComponent.h"
#include "Components/Enemy/EnemyPerceptionComponent.h"
#include "Components/Enemy/EnemyAttackRunner.h"
#include "Components/Enemy/EnemyActionSelector.h"
#include "Components/Enemy/EnemyInterruptComponent.h"
#include "Components/Enemy/EnemyCardMover.h"
#include "Components/PlayerAISignal.h"
#include "Components/WeakPointRoll.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/CombatTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Items/Soul.h"
#include "Items/Weapons/Weapon.h"
#include "Level/SavePointSubsystem.h"
#include "Windless/DebugMacro.h"

namespace
{
	const TCHAR* EnemyStateToString(EEnemyState state)
	{
		switch (state)
		{
		case EEnemyState::EES_NoState:    return TEXT("NoState");
		case EEnemyState::EES_Dead:       return TEXT("Dead");
		case EEnemyState::EES_Patrolling: return TEXT("Patrolling");
		case EEnemyState::EES_Chasing:    return TEXT("Chasing");
		case EEnemyState::EES_Attacking:  return TEXT("Attacking");
		case EEnemyState::EES_Engaged:    return TEXT("Engaged");
		case EEnemyState::EES_Idle:       return TEXT("Idle");
		case EEnemyState::EES_Searching:  return TEXT("Searching");
		case EEnemyState::EES_Returning:  return TEXT("Returning");
		case EEnemyState::EES_Staggered:  return TEXT("Staggered");
		default:                          return TEXT("Unknown");
		}
	}
}

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	GetMesh()->SetGenerateOverlapEvents(true);

	healthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	healthBarWidget->SetupAttachment(GetRootComponent());

	perception = CreateDefaultSubobject<UEnemyPerceptionComponent>(TEXT("Perception"));
	weakPoints = CreateDefaultSubobject<UWeakPointComponent>(TEXT("WeakPoints"));
	attackRunner = CreateDefaultSubobject<UEnemyAttackRunner>(TEXT("AttackRunner"));
	actionSelector = CreateDefaultSubobject<UEnemyActionSelector>(TEXT("ActionSelector"));
	interrupt = CreateDefaultSubobject<UEnemyInterruptComponent>(TEXT("Interrupt"));
	cardMover = CreateDefaultSubobject<UEnemyCardMover>(TEXT("CardMover"));
}

void AEnemy::DebugForceNextCard(EEnemyCard card)
{
	if (actionSelector)
		actionSelector->ForceNextCard(card);
}

void AEnemy::BeginPlay()
{
	ResolveArchetypeData();
	if (attributes && archetypeData)
		attributes->InitializeHealth(archetypeData->maxHP);
	ApplyArchetypeWeakPoints();
	Super::BeginPlay();
	Tags.Add(FName("Enemy"));
	InitializeEnemy();
}

void AEnemy::InitializeEnemy()
{
	enemyController = Cast<AAIController>(GetController());
	homeLocation = GetActorLocation();
	homeRotation = GetActorRotation();
	idleWaitRemaining = FMath::RandRange(patrolWaitMin, patrolWaitMax);

	HideHealthBar();
	SpawnDefaultWeapon();

	if (perception)
		perception->onTargetLost.AddDynamic(this, &AEnemy::HandleTargetLost);

	if (attackRunner)
	{
		attackRunner->onHitboxChanged.BindUObject(this, &AEnemy::SetAttackHitbox);
		attackRunner->onAttackFinished.BindUObject(this, &AEnemy::OnDataAttackFinished);
	}

	if (actionSelector && archetypeData && UsesDataAttacks())
		actionSelector->Setup(*archetypeData, profile, attackData, attackDataIds);

	if (cardMover && archetypeData)
	{
		FEnemyCardMoveParams moveParams;
		moveParams.walkSpeed = archetypeData->walkSpeed;
		moveParams.runSpeed = archetypeData->runSpeed;
		moveParams.runDistance = archetypeData->runDistance;
		moveParams.preferredRange = archetypeData->preferredRange;
		moveParams.turnRate = GetCombatTurnRate();
		cardMover->Setup(moveParams);
	}

	if (interrupt)
		interrupt->onInterruptEnded.BindUObject(this, &AEnemy::EndStagger);
}

void AEnemy::SetPatrolRoute(const TArray<AActor*>& route)
{
	if (route.Num() == 0)
		return;

	patrolTargets = route;
	patrolIndex = INDEX_NONE;
}

void AEnemy::SetPlacement(const FDataTableRowHandle& archetype, EEnemyIdleType idleType)
{
	if (!archetype.IsNull())
		archetypeRow = archetype;

	if (idleType != EEnemyIdleType::EIT_None)
		idleTypeOverride = idleType;
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead())
		return;

	switch (enemyState)
	{
		case EEnemyState::EES_Idle:       UpdateIdle(DeltaTime);      break;
		case EEnemyState::EES_Patrolling: UpdatePatrolling();         break;
		case EEnemyState::EES_Chasing:    UpdateChasing(DeltaTime);   break;
		case EEnemyState::EES_Attacking:  UpdateAttacking(DeltaTime); break;
		case EEnemyState::EES_Searching:  UpdateSearching(DeltaTime); break;
		case EEnemyState::EES_Returning:  UpdateReturning();          break;
		default: break;
	}
}

void AEnemy::SetEnemyState(EEnemyState newState)
{
	if (enemyState == newState)
		return;

	if (enemyState == EEnemyState::EES_Attacking && newState != EEnemyState::EES_Attacking)
		EndCurrentCard();

	if (bLogStateChanges)
	{
		const UEnum* stateEnum = StaticEnum<EEnemyState>();
		UE_LOG(LogWindless, Log, TEXT("[AI] %s 상태 %s -> %s"), *GetName(),
			*stateEnum->GetDisplayNameTextByValue((int64)enemyState).ToString(),
			*stateEnum->GetDisplayNameTextByValue((int64)newState).ToString());
	}

	enemyState = newState;
}

bool AEnemy::IsDead() const
{
	return enemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsInCombat() const
{
	switch (enemyState)
	{
	case EEnemyState::EES_Chasing:
	case EEnemyState::EES_Attacking:
	case EEnemyState::EES_Engaged:
	case EEnemyState::EES_Staggered:
		return true;
	case EEnemyState::EES_Searching:
		return bHesitating;
	default:
		return false;
	}
}

void AEnemy::UpdateIdle(float DeltaTime)
{
	if (TryEngageSightedTarget(true))
		return;

	if (moveType == EEnemyMoveType::EMT_Fixed)
		return;

	idleWaitRemaining -= DeltaTime;
	if (idleWaitRemaining <= 0.f)
		StartNextPatrol();
}

void AEnemy::UpdatePatrolling()
{
	if (TryEngageSightedTarget(true))
		return;

	if (HasArrived())
	{
		idleWaitRemaining = FMath::RandRange(patrolWaitMin, patrolWaitMax);
		SetEnemyState(EEnemyState::EES_Idle);
	}
}

void AEnemy::UpdateChasing(float DeltaTime)
{
	if (!IsTargetValid())
	{
		HandleNoTarget();
		return;
	}

	if (IsTargetInSafeZone(combatTarget))
	{
		StartReturn(TEXT("대상 안전지대 진입"));
		return;
	}

	if (bForcedChase && perception && perception->HasLineOfSight())
	{
		bForcedChase = false;
		UE_LOG(LogWindless, Log, TEXT("[AI] %s 강제 추적 해제 - 대상 시야 확보"), *GetName());
	}

	if (ShouldGiveUpChase())
	{
		StartReturn(TEXT("추적 포기"));
		return;
	}


	if (UsesCardSelection())
	{
		SetEnemyState(EEnemyState::EES_Attacking);
		return;
	}

	if (IsWithin(combatTarget, GetAttackRadius())) // 사거리 진입 시 대치 없이 공격
	{
		StopMovement();
		nextAttackTime = FMath::Max(nextAttackTime, GetNow() + attackStartDelay);
		SetEnemyState(EEnemyState::EES_Attacking);
		return;
	}

	if (moveType == EEnemyMoveType::EMT_Fixed) // 고정형 적은 관망
	{
		FaceTarget(DeltaTime);
		return;
	}

	if (HasArrived()) // 이동 요청이 끝났거나 실패하면 다시 요청
		MoveToActor(combatTarget, chasingSpeed);
}

void AEnemy::UpdateAttacking(float DeltaTime)
{
	if (!IsTargetValid())
	{
		HandleNoTarget();
		return;
	}

	if (IsTargetInSafeZone(combatTarget))
	{
		StartReturn(TEXT("대상 안전지대 진입"));
		return;
	}

	if (UsesCardSelection())
	{
		if (ShouldGiveUpChase())
		{
			StartReturn(TEXT("추적 포기"));
			return;
		}

		UpdateCardCombat(DeltaTime);
		return;
	}

	FaceTarget(DeltaTime);

	if (!IsWithin(combatTarget, GetAttackRadius() * attackRangeExitMultiplier))
	{
		StartChase();
		return;
	}

	if (CanAttack())
		BeginAttackSequence();
}

void AEnemy::UpdateSearching(float DeltaTime)
{
	if (bHesitating) // 플레이어 조우(대상 접근)
	{
		if (!IsTargetValid())
		{
			HandleNoTarget();
			return;
		}

		FaceTarget(DeltaTime);
		searchWaitRemaining -= DeltaTime;
		if (searchWaitRemaining <= 0.f)
			StartChase();
		return;
	}

	// 마지막 위치 탐색(이동 후 머물다 복귀)
	if (TryEngageSightedTarget(false))
		return;

	if (!HasArrived())
		return;

	searchWaitRemaining -= DeltaTime;
	if (searchWaitRemaining <= 0.f)
		StartReturn(TEXT("탐색 종료"));
}

void AEnemy::UpdateReturning()
{
	if (CanReengageWhileReturning() && TryEngageSightedTarget(false))
	{
		UE_LOG(LogWindless, Log, TEXT("[AI] %s 복귀 중 재발견 - 추적 재개"), *GetName());
		return;
	}

	if (HasArrived())
		ResetToIdle();
}

bool AEnemy::TryEngageSightedTarget(bool bRollEncounter)
{
	APawn* target = perception ? perception->GetTrackedTarget() : nullptr;
	if (!target || !perception->HasLineOfSight())
		return false;

	if (!bAggressive && !bProvoked)
		return false;

	if (IsTargetInSafeZone(target))
		return false;

	EngageTarget(target, bRollEncounter);
	return true;
}

void AEnemy::EngageTarget(AActor* target, bool bRollEncounter)
{
	combatTarget = target;
	bHasPendingSearch = false;

	if (bRollEncounter)
	{
		if (FMath::FRand() > encounterImmediateChance)
		{
			const float waitTime = FMath::RandRange(encounterHesitateMin, encounterHesitateMax);
			UE_LOG(LogWindless, Log, TEXT("[AI] %s 조우 반응: 관망 후 접근 (%.1f초)"), *GetName(), waitTime);
			StartSearch(target->GetActorLocation(), waitTime, true);
			return;
		}

		UE_LOG(LogWindless, Log, TEXT("[AI] %s 조우 반응: 즉시 공격"), *GetName());
	}

	StartChase();
}

void AEnemy::StartChase()
{
	bHesitating = false;
	SetEnemyState(EEnemyState::EES_Chasing);

	if (moveType == EEnemyMoveType::EMT_Fixed)
		StopMovement();
	else
		MoveToActor(combatTarget, chasingSpeed);
}

void AEnemy::StartSearch(const FVector& location, float waitTime, bool bHesitate)
{
	bHesitating = bHesitate;
	searchWaitRemaining = waitTime;
	SetEnemyState(EEnemyState::EES_Searching);

	if (bHesitate)
		StopMovement();
	else
	{
		UE_LOG(LogWindless, Log, TEXT("[AI] %s 마지막 위치로 이동해 탐색 (%.1f초)"), *GetName(), waitTime);
		MoveToLocation(location, chasingSpeed);
	}
}

void AEnemy::StartReturn(const TCHAR* reason)
{
	combatTarget = nullptr;
	bForcedChase = false;
	bHasPendingSearch = false;
	bHesitating = false;
	comboHitsRemaining = 0;

	if (actionSelector)
		actionSelector->ResetCombatMemory();

	HideHealthBar();

	UE_LOG(LogWindless, Log, TEXT("[AI] %s 복귀 시작 (%s)"), *GetName(), reason);
	SetEnemyState(EEnemyState::EES_Returning);
	MoveToLocation(homeLocation, chasingSpeed);
}

void AEnemy::ResetToIdle()
{
	bProvoked = false;
	bForcedChase = false;
	idleWaitRemaining = FMath::RandRange(patrolWaitMin, patrolWaitMax);

	if (moveType == EEnemyMoveType::EMT_Fixed)
		SetActorRotation(homeRotation);

	SetEnemyState(EEnemyState::EES_Idle);
}

void AEnemy::HandleNoTarget()
{
	combatTarget = nullptr;

	if (bHasPendingSearch && moveType != EEnemyMoveType::EMT_Fixed)
	{
		bHasPendingSearch = false;
		StartSearch(pendingSearchLocation, searchDuration, false);
		return;
	}

	bHasPendingSearch = false;
	StartReturn(TEXT("대상 없음"));
}

void AEnemy::HandleTargetLost(FVector lastKnownLocation)
{
	if (IsDead() || !IsInCombat())
		return;

	combatTarget = nullptr;
	pendingSearchLocation = lastKnownLocation;
	bHasPendingSearch = true;

	if (enemyState == EEnemyState::EES_Chasing || enemyState == EEnemyState::EES_Attacking || enemyState == EEnemyState::EES_Searching)
		HandleNoTarget();
}

bool AEnemy::IsTargetValid() const
{
	return combatTarget && !combatTarget->ActorHasTag(FName("Dead"));
}

bool AEnemy::IsTargetInSafeZone(const AActor* target) const
{
	if (!target)
		return false;

	const UWorld* world = GetWorld();
	const USavePointSubsystem* savePoints = world ? world->GetSubsystem<USavePointSubsystem>() : nullptr;
	return savePoints && savePoints->IsInSafeZone(target->GetActorLocation());
}

bool AEnemy::ShouldGiveUpChase() const
{
	if (!combatTarget)
		return true;

	if (bForcedChase)
		return false;

	const bool bTooFarFromTarget = FVector::Dist(GetActorLocation(), combatTarget->GetActorLocation()) > chaseGiveUpDistance;
	const bool bTooFarFromHome = FVector::Dist(GetActorLocation(), homeLocation) > leashDistance;
	return bTooFarFromTarget || bTooFarFromHome;
}

bool AEnemy::CanReengageWhileReturning() const
{
	const APawn* target = perception ? perception->GetTrackedTarget() : nullptr;
	if (!target)
		return false;

	return FVector::Dist(GetActorLocation(), homeLocation) <= leashDistance
		&& FVector::Dist(target->GetActorLocation(), homeLocation) <= leashDistance;
}

bool AEnemy::IsWithin(const AActor* target, float radius) const
{
	return target && FVector::Dist(GetActorLocation(), target->GetActorLocation()) <= radius;
}

bool AEnemy::HasArrived() const
{
	return !enemyController || enemyController->GetMoveStatus() != EPathFollowingStatus::Moving;
}

float AEnemy::GetNow() const
{
	return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
}

bool AEnemy::IsTargetStaggered()
{
	const ABaseCharacter* target = Cast<ABaseCharacter>(combatTarget);
	const bool bStaggered = target && target->GetStaggerStatus().IsStaggered();

	if (bStaggered != bHoldingForTargetStagger)
	{
		bHoldingForTargetStagger = bStaggered;
		UE_LOG(LogWindless, Log, TEXT("[AI] %s %s"), *GetName(), bStaggered ? TEXT("대상 경직 중(새 공격 보류)") : TEXT("대상 경직 끝(공격 가능)"));
	}
	return bStaggered;
}

void AEnemy::MoveToActor(AActor* target, float speed)
{
	if (!enemyController || !target)
		return;

	GetCharacterMovement()->MaxWalkSpeed = speed;

	FAIMoveRequest moveRequest;
	moveRequest.SetGoalActor(target);
	moveRequest.SetAcceptanceRadius(acceptanceRadius);
	enemyController->MoveTo(moveRequest);
}

void AEnemy::MoveToLocation(const FVector& location, float speed)
{
	if (!enemyController)
		return;

	GetCharacterMovement()->MaxWalkSpeed = speed;

	FAIMoveRequest moveRequest;
	moveRequest.SetGoalLocation(location);
	moveRequest.SetAcceptanceRadius(acceptanceRadius);
	enemyController->MoveTo(moveRequest);
}

void AEnemy::StopMovement()
{
	if (enemyController)
		enemyController->StopMovement();
}

void AEnemy::FaceTarget(float DeltaTime)
{
	if (!combatTarget || (interrupt && interrupt->IsInterrupted()))
		return;

	const FVector toTarget = (combatTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (toTarget.IsNearlyZero())
		return;

	if (UsesDataAttacks())
	{
		FRotator rotation = GetActorRotation();
		rotation.Yaw = FMath::FixedTurn(rotation.Yaw, toTarget.Rotation().Yaw, GetCombatTurnRate() * DeltaTime);
		SetActorRotation(rotation);
		return;
	}

	const FRotator targetRotation(0.f, toTarget.Rotation().Yaw, 0.f);
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), targetRotation, DeltaTime, faceTargetSpeed));
}

void AEnemy::StartNextPatrol()
{
	if (moveType == EEnemyMoveType::EMT_Patrol)
	{
		AActor* nextTarget = ChooseNextPatrolTarget();
		if (!nextTarget)
		{
			if (!bWarnedNoPatrolRoute)
			{
				bWarnedNoPatrolRoute = true;
				UE_LOG(LogWindless, Warning, TEXT("[AI] %s 순찰형인데 순찰 경로 없음 - 제자리 대기"), *GetName());
			}
			idleWaitRemaining = patrolWaitMax;
			return;
		}

		SetEnemyState(EEnemyState::EES_Patrolling);
		MoveToActor(nextTarget, patrollingSpeed);
		return;
	}

	FVector wanderLocation;
	if (!ChooseWanderLocation(wanderLocation))
	{
		idleWaitRemaining = patrolWaitMax;
		return;
	}

	SetEnemyState(EEnemyState::EES_Patrolling);
	MoveToLocation(wanderLocation, patrollingSpeed);
}

bool AEnemy::ChooseWanderLocation(FVector& outLocation) const
{
	const UNavigationSystemV1* navSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!navSystem)
		return false;

	FNavLocation result;
	if (!navSystem->GetRandomReachablePointInRadius(homeLocation, wanderRadius, result))
		return false;

	outLocation = result.Location;
	return true;
}

AActor* AEnemy::ChooseNextPatrolTarget()
{
	if (patrolTargets.Num() == 0)
		return nullptr;

	patrolIndex = (patrolIndex + 1) % patrolTargets.Num(); // 정해진 경로를 순서대로 순찰
	return patrolTargets[patrolIndex];
}

bool AEnemy::CanAttack()
{
	if (IsTargetStaggered())
		return false;

	return enemyState == EEnemyState::EES_Attacking && IsTargetValid()
		&& IsWithin(combatTarget, GetAttackRadius() * attackRangeExitMultiplier) && GetNow() >= nextAttackTime;
}

void AEnemy::CancelCurrentAttack()
{
	if (attackRunner && attackRunner->IsRunning())
		attackRunner->CancelAttack(TEXT("피격 경직"));

	Super::CancelCurrentAttack();
}

void AEnemy::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (UsesDataAttacks())
		return;

	Super::SetWeaponCollisionEnabled(CollisionEnabled);
}

void AEnemy::BeginAttackSequence()
{
	if (interrupt && interrupt->ConsumeStaggerRecovered())
		UE_LOG(LogWindless, Log, TEXT("[AI] %s 경직 회복 직후 보정 1회 사용(뒤로 빠지기 x2, 공격 묶음 x0.5)"), *GetName());

	if (UsesDataAttacks())
		comboHitsRemaining = 1;
	else
		comboHitsRemaining = FMath::RandRange(FMath::Min(minComboHits, maxComboHits), maxComboHits);

	UE_LOG(LogWindless, Log, TEXT("[AI] %s 공격 개시 (%s)"), *GetName(),
		UsesDataAttacks() ? TEXT("데이터 공격") : *FString::Printf(TEXT("콤보 %d타"), comboHitsRemaining));
	Attack();
}

void AEnemy::Attack()
{
	Super::Attack();

	if (!IsTargetValid() || IsTargetInSafeZone(combatTarget))
	{
		FinishAttackSequence();
		return;
	}

	SetEnemyState(EEnemyState::EES_Engaged);
	--comboHitsRemaining;

	if (UsesDataAttacks())
	{
		StartDataAttack();
		return;
	}

	if (PlayAttackMontage() < 0)
		FinishAttackSequence();
}

void AEnemy::AttackEnd()
{
	if (UsesDataAttacks())
		return;

	if (IsDead() || enemyState != EEnemyState::EES_Engaged)
		return;

	if (comboHitsRemaining > 0 && IsTargetValid() && IsWithin(combatTarget, GetAttackRadius() * attackRangeExitMultiplier))
	{
		Attack();
		return;
	}

	FinishAttackSequence();
}

void AEnemy::FinishAttackSequence()
{
	comboHitsRemaining = 0;

	const float restTime = (UsesDataAttacks() && archetypeData) ? archetypeData->minAttackInterval : attackCooldown;
	nextAttackTime = GetNow() + restTime;
	if (actionSelector)
		actionSelector->NotifyAttackCompleted();

	UE_LOG(LogWindless, Log, TEXT("[AI] %s 공격 종료(쉬는 시간 %.1f초)"), *GetName(), restTime);
	SetEnemyState(EEnemyState::EES_Attacking);
}

bool AEnemy::UsesDataAttacks() const
{
	return attackRunner && attackData.Num() > 0;
}

void AEnemy::StartDataAttack()
{
	int32 index = attackData.IsValidIndex(pendingAttackIndex) ? pendingAttackIndex : FMath::RandRange(0, attackData.Num() - 1);
	pendingAttackIndex = INDEX_NONE;
	if (!debugForcedAttackId.IsNone())
	{
		const int32 forcedIndex = attackDataIds.IndexOfByKey(debugForcedAttackId);
		if (forcedIndex != INDEX_NONE)
			index = forcedIndex;
		else
			UE_LOG(LogWindless, Warning, TEXT("[AI] %s 시험용 고정 공격 %s가 원형 공격 목록에 없음(무작위로 실행)"), *GetName(), *debugForcedAttackId.ToString());
	}

	bAttackTouched = false;
	bAttackLanded = false;
	bCurrentAttackHeavy = attackData[index]->attackClass == EEnemyAttackClass::EATC_Heavy;

	if (!attackRunner->StartAttack(*attackData[index], attackDataIds[index], combatTarget, GetCombatTurnRate()))
		FinishAttackSequence();
}

void AEnemy::OnDataAttackFinished(EEnemyAttackEnd endReason)
{
	if (IsDead() || enemyState != EEnemyState::EES_Engaged)
		return;

	switch (endReason)
	{
	case EEnemyAttackEnd::EAE_Completed:
		RecordAttackResult();
		FinishAttackSequence();
		break;

	case EEnemyAttackEnd::EAE_TurnFailed:
		comboHitsRemaining = 0;
		SetEnemyState(EEnemyState::EES_Attacking);
		break;

	default:
		break;
	}
}

void AEnemy::SetAttackHitbox(bool bEnable)
{
	if (bEnable)
		ApplyAttackHitData();

	Super::SetWeaponCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

	if (!bEnable && rightHandWeapon)
		rightHandWeapon->ClearHitOverride();
}

void AEnemy::ApplyAttackHitData()
{
	const FEnemyAttackRow* attack = attackRunner ? attackRunner->GetCurrentAttack() : nullptr;
	if (!attack || !rightHandWeapon)
		return;

	const float attackPower = archetypeData ? archetypeData->attackPower : 0.f;
	const FWeaponHitOverride hitData = EnemyAttackHit::BuildHitOverride(*attack, attackRunner->GetCurrentAttackId(), attackPower, attackRunner->GetCurrentWindup());
	rightHandWeapon->SetHitOverride(hitData);

	UE_LOG(LogWindless, Log, TEXT("[적 공격] %s %s 판정 데이터: 대미지 %.1f (공격력 %.0f * 배율 %.2f), 충격량 %.0f, %s, 강한 경직 %s, 예고 %.2f초"),
		*GetName(), *hitData.attackId.ToString(), hitData.damage, attackPower, attack->damageMultiplier, hitData.impact,
		WindlessCombat::GetAttackTypeName(hitData.attackType), hitData.bCanStaggerStrong ? TEXT("가능") : TEXT("불가"), hitData.windup);
}

float AEnemy::GetAttackRadius() const
{
	if (!UsesDataAttacks())
		return attackRadius;

	float maxRange = 0.f;
	for (const FEnemyAttackRow* attack : attackData)
		maxRange = FMath::Max(maxRange, attack->GetRange());

	if (maxRange <= 0.f)
		return attackRadius;

	const float engageRange = (archetypeData && archetypeData->preferredRange > 0.f) ? FMath::Min(archetypeData->preferredRange, maxRange) : maxRange;
	return engageRange * 100.f;
}

float AEnemy::GetCombatTurnRate() const
{
	return (archetypeData && archetypeData->turnRateCombat > 0.f) ? archetypeData->turnRateCombat : 180.f;
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead()) // 사망 후 피격 무시
		return 0.f;

	HandleDamage(DamageAmount);

	APawn* attacker = EventInstigator ? EventInstigator->GetPawn() : nullptr;
	if (attacker)
	{
		bProvoked = true;
		bForcedChase = true;

		const bool bSeesAttacker = perception && perception->HasLineOfSight();
		if (perception)
			perception->ForceTrack(attacker);

		if (!bSeesAttacker)
			UE_LOG(LogWindless, Log, TEXT("[AI] %s 피격 - 대상 미확인, 강제 추적 (거리 %.0f)"), *GetName(),
				FVector::Dist(GetActorLocation(), attacker->GetActorLocation()));

		/* 비전투 중 피격되면 즉시 교전 */
		if (!IsInCombat() || enemyState == EEnemyState::EES_Searching)
			EngageTarget(attacker, false);
		else if (!IsTargetValid())
			combatTarget = attacker;
	}

	return DamageAmount;
}

bool AEnemy::ReceiveWeaponHit(const FCombatHitInfo& hitInfo)
{
	if (IsDead()) // 사망 후 피격 무효 처리
		return false;

	FCombatHitInfo finalHit = hitInfo;

	/* 기습 피격(비전투 상태에서 웅크린 공격자에게 맞으면 추가 피해) */
	const ACharacter* attackerCharacter = Cast<ACharacter>(hitInfo.attacker);
	const bool bMeleeHit = hitInfo.attackType != EAttackType::EAT_Ranged;
	if (bMeleeHit && !IsInCombat() && attackerCharacter && attackerCharacter->bIsCrouched)
	{
		const float before = finalHit.damage;
		finalHit.damage *= sneakAttackMultiplier;
		UE_LOG(LogWindless, Log, TEXT("[AI] 기습 공격 %s x%.2f 대미지 %.1f -> %.1f"), *GetName(), sneakAttackMultiplier, before, finalHit.damage);
	}

	if (!weakPoints)
		return Super::ReceiveWeaponHit(finalHit);

	const FWeakPointHitResult weakHit = weakPoints->EvaluateHit(finalHit);
	if (!weakHit.bCritical)
	{
		UE_LOG(LogWindless, Log, TEXT("[약점] 일반 적중 %s 본=%s 부위=%s"), *GetName(),
			*hitInfo.hitBoneName.ToString(), *weakHit.regionName.ToString());
		return Super::ReceiveWeaponHit(finalHit);
	}

	const float beforeCritical = finalHit.damage;
	finalHit.damage *= weakHit.multiplier;

	UE_LOG(LogWindless, Log, TEXT("[약점] 크리티컬 %s 부위=%s 배율=x%.2f (각성 x%.2f) 대미지 %.1f -> %.1f"),
		*GetName(), *weakHit.regionName.ToString(), weakHit.multiplier, weakHit.awakeningBonus, beforeCritical, finalHit.damage);

	return Super::ReceiveWeaponHit(finalHit);
}

float AEnemy::GetTenacity() const
{
	return archetypeData ? archetypeData->toughness : Super::GetTenacity();
}

void AEnemy::DieByFall()
{
	if (IsDead())
		return;

	bFellToDeath = true;
	UE_LOG(LogWindless, Log, TEXT("[낙사] %s 낙사 처치(영구 사망)"), *GetName());
	Super::DieByFall();
}

void AEnemy::GetHit_Implementation(const FVector& impactPoint, AActor* hitter)
{
	Super::GetHit_Implementation(impactPoint, hitter);

	if (!IsDead())
		ShowHealthBar();
}

void AEnemy::OnDealtWeaponHit()
{
	Super::OnDealtWeaponHit();
	bAttackTouched = true;
}

void AEnemy::OnWeaponHitLanded()
{
	Super::OnWeaponHitLanded();
	bAttackLanded = true;
}

void AEnemy::HandleDamage(float damageAmount)
{
	Super::HandleDamage(damageAmount);
	if (attributes && healthBarWidget)
		healthBarWidget->SetHealthPercent(attributes->GetHealthPercent());
}

void AEnemy::OnStaggered(const FVector& impactPoint, AActor* hitter)
{
	Super::OnStaggered(impactPoint, hitter);

	if (IsDead())
		return;

	StopMovement();
	comboHitsRemaining = 0;
	SetEnemyState(EEnemyState::EES_Staggered);

	if (interrupt)
		interrupt->ApplyStagger(GetLastStaggerResult(), hitter ? hitter->GetActorLocation() : impactPoint);
}

void AEnemy::EndStagger()
{
	if (IsDead() || enemyState != EEnemyState::EES_Staggered)
		return;

	if (!IsTargetValid())
	{
		HandleNoTarget();
		return;
	}

	if (IsWithin(combatTarget, GetAttackRadius()))
	{
		nextAttackTime = FMath::Max(nextAttackTime, GetNow() + attackStartDelay);
		SetEnemyState(EEnemyState::EES_Attacking);
	}
	else
		StartChase();
}

void AEnemy::ResolveArchetypeData()
{
	archetypeData = nullptr;
	attackData.Reset();
	attackDataIds.Reset();
	profile = FMonsterProfile();

	if (archetypeRow.IsNull())
		return;

	archetypeData = archetypeRow.GetRow<FMonsterArchetypeRow>(TEXT("AEnemy::ResolveArchetypeData"));
	if (!archetypeData)
		return;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const UDataTable* attackTable = settings ? settings->attackTable.LoadSynchronous() : nullptr;

	for (const FName& attackId : archetypeData->attackIds)
	{
		const FEnemyAttackRow* attack = attackTable ? attackTable->FindRow<FEnemyAttackRow>(attackId, TEXT("AEnemy::ResolveArchetypeData"), false) : nullptr;
		if (attack)
		{
			attackData.Add(attack);
			attackDataIds.Add(attackId);
		}
	}

	UE_LOG(LogWindless, Log, TEXT("[AI 데이터] %s 원형 %s (%s) 로드 - 공격 %d / %d종"), *GetName(),
		*archetypeRow.RowName.ToString(), *archetypeData->monsterId, attackData.Num(), archetypeData->attackIds.Num());

	profile = EnemyAIProfile::Resolve(*archetypeData, idleTypeOverride);
	for (const FString& line : EnemyAIProfile::DescribeLines(profile))
		UE_LOG(LogWindless, Log, TEXT("[AI 프로필] %s %s"), *GetName(), *line);
}

void AEnemy::ApplyArchetypeWeakPoints()
{
	if (!weakPoints || !archetypeData)
		return;

	const FWeakPointRule rule = WindlessWeakPoint::MakeArchetypeRule(*archetypeData);
	if (rule.bFixed && rule.fixedNames.Num() == 0)
	{
		UE_LOG(LogWindless, Warning, TEXT("[약점] %s 보스 원형의 고정 약점이 비어 있음"), *GetName());
	}
	else if (!rule.bFixed && archetypeData->fixedWeakPoints.Num() > 0)
	{
		UE_LOG(LogWindless, Warning, TEXT("[약점] %s 일반, 엘리트 원형의 고정 약점은 쓰지 않음"), *GetName());
	}
	weakPoints->ApplyRule(rule);
}

bool AEnemy::UsesCardSelection() const
{
	return actionSelector && actionSelector->IsReady() && UsesDataAttacks();
}

void AEnemy::UpdateCardCombat(float DeltaTime)
{
	if (!bCardRunning)
	{
		SelectNextCard();
		return;
	}

	if (!bGlaring && CheckPlayerSignals())
		return;

	/* 노려보기 중 경로가 다시 생기면 바로 재교전 */
	if (bGlaring)
	{
		glareRecheckTimer -= DeltaTime;
		if (glareRecheckTimer <= 0.f)
		{
			glareRecheckTimer = 0.5f;
			UNavigationPath* path = nullptr;
			if (combatTarget && !IsTargetFalling() && JudgeTargetReach(BuildCardContext(), path) == EEnemyReach::EER_Reachable)
			{
				UE_LOG(LogWindless, Log, TEXT("[AI] %s 노려보기 중 경로가 다시 생김, 재교전"), *GetName());
				EndCurrentCard();
				return;
			}
		}
	}

	if (cardMover->UpdateCard(DeltaTime))
		return;

	bCardRunning = false;
	if (bGlaring)
	{
		bGlaring = false;
		StartReturn(TEXT("노려보기 끝"));
	}
}

void AEnemy::SelectNextCard()
{
	/* 경직 회복 직후 보정 */
	if (interrupt && interrupt->ConsumeStaggerRecovered())
	{
		actionSelector->RecordLastAct(EEnemyLastAct::ELA_StaggerRecovered);
		UE_LOG(LogWindless, Log, TEXT("[AI] %s 경직 회복 직후 보정 1회 사용"), *GetName());
	}

	if (UPlayerAISignal* signals = GetTargetSignals())
		seenSignalSerial = signals->GetEventSerial();

	const FEnemyCardContext context = BuildCardContext();
	if (TryStartGlare(context))
		return;

	ExecuteCardChoice(actionSelector->Select(context));
}

void AEnemy::EndCurrentCard()
{
	bGlaring = false;
	if (!bCardRunning)
		return;

	bCardRunning = false;
	if (cardMover)
		cardMover->StopCard();
}

bool AEnemy::TryStartGlare(const FEnemyCardContext& context)
{
	if (!cardMover || !archetypeData || archetypeData->moveClass == EMonsterMoveClass::EMC_Fixed)
		return false;

	if (IsTargetFalling())
		return false;

	UNavigationPath* path = nullptr;
	const EEnemyReach reach = JudgeTargetReach(context, path);
	if (reach == EEnemyReach::EER_Reachable)
		return false;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const FVector start = GetActorLocation();
	const bool bPartial = path && path->IsPartial();
	const float pathLength = path ? path->GetPathLength() / 100.f : 0.f;
	const float straight = FVector::Dist(start, combatTarget->GetActorLocation()) / 100.f;

	/* 일부 경로면 가서, 아니면 제자리에서 노려봄 */
	const bool bMoveFirst = bPartial && FVector::Dist2D(path->PathPoints.Last(), start) > 100.f;
	const float glareTime = settings ? settings->glareTime : 4.f;
	cardMover->StartGlare(combatTarget, bMoveFirst ? path->PathPoints.Last() : start, bMoveFirst, glareTime);

	currentCard = EEnemyCard::EC_Rest;
	bCardRunning = true;
	bGlaring = true;
	UE_LOG(LogWindless, Log, TEXT("[AI] %s 도달 불가(%s, 직선 %.1fm, 경로 %.1fm)(%s %.0f초 노려보기)"), *GetName(),
		EnemyReach::GetReachName(reach), straight, pathLength, bMoveFirst ? TEXT("가장 가까운 지점에서") : TEXT("제자리에서"), glareTime);
	return true;
}

EEnemyReach AEnemy::JudgeTargetReach(const FEnemyCardContext& context, UNavigationPath*& outPath)
{
	outPath = nullptr;
	if (!combatTarget || !archetypeData)
		return EEnemyReach::EER_Reachable;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const float heightLimit = settings ? settings->pathCheckHeight : 3.f;
	if (!EnemyReach::ShouldCheckPath(context.distance, context.heightDelta, archetypeData->preferredRange, heightLimit))
		return EEnemyReach::EER_Reachable;

	const FVector start = GetActorLocation();
	const FVector goal = combatTarget->GetActorLocation();
	UNavigationPath* path = UNavigationSystemV1::FindPathToLocationSynchronously(this, start, goal, this);
	const bool bFound = path && path->IsValid() && path->PathPoints.Num() > 0;
	const bool bPartial = bFound && path->IsPartial();
	const float pathLength = bFound ? path->GetPathLength() / 100.f : 0.f;
	const float straight = FVector::Dist(start, goal) / 100.f;
	outPath = bFound ? path : nullptr;
	return EnemyReach::Judge(bFound, bPartial, pathLength, straight, settings ? settings->unreachablePathRatio : 3.f);
}

bool AEnemy::IsTargetFalling() const
{
	const ACharacter* targetCharacter = Cast<ACharacter>(combatTarget);
	const UCharacterMovementComponent* movement = targetCharacter ? targetCharacter->GetCharacterMovement() : nullptr;
	return movement && movement->IsFalling();
}

FEnemyCardContext AEnemy::BuildCardContext()
{
	FEnemyCardContext context;
	const FVector delta = combatTarget->GetActorLocation() - GetActorLocation();
	context.distance = delta.Size2D() / 100.f;
	context.heightDelta = delta.Z / 100.f;
	context.bPlayerStaggered = IsTargetStaggered();

	const FVector forward = GetActorForwardVector().GetSafeNormal2D();
	const float dot = FVector::DotProduct(forward, delta.GetSafeNormal2D());
	context.angleFromFront = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(dot, -1.f, 1.f)));

	if (const UPlayerAISignal* signals = GetTargetSignals())
		context.player = signals->GetSnapshot();

	if (cardMover)
	{
		context.bSideStepBlocked = !cardMover->CanSideStep(combatTarget);
		context.bBackOffBlocked = !cardMover->CanBackOff(combatTarget);
	}

	return context;
}

void AEnemy::ExecuteCardChoice(const FEnemyCardChoice& choice)
{
	/* 공격 카드 */
	if (attackData.IsValidIndex(choice.attackIndex))
	{
		StopMovement();
		pendingAttackIndex = choice.attackIndex;
		BeginAttackSequence();
		return;
	}

	/* 공격이 아닌 카드 */
	EEnemyCard card = choice.card;
	if (!cardMover->StartCard(card, combatTarget))
	{
		card = EEnemyCard::EC_Rest;
		cardMover->StartCard(card, combatTarget);
	}

	currentCard = card;
	bCardRunning = true;
	actionSelector->NotifyCardStarted(card);
}

bool AEnemy::CheckPlayerSignals()
{
	UPlayerAISignal* signals = GetTargetSignals();
	if (!signals || signals->GetEventSerial() == seenSignalSerial)
		return false;

	seenSignalSerial = signals->GetEventSerial();
	const EPlayerAIState signal = signals->GetLastEvent();

	const FEnemyCardContext context = BuildCardContext();
	if (!actionSelector->GetActivePlayerStates(context).Contains(signal))
		return false;

	const UEnemyAISettings* settings = UEnemyAISettings::Get();
	const float interval = settings ? settings->signalReselectInterval : 0.5f;
	const uint8 key = static_cast<uint8>(signal);
	const float* lastTime = signalReselectTime.Find(key);
	if (lastTime && GetNow() - *lastTime < interval)
		return false;

	signalReselectTime.Add(key, GetNow());

	UE_LOG(LogWindless, Log, TEXT("[AI] %s 즉시 다시 고르기(%s, %s 하던 중)"), *GetName(), EnemyCardScore::GetPlayerStateName(signal), EnemyCardScore::GetCardName(currentCard));
	const FEnemyCardChoice choice = actionSelector->Select(context);

	if (!attackData.IsValidIndex(choice.attackIndex) && choice.card == currentCard)
	{
		UE_LOG(LogWindless, Log, TEXT("[AI] %s 같은 카드라 하던 행동 유지"), *GetName());
		return false;
	}

	EndCurrentCard();
	ExecuteCardChoice(choice);
	return true;
}

void AEnemy::RecordAttackResult()
{
	if (!UsesCardSelection())
		return;

	if (bAttackLanded)
		actionSelector->RecordLastAct(bCurrentAttackHeavy ? EEnemyLastAct::ELA_HeavyHit : EEnemyLastAct::ELA_LightHit);
	else if (!bAttackTouched)
		actionSelector->RecordLastAct(EEnemyLastAct::ELA_Whiff); // 아무것도 닿지 않음 = 플레이어가 범위를 벗어남
	else
		UE_LOG(LogWindless, Log, TEXT("[카드] %s 공격이 가드나 무적에 막힘"), *GetName());
}

UPlayerAISignal* AEnemy::GetTargetSignals() const
{
	return combatTarget ? combatTarget->FindComponentByClass<UPlayerAISignal>() : nullptr;
}

void AEnemy::OnEndured()
{
	Super::OnEndured();
	if (UsesCardSelection())
		actionSelector->RecordLastAct(EEnemyLastAct::ELA_Endured);
}

bool AEnemy::IsKnockbackImmune() const
{
	return archetypeData && archetypeData->bKnockbackImmune;
}

void AEnemy::Die_Implementation()
{
	if (attackRunner)
		attackRunner->CancelAttack(TEXT("사망"));

	Super::Die_Implementation();
	bForcedChase = false;
	SetEnemyState(EEnemyState::EES_Dead);

	StopMovement();

	if (interrupt)
		interrupt->CancelInterrupt();

	if (perception)
		perception->SetPerceptionEnabled(false);

	HideHealthBar();
	SetLifeSpan(deathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (bFellToDeath)
		GrantDropsToPlayer();
	else
		SpawnSouls();

	DisableCapsule();
	DisableMeshCollision();
}

void AEnemy::SpawnSouls()
{
	UWorld* world = GetWorld();
	if (world && soulClass && attributes)
	{
		const FVector spawnLocation = GetActorLocation() + FVector(0.f, 0.f, 125.f);
		ASoul* spawnedSoul = world->SpawnActor<ASoul>(soulClass, spawnLocation, GetActorRotation());
		if (spawnedSoul)
		{
			spawnedSoul->SetSouls(attributes->GetMoltStoneReward());
			spawnedSoul->SetOwner(this);
		}
	}
}

void AEnemy::GrantDropsToPlayer()
{
	const int32 reward = attributes ? attributes->GetMoltStoneReward() : 0;
	ABaseCharacter* player = Cast<ABaseCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	UAttributeComponent* playerAttributes = player ? player->GetAttributes() : nullptr;
	if (!playerAttributes)
	{
		UE_LOG(LogWindless, Warning, TEXT("[낙사] %s 탈피석을 받을 플레이어가 없음"), *GetName());
		return;
	}

	if (reward > 0)
		playerAttributes->AddMoltStones(reward);

	UE_LOG(LogWindless, Log, TEXT("[낙사] %s 탈피석 획득(탈피석 %d)"), *GetName(), reward);
}

void AEnemy::Destroyed()
{
	Super::Destroyed();
	if (rightHandWeapon) // 적 장착 무기 파괴
		rightHandWeapon->Destroy();
}

void AEnemy::HideHealthBar()
{
	if (healthBarWidget)
		healthBarWidget->SetVisibility(false);
}

void AEnemy::ShowHealthBar()
{
	if (healthBarWidget)
		healthBarWidget->SetVisibility(true);
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* world = GetWorld();
	if (world && weaponClass)
	{
		AWeapon* defaultWeapon = world->SpawnActor<AWeapon>(weaponClass);
		defaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
		rightHandWeapon = defaultWeapon;
	}
}