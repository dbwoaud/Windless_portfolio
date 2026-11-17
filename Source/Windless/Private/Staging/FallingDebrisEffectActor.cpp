#include "Staging/FallingDebrisEffectActor.h"
#include "Windless/DebugMacro.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AFallingDebrisEffectActor::AFallingDebrisEffectActor()
{
	spawnVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnVolume"));
	spawnVolume->SetupAttachment(RootComponent);
	spawnVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	spawnVolume->InitBoxExtent(FVector(200.f, 200.f, 20.f));
	spawnVolume->ShapeColor = FColor::Orange;
}

void AFallingDebrisEffectActor::HandlePlay()
{
	if (!debrisMesh)
		UE_LOG(LogWindless, Warning, TEXT("낙석 메시가 없음: %s"), *GetName());
	
	PlayDebrisVFX();
	spawnedCount = 0;
	if (spawnInterval <= 0.f) // 낙석 생성 간격이 없는 경우
	{
		for (int32 i = 0; i < debrisCount; ++i)
			SpawnSingleDebris();
	}
	else // 낙석 생성 간격이 있는 경우
	{
		SpawnSingleDebris();
		if (debrisCount > 1)
			GetWorldTimerManager().SetTimer(spawnTimer, this, &AFallingDebrisEffectActor::SpawnSingleDebris, spawnInterval, true);
	}
}

void AFallingDebrisEffectActor::SpawnSingleDebris()
{
	UWorld* world = GetWorld();
	if (!world || !debrisMesh)
		return;

	AStaticMeshActor* rock = SpawnDebrisActor(world, GetRandomSpawnLocation());
	if (!rock)
		return;

	SetupDebris(rock);
	if (++spawnedCount >= debrisCount)
		GetWorldTimerManager().ClearTimer(spawnTimer);
}

FVector AFallingDebrisEffectActor::GetRandomSpawnLocation() const
{
	const FVector origin = spawnVolume->GetComponentLocation();
	const FVector extent = spawnVolume->GetScaledBoxExtent();
	return origin + FVector(
		FMath::FRandRange(-extent.X, extent.X),
		FMath::FRandRange(-extent.Y, extent.Y),
		FMath::FRandRange(-extent.Z, extent.Z));
}

AStaticMeshActor* AFallingDebrisEffectActor::SpawnDebrisActor(UWorld* world, const FVector& location)
{
	const FRotator randomRotation(
		FMath::FRandRange(0.f, 360.f),
		FMath::FRandRange(0.f, 360.f),
		FMath::FRandRange(0.f, 360.f));

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return world->SpawnActor<AStaticMeshActor>(location, randomRotation, params);
}

void AFallingDebrisEffectActor::SetupDebris(AStaticMeshActor* rock)
{
	/* 크기 설정 */
	const float scale = FMath::FRandRange(debrisScaleRange.X, debrisScaleRange.Y);
	rock->SetActorScale3D(FVector(scale));

	/* 물리 설정 */
	UStaticMeshComponent* meshComp = rock->GetStaticMeshComponent();
	rock->SetMobility(EComponentMobility::Movable);
	meshComp->SetStaticMesh(debrisMesh);
	meshComp->SetCollisionProfileName(TEXT("PhysicsActor"));
	meshComp->SetSimulatePhysics(true);
	meshComp->AddImpulse(FVector(0.f, 0.f, -downwardImpulse), NAME_None, true);

	/* 소멸 설정 */
	rock->SetLifeSpan(debrisLifespan);
}

void AFallingDebrisEffectActor::PlayDebrisVFX()
{
	if (dustEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), dustEffect, spawnVolume->GetComponentLocation(), GetActorRotation());
	if (impactSound)
		UGameplayStatics::PlaySoundAtLocation(this, impactSound, spawnVolume->GetComponentLocation());
}