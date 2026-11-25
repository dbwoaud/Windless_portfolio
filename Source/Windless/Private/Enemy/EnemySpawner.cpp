#include "Enemy/EnemySpawner.h"
#include "Enemy/Enemy.h"
#include "Enemy/EnemyRespawnSubsystem.h"
#include "Level/TriggerVolumeBase.h"
#include "Windless/DebugMacro.h"
#include "Engine/World.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = sceneRoot;
}

void AEnemySpawner::RespawnEnemy(bool bIncludeAlive)
{
	if (bBossSpawner)
		return;

	if (IsEnemyAlive() && !bIncludeAlive)
		return;

	if (IsValid(spawnedEnemy))
	{
		spawnedEnemy->OnDestroyed.RemoveDynamic(this, &AEnemySpawner::HandleEnemyDeath);
		spawnedEnemy->Destroy();
	}

	spawnedEnemy = nullptr;
	bEnemyDead = false;

	if (triggerVolume) // 트리거 스포너는 바로 만들지 않고 트리거를 다시 기다린다
	{
		triggerVolume->ResetTrigger();
		return;
	}

	SpawnEnemy();
}

bool AEnemySpawner::IsEnemyAlive() const
{
	return IsValid(spawnedEnemy) && !spawnedEnemy->ActorHasTag(FName("Dead"));
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();
	if (UEnemyRespawnSubsystem* respawn = world ? world->GetSubsystem<UEnemyRespawnSubsystem>() : nullptr)
		respawn->RegisterSpawner(this);

	if (triggerVolume) // 트리거가 있으면 시작할 때 만들지 않는다
	{
		triggerVolume->onTriggerEnter.AddDynamic(this, &AEnemySpawner::HandleTriggerEnter);
		return;
	}

	if (bSpawnOnBeginPlay && !bEnemyDead)
		SpawnEnemy();
}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorld* world = GetWorld();
	if (UEnemyRespawnSubsystem* respawn = world ? world->GetSubsystem<UEnemyRespawnSubsystem>() : nullptr)
		respawn->UnregisterSpawner(this);

	if (EndPlayReason == EEndPlayReason::RemovedFromWorld || EndPlayReason == EEndPlayReason::LevelTransition)
	{
		if (IsValid(spawnedEnemy))
		{
			spawnedEnemy->OnDestroyed.RemoveDynamic(this, &AEnemySpawner::HandleEnemyDeath); // ★추가
			spawnedEnemy->Destroy();
		}
		spawnedEnemy = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AEnemySpawner::SpawnEnemy()
{
	UWorld* world = GetWorld();
	if (!world || !enemyClass)
		return;

	const FTransform spawnTransform(GetActorRotation(), GetActorLocation());
	spawnedEnemy = world->SpawnActorDeferred<AEnemy>(enemyClass, spawnTransform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!spawnedEnemy)
		return;

	spawnedEnemy->SetFlags(RF_Transient);
	spawnedEnemy->SetPlacement(archetypeOverride, idleTypeOverride);
	spawnedEnemy->FinishSpawning(spawnTransform);

	bEnemyDead = false;
	spawnedEnemy->SetPatrolRoute(patrolPoints);
	spawnedEnemy->OnDestroyed.AddDynamic(this, &AEnemySpawner::HandleEnemyDeath);

	UE_LOG(LogWindless, Log, TEXT("[스포너] %s: %s 생성 (원형 %s)"), *GetName(), *spawnedEnemy->GetName(),
		archetypeOverride.IsNull() ? TEXT("BP 기본값") : *archetypeOverride.RowName.ToString());
}

void AEnemySpawner::HandleTriggerEnter(AActor* activator)
{
	if (!IsValid(spawnedEnemy) && !bEnemyDead) // 이미 생성했거나 죽은 뒤면 무시
		SpawnEnemy();
}

void AEnemySpawner::HandleEnemyDeath(AActor* destroyedActor)
{
	if (destroyedActor && destroyedActor->ActorHasTag(FName("Dead")))
		bEnemyDead = true;

	spawnedEnemy = nullptr;
}

