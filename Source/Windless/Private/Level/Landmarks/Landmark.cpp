#include "Level/Landmarks/Landmark.h"
#include "Level/Landmarks/LandmarkSubsystem.h"
#include "Components/SphereComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Windless/DebugMacro.h"
#include "Engine/World.h"
#include "Level/Landmarks/Landmark.h"

ALandmark::ALandmark()
{
	PrimaryActorTick.bCanEverTick = false;

	sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = sceneRoot;

	discoverySphere = CreateDefaultSubobject<USphereComponent>(TEXT("DiscoverySphere"));
	discoverySphere->SetupAttachment(sceneRoot);
	discoverySphere->InitSphereRadius(discoveryRadius);
	discoverySphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	discoverySphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	discoverySphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ALandmark::OnConstruction(const FTransform& transform)
{
	Super::OnConstruction(transform);

	if (discoverySphere)
		discoverySphere->SetSphereRadius(discoveryRadius);
}

void ALandmark::BeginPlay()
{
	Super::BeginPlay();

	if (!discoverySphere)
		return;

	discoverySphere->SetSphereRadius(discoveryRadius);
	discoverySphere->OnComponentBeginOverlap.AddDynamic(this, &ALandmark::OnDiscoveryBeginOverlap);

	TArray<AActor*> overlapping;
	discoverySphere->GetOverlappingActors(overlapping, AWindlessCharacter::StaticClass());
	for (AActor* actor : overlapping)
		OnDiscoveryBeginOverlap(discoverySphere, actor, nullptr, 0, false, FHitResult());
}

void ALandmark::OnDiscoveryBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Cast<AWindlessCharacter>(OtherActor))
		return;

	UWorld* world = GetWorld();
	ULandmarkSubsystem* landmarks = world ? world->GetSubsystem<ULandmarkSubsystem>() : nullptr;

	if (landmarks && landmarks->DiscoverLandmark(this))
		OnDiscoveredBP();
}

