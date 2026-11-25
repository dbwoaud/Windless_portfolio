// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Soul.h"
#include "Interfaces/PickupInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void ASoul::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MoveSoulToDesireZ(DeltaTime);
}

void ASoul::BeginPlay()
{
	Super::BeginPlay();
	const FVector start = GetActorLocation();
	const FVector end = start - FVector(0.f, 0.f, 2000.0f);
	
	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypes;
	objectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1); // WorldStatic 타입(바닥, 벽 등)의 오브젝트를 저장

	TArray<AActor*> actorsToIgnore;
	actorsToIgnore.Add(GetOwner()); // 소울을 생성시킨 액터 무시

	FHitResult hitResult;
	UKismetSystemLibrary::LineTraceSingleForObjects(
		this,
		start,
		end,
		objectTypes,
		false,
		actorsToIgnore,
		EDrawDebugTrace::None,
		hitResult,
		true
	);

	desiredZ = hitResult.ImpactPoint.Z + 50.f;
}

void ASoul::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IPickupInterface* pickupInterface = Cast<IPickupInterface>(OtherActor);
	if (pickupInterface) // PickupInterface를 상속받은 액터와 겹칠경우 소울을 획득
	{
		pickupInterface->AddSouls(this);
		SpawnPickupEffect();
		SpawnPickupSound();
		Destroy();
	}
}

void ASoul::MoveSoulToDesireZ(float DeltaTime)
{
	const double locationZ = GetActorLocation().Z;
	if (locationZ > desiredZ)
	{
		const FVector deltaLocation = FVector(0.f, 0.f, moveSpeed * DeltaTime);
		AddActorWorldOffset(deltaLocation);
	}
}