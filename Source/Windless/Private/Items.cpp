#include "Items.h"
#include "Windless/DebugMacro.h"
#include "Components/SphereComponent.h"
#include "Interfaces/PickupInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AItems::AItems()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); 
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = ItemMesh;

	sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	sphere->SetupAttachment(GetRootComponent());

	itemEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
	itemEffect->SetupAttachment(GetRootComponent());
}

void AItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	runningTime += DeltaTime;

	if (itemState == EItemState::EIS_Hovering)
		AddActorWorldOffset(FVector(0.f, 0.f, TransformedSin()));
}

void AItems::BeginPlay()
{
	Super::BeginPlay();
	sphere->OnComponentBeginOverlap.AddDynamic(this, &AItems::OnSphereBeginOverlap);
	sphere->OnComponentEndOverlap.AddDynamic(this, &AItems::OnSphereEndOverlap);
}

void AItems::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* pickupInterface = Cast<IPickupInterface>(OtherActor);
	if (pickupInterface)
		pickupInterface->SetOverlappingItem(this);
}

void AItems::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IPickupInterface* pickupInterface = Cast<IPickupInterface>(OtherActor);
	if (pickupInterface)
		pickupInterface->SetOverlappingItem(nullptr);
}

void AItems::SpawnPickupEffect()
{
	if (pickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			pickupEffect,
			GetActorLocation()
		);
	}
}

void AItems::SpawnPickupSound()
{
	if (pickupSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			pickupSound,
			GetActorLocation()
		);
	}
}

float AItems::TransformedSin()
{
	return amplitude * FMath::Sin(runningTime * timeConstant);
}