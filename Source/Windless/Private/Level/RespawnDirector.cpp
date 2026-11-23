#include "Level/RespawnDirector.h"
#include "Level/SavePointSubsystem.h"
#include "Level/SoulMarkerSubsystem.h"
#include "Characters/WindlessCharacter.h"
#include "Mount/SnakeMount.h"
#include "Windless/DebugMacro.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Enemy/EnemyRespawnSubsystem.h"


ARespawnDirector::ARespawnDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARespawnDirector::BeginPlay()
{
	Super::BeginPlay();
	CachePlayer();
	if (playerCharacter)
	{
		playerCharacter->onPlayerDeath.AddDynamic(this, &ARespawnDirector::HandlePlayerDeath);
		startLocation = playerCharacter->GetActorLocation();
		startRotation = playerCharacter->GetActorRotation();
	}
	if (UWorld* world = GetWorld())
	{
		if (USoulMarkerSubsystem* markers = world->GetSubsystem<USoulMarkerSubsystem>())
			markers->SetMarkerClass(soulMarkerClass);
	}

}

void ARespawnDirector::HandlePlayerDeath()
{
	if (playerCharacter)
	{
		deathLocation = playerCharacter->GetActorLocation();

		if (UAttributeComponent* attributes = playerCharacter->FindComponentByClass<UAttributeComponent>())
		{
			const int32 lostStones = attributes->GetMoltStones();

			UWorld* world = GetWorld();
			if (USoulMarkerSubsystem* markers = world ? world->GetSubsystem<USoulMarkerSubsystem>() : nullptr)
				markers->CreateMarker(deathLocation, lostStones);

			attributes->SpendMoltStones(lostStones);
		}
	}
	DisablePlayerControl();
	OnRespawnStartedBP();
	BeginSnakeCinematic();
}

void ARespawnDirector::OnSnakeMontageEnded(UAnimMontage* montage, bool bInterrupted)
{
	if (montage != snakeGrabMontage)
		return;

	if (cachedSnakeAnim)
	{
		cachedSnakeAnim->OnMontageEnded.RemoveDynamic(this, &ARespawnDirector::OnSnakeMontageEnded);
		cachedSnakeAnim = nullptr;
	}
	OnSnakeCinematicComplete();
}

void ARespawnDirector::CachePlayer()
{
	playerCharacter = Cast<AWindlessCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	playerController = UGameplayStatics::GetPlayerController(this, 0);
}

void ARespawnDirector::DisablePlayerControl()
{
	if (playerCharacter && playerController)
		playerCharacter->DisableInput(playerController);
}

void ARespawnDirector::TeleportAndRevive()
{
	if (!playerCharacter)
		return;

	FVector  loc;
	FRotator rot;
	ResolveRespawnTransform(loc, rot);

	playerCharacter->SetActorLocationAndRotation(loc, rot, false, nullptr, ETeleportType::TeleportPhysics);
	playerCharacter->Revive();
	FadeInAndRestore();
}

void ARespawnDirector::ResolveRespawnTransform(FVector& outLocation, FRotator& outRotation) const
{
	// 1순위: 가장 최근에 활성화한 세이브포인트
	const UWorld* world = GetWorld();
	if (const USavePointSubsystem* savePoints = world ? world->GetSubsystem<USavePointSubsystem>() : nullptr)
	{
		FSavePointRecord record;
		if (savePoints->FindLatestActivated(record))
		{
			outLocation = record.respawnLocation;
			outRotation = record.respawnRotation;
			UE_LOG(LogWindless, Log, TEXT("[리스폰] 최근 활성화 세이브포인트: %s (#%d)"), *record.savePointId.ToString(), record.activationOrder);
			return;
		}
	}

	// 2순위: 레벨에 지정된 기본 부활 지점
	if (defaultRespawnPoint)
	{
		outLocation = defaultRespawnPoint->GetActorLocation();
		outRotation = defaultRespawnPoint->GetActorRotation();
		return;
	}

	// 3순위: 시작 위치
	outLocation = startLocation;
	outRotation = startRotation;
}

void ARespawnDirector::FinishRespawn()
{
	OnRespawnedBP();

	UWorld* world = GetWorld();
	if (UEnemyRespawnSubsystem* respawn = world ? world->GetSubsystem<UEnemyRespawnSubsystem>() : nullptr)
		respawn->RespawnAll();

	onRespawnFinished.Broadcast();
}

void ARespawnDirector::EnablePlayerControl()
{
	if (playerCharacter && playerController)
		playerCharacter->EnableInput(playerController);
}

void ARespawnDirector::BeginSnakeCinematic()
{
	if (rescueSnake && playerCharacter)
		rescueSnake->SetActorLocation(playerCharacter->GetActorLocation());

	UAnimInstance* snakeAnim = (rescueSnake && rescueSnake->GetMountMesh())
		? rescueSnake->GetMountMesh()->GetAnimInstance() : nullptr;

	if (snakeAnim && snakeGrabMontage)
	{
		cachedSnakeAnim = snakeAnim;
		snakeAnim->OnMontageEnded.AddDynamic(this, &ARespawnDirector::OnSnakeMontageEnded);
		snakeAnim->Montage_Play(snakeGrabMontage);
	}
	else
	{
		UE_LOG(LogWindless, Log, TEXT("[리스폰] 타이머 대체 사용 (%.1fs)"), snakeCinematicDuration);
		GetWorldTimerManager().SetTimer(cinematicTimer, this, &ARespawnDirector::OnSnakeCinematicComplete, snakeCinematicDuration, false);
	}
}

void ARespawnDirector::OnSnakeCinematicComplete()
{
	FadeOut();
	GetWorldTimerManager().SetTimer(fadeOutTimer, this, &ARespawnDirector::TeleportAndRevive, fadeOutDuration, false);
}

void ARespawnDirector::FadeInAndRestore()
{
	if (playerController && playerController->PlayerCameraManager)
		playerController->PlayerCameraManager->StartCameraFade(1.f, 0.f, fadeInDuration, fadeColor, false, false);

	EnablePlayerControl();
	FinishRespawn();
}

void ARespawnDirector::FadeOut()
{
	if (playerController && playerController->PlayerCameraManager)
		playerController->PlayerCameraManager->StartCameraFade(0.f, 1.f, fadeOutDuration, fadeColor, false, true);
}