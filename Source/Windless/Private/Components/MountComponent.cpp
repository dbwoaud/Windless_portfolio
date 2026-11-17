#include "Components/MountComponent.h"
#include "Mount/SnakeMount.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"

UMountComponent::UMountComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMountComponent::Mount()
{
	if (!CanMount())
		return;

	OnMountPhaseChanged.Broadcast(EMountPhase::EMP_Mounting);

	if (!TrySpawnMount())
	{
		OnMountPhaseChanged.Broadcast(EMountPhase::EMP_Grounded);
		return;
	}

	OnMountPhaseChanged.Broadcast(EMountPhase::EMP_Mounting);
	SetMountCollision();
	currentMount->OnEmergeFinished.AddUObject(this, &UMountComponent::OnMountEmerged);
	currentMount->Emerge();
}

void UMountComponent::Dismount()
{
	if (!ownerCharacter || currentMount == nullptr)
		return;

	OnMountPhaseChanged.Broadcast(EMountPhase::EMP_Dismounting);

	ResetMountCollision();
	MoveToGround();
	DisapplyMountEffect();

	FDetachmentTransformRules rules(EDetachmentRule::KeepWorld, true);
	currentMount->DetachFromActor(rules);
	currentMount->OnSubmergeFinished.AddUObject(this, &UMountComponent::OnMountSubmerged);
	currentMount->Submerge();
}

void UMountComponent::BeginPlay()
{
	Super::BeginPlay();
	ownerCharacter = Cast<ACharacter>(GetOwner());
}

void UMountComponent::OnMountEmerged()
{
	if (!ownerCharacter || currentMount == nullptr)
	{
		OnMountPhaseChanged.Broadcast(EMountPhase::EMP_Grounded);
		return;
	}

	MoveToMountSocket();
	currentMount->AttachToComponent(ownerCharacter->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	currentMount->SetActorRelativeRotation(mountMeshRotationOffset);
	ApplyMountEffect();
	OnMountPhaseChanged.Broadcast(EMountPhase::EMP_Mounted);
}

bool UMountComponent::CanMount() const
{
	if (!ownerCharacter || snakeMountClass == nullptr) //소유자나 탈 것이 없으면 무시
		return false;

	if (const UCharacterMovementComponent* movement = ownerCharacter->GetCharacterMovement())
		return movement->IsMovingOnGround();

	return true;
}

bool UMountComponent::TrySpawnMount()
{
	const FVector spawnLocation = ownerCharacter->GetActorLocation() - FVector(0.f, 0.f, GetCapsuleHalfHeight());
	FActorSpawnParameters params;
	params.Owner = ownerCharacter;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	currentMount = GetWorld()->SpawnActor<ASnakeMount>(snakeMountClass, spawnLocation, ownerCharacter->GetActorRotation(), params);
	if (currentMount == nullptr)
		return false;

	currentMount->SetOwner(ownerCharacter);
	return true;
}

void UMountComponent::SetMountCollision()
{
	if (ownerCharacter->GetCapsuleComponent())
		ownerCharacter->GetCapsuleComponent()->IgnoreActorWhenMoving(currentMount, true);
}

void UMountComponent::MoveToMountSocket()
{
	USkeletalMeshComponent* snakeMesh = currentMount->GetMountMesh();
	if (!snakeMesh)
		return;

	snakeMesh->RefreshBoneTransforms();
	snakeMesh->UpdateComponentToWorld();

	const FVector socketLocation = snakeMesh->GetSocketLocation(FName("RiderSocket"));
	const FVector playerLocation = ownerCharacter->GetActorLocation() - FVector(0.f, 0.f, GetCapsuleHalfHeight());
	const FVector offset = socketLocation - playerLocation;
	ownerCharacter->AddActorWorldOffset(offset, false, nullptr, ETeleportType::TeleportPhysics);
}

void UMountComponent::ApplyMountEffect()
{
	if (UCharacterMovementComponent* move = ownerCharacter->GetCharacterMovement())
	{
		move->MaxWalkSpeed = mountSpeed;
		move->MaxStepHeight += mountStepHeightBoost;
	}
}

void UMountComponent::OnMountSubmerged()
{
	if (currentMount)
	{
		currentMount->Destroy();
		currentMount = nullptr;
	}
	OnMountPhaseChanged.Broadcast(EMountPhase::EMP_Grounded);
}

void UMountComponent::ResetMountCollision()
{
	if (ownerCharacter->GetCapsuleComponent())
		ownerCharacter->GetCapsuleComponent()->IgnoreActorWhenMoving(currentMount, false);
	if (UBoxComponent* mountBox = currentMount->GetCollisionBox())
		mountBox->IgnoreActorWhenMoving(ownerCharacter, false);
}

void UMountComponent::MoveToGround()
{
	USkeletalMeshComponent* snakeMesh = currentMount->GetMountMesh();
	if (!snakeMesh)
		return;

	const float socketLocationZ = snakeMesh->GetSocketLocation(FName("RiderSocket")).Z;
	const float playerLocationZ = ownerCharacter->GetActorLocation().Z - GetCapsuleHalfHeight();
	const float height = socketLocationZ - playerLocationZ; 
	ownerCharacter->AddActorWorldOffset(FVector(0.f, 0.f, -height), false, nullptr, ETeleportType::TeleportPhysics);
}

void UMountComponent::DisapplyMountEffect()
{
	if (UCharacterMovementComponent* move = ownerCharacter->GetCharacterMovement())
		move->MaxStepHeight -= mountStepHeightBoost;
}

float UMountComponent::GetCapsuleHalfHeight() const
{
	if (ownerCharacter && ownerCharacter->GetCapsuleComponent())
		return ownerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return 90.f;
}