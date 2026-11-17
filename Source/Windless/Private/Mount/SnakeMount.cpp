#include "Mount/SnakeMount.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "NiagaraFunctionLibrary.h"

ASnakeMount::ASnakeMount()
{
	PrimaryActorTick.bCanEverTick = false;

	collisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	collisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(collisionBox);

	mountMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MountMesh"));
	mountMesh->SetupAttachment(collisionBox);
	mountMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASnakeMount::Emerge()
{
	SpawnGroundBurst();
	UAnimInstance* animInstance = mountMesh ? mountMesh->GetAnimInstance() : nullptr;
	if (animInstance && emergeMontage)
	{
		animInstance->Montage_Play(emergeMontage);
		FOnMontageEnded endDelegate;
		endDelegate.BindUObject(this, &ASnakeMount::HandleEmergeEnded);
		animInstance->Montage_SetEndDelegate(endDelegate, emergeMontage);
	}
	else
		OnEmergeFinished.Broadcast();
}

void ASnakeMount::Submerge()
{
	UAnimInstance* animInstance = mountMesh ? mountMesh->GetAnimInstance() : nullptr;
	if (animInstance && submergeMontage)
	{
		animInstance->Montage_Play(submergeMontage);
		FOnMontageEnded endDelegate;
		endDelegate.BindUObject(this, &ASnakeMount::HandleSubmergeEnded);
		animInstance->Montage_SetEndDelegate(endDelegate, submergeMontage);
	}
	else
		OnSubmergeFinished.Broadcast();
}

void ASnakeMount::BeginPlay()
{
	Super::BeginPlay();
}

void ASnakeMount::HandleEmergeEnded(UAnimMontage* montage, bool bInterrupted)
{
	OnEmergeFinished.Broadcast();
}

void ASnakeMount::SpawnGroundBurst()
{
	if (groundBurstEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			groundBurstEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

void ASnakeMount::HandleSubmergeEnded(UAnimMontage* montage, bool bInterrupted)
{
	OnSubmergeFinished.Broadcast();
}