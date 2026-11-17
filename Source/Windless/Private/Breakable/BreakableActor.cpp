#include "Breakable/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Items/Treasure.h"
#include "Components/CapsuleComponent.h"

ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	geometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	SetRootComponent(geometryCollection);

	geometryCollection->SetGenerateOverlapEvents(true);
	geometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 파괴된 조각에 대해 카메라가 충돌 무시하도록 설정
	geometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); // 파괴된 조각에 대해 플레이어가 충돌 무시하도록 설정

	capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	capsule->SetupAttachment(GetRootComponent());
	capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); 
	capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block); // 파괴 가능한 객체의 캡슐 콜리전에 대해 플레이어가 충돌 막히도록 설정 
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABreakableActor::GetHit_Implementation(const FVector& impactPoint, AActor* hitter)
{
	if (bBroken) // 이미 파괴되었으면 무시
		return;
	
	bBroken = true;
	UWorld* world = GetWorld();
	SpawnTreasureItem(world); // 깨진 자리에 보물 아이템 생성
}

void ABreakableActor::SpawnTreasureItem(UWorld* world)
{
	if (world && treasureClasses.Num() > 0)
	{
		FVector location = GetActorLocation();
		location.Z += 75.f;
		const int32 selection = FMath::RandRange(0, treasureClasses.Num() - 1);
		world->SpawnActor<ATreasure>(treasureClasses[selection], location, GetActorRotation());
	}
}