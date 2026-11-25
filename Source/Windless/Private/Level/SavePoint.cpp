#include "Level/SavePoint.h"
#include "Level/SavePointSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/InteractionSphereComponent.h"
#include "Components/MaintenanceComponent.h"
#include "Characters/WindlessCharacter.h"
#include "NavAreas/NavArea_Null.h"
#include "Windless/DebugMacro.h"
#include "Engine/World.h"
#include "Enemy/EnemyRespawnSubsystem.h" 

ASavePoint::ASavePoint()
{
	PrimaryActorTick.bCanEverTick = false;

	sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = sceneRoot;

	savePointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SavePointMesh"));
	savePointMesh->SetupAttachment(sceneRoot);
	savePointMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	interactionSphere = CreateDefaultSubobject<UInteractionSphereComponent>(TEXT("InteractionSphere"));
	interactionSphere->SetupAttachment(sceneRoot);

	respawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("RespawnPoint"));
	respawnPoint->SetupAttachment(sceneRoot);

	saveZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SaveZoneSphere"));
	saveZoneSphere->SetupAttachment(sceneRoot);
	saveZoneSphere->InitSphereRadius(saveZoneRadius);
	saveZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	saveZoneSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	saveZoneSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	saveZoneSphere->SetCanEverAffectNavigation(true);
	saveZoneSphere->bDynamicObstacle = true;
	saveZoneSphere->SetAreaClassOverride(UNavArea_Null::StaticClass());
}

void ASavePoint::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);
	if (saveZoneSphere)
		saveZoneSphere->SetSphereRadius(saveZoneRadius);
}

void ASavePoint::BeginPlay()
{
	Super::BeginPlay();

	if (saveZoneSphere)
	{
		saveZoneSphere->SetSphereRadius(saveZoneRadius);
		saveZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &ASavePoint::OnSaveZoneBeginOverlap);
		saveZoneSphere->OnComponentEndOverlap.AddDynamic(this, &ASavePoint::OnSaveZoneEndOverlap);
	}

#if !UE_BUILD_SHIPPING
	if (savePointId.IsNone())
		UE_LOG(LogWindless, Error, TEXT("[세이브] savePointId 미지정: %s (디테일 패널에서 지정 필요)"), *GetName());
#endif

	if (USavePointSubsystem* savePoints = GetSavePointSubsystem())
		savePoints->RegisterSavePoint(this);
	RefreshActivationState();

	if (saveZoneSphere)
	{
		TArray<AActor*> overlappingActors;
		saveZoneSphere->GetOverlappingActors(overlappingActors, AWindlessCharacter::StaticClass());
		for (AActor* actor : overlappingActors)
			HandlePlayerEnter(Cast<AWindlessCharacter>(actor));
	}
}

void ASavePoint::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(overlappingPlayer))
		HandlePlayerExit(overlappingPlayer);
	overlappingPlayer = nullptr;

	if (USavePointSubsystem* savePoints = GetSavePointSubsystem())
		savePoints->UnregisterSavePoint(this);

	Super::EndPlay(EndPlayReason);
}

void ASavePoint::Interact_Implementation(AActor* interactor)
{
	AWindlessCharacter* player = Cast<AWindlessCharacter>(interactor);
	USavePointSubsystem* savePoints = GetSavePointSubsystem();
	if (!player || !savePoints || savePointId.IsNone())
		return;

	if (savePoints->ActivateSavePoint(this))
	{
		RefreshActivationState();
		OnActivatedBP();
	}

	savePoints->TouchActivationOrder(savePointId);
	ApplyRestEffects(player);
	OnRestedBP();

	if (UMaintenanceComponent* maintenance = player->FindComponentByClass<UMaintenanceComponent>())
		maintenance->EnterMaintenance(this);
}

FText ASavePoint::GetInteractionText_Implementation()
{
	return IsActivated() ? interactionText : activateText;
}

bool ASavePoint::CanInteract_Implementation(AActor* interactor)
{
	return !savePointId.IsNone();
}

FVector ASavePoint::GetRespawnLocation() const
{
	return respawnPoint ? respawnPoint->GetComponentLocation() : GetActorLocation();
}

FRotator ASavePoint::GetRespawnRotation() const
{
	return respawnPoint ? respawnPoint->GetComponentRotation() : GetActorRotation();
}

bool ASavePoint::IsActivated() const
{
	const USavePointSubsystem* savePoints = GetSavePointSubsystem();
	return savePoints && savePoints->IsActivated(savePointId);
}

void ASavePoint::RefreshActivationState()
{
	const bool bActivated = IsActivated();
	if (bActivationStateInitialized && bActivated == bCachedActivated)
		return;

	bActivationStateInitialized = true;
	bCachedActivated = bActivated;

	if (IsValid(overlappingPlayer))
		SetBodyPartRegen(overlappingPlayer, bActivated);

	OnActivationStateChangedBP(bActivated);
}

bool ASavePoint::IsLocationInSafeZone(const FVector& location) const
{
	if (!saveZoneSphere)
		return false;

	const float radius = saveZoneSphere->GetScaledSphereRadius();
	return FVector::DistSquared(location, saveZoneSphere->GetComponentLocation()) <= FMath::Square(radius);
}

void ASavePoint::OnSaveZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	HandlePlayerEnter(Cast<AWindlessCharacter>(OtherActor));
}

void ASavePoint::OnSaveZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	HandlePlayerExit(Cast<AWindlessCharacter>(OtherActor));
}

void ASavePoint::HandlePlayerEnter(AWindlessCharacter* player)
{
	if (!player || overlappingPlayer == player)
		return;

	overlappingPlayer = player;
	player->EnterLightZone();
	SetBodyPartRegen(player, IsActivated());
}

void ASavePoint::HandlePlayerExit(AWindlessCharacter* player)
{
	if (!player || overlappingPlayer != player)
		return;

	overlappingPlayer = nullptr;
	player->ExitLightZone();
	SetBodyPartRegen(player, false);
}

void ASavePoint::SetBodyPartRegen(AWindlessCharacter* player, bool bActive) const
{
	if (!player)
		return;

	if (UBodyPartComponent* bodyParts = player->FindComponentByClass<UBodyPartComponent>())
		bodyParts->SetSaveZoneActive(bActive);
}

void ASavePoint::ApplyRestEffects(AWindlessCharacter* player)
{
	player->RestAtSavePoint();

	UWorld* world = GetWorld();
	if (UEnemyRespawnSubsystem* respawn = world ? world->GetSubsystem<UEnemyRespawnSubsystem>() : nullptr)
		respawn->RespawnAll();

	player->RequestSave(savePointId, GetRespawnLocation(), GetRespawnRotation());
}

USavePointSubsystem* ASavePoint::GetSavePointSubsystem() const
{
	UWorld* world = GetWorld();
	return world ? world->GetSubsystem<USavePointSubsystem>() : nullptr;
}