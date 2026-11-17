#include "Level/SoulMarker.h"
#include "Level/SoulMarkerSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/AttributeComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Windless/DebugMacro.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

ASoulMarker::ASoulMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = sceneRoot;

	pickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	pickupSphere->SetupAttachment(sceneRoot);
	pickupSphere->InitSphereRadius(pickupRadius);
	pickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	pickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	pickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ASoulMarker::Initialize(int32 inMoltStones)
{
	moltStones = FMath::Max(0, inMoltStones);
}

void ASoulMarker::BeginPlay()
{
	Super::BeginPlay();

	if (!pickupSphere)
		return;

	pickupSphere->SetSphereRadius(pickupRadius);
	pickupSphere->OnComponentBeginOverlap.AddDynamic(this, &ASoulMarker::OnPickupBeginOverlap);
}

void ASoulMarker::OnPickupBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AWindlessCharacter* player = Cast<AWindlessCharacter>(OtherActor);
	if (!player || !player->IsAlive())
		return;

	if (UAttributeComponent* attributes = player->FindComponentByClass<UAttributeComponent>())
		attributes->AddMoltStones(moltStones);

	UE_LOG(LogWindless, Log, TEXT("[손실 회수] 탈피석 %d 회수"), moltStones);

	OnRecoveredBP();

	UWorld* world = GetWorld();
	if (USoulMarkerSubsystem* markers = world ? world->GetSubsystem<USoulMarkerSubsystem>() : nullptr)
		markers->ClearMarker();
	else
		Destroy();
}
