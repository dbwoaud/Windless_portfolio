#include "Level/BaseDoor.h"
#include "Windless/DebugMacro.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InteractionSphereComponent.h"
#include "Staging/BaseStagingActor.h"
#include "Save/WindlessSaveSubsystem.h"
#include "Engine/GameInstance.h"

static UWindlessSaveSubsystem* DoorSaveSubsystem(const AActor* door)
{
	const UGameInstance* gameInstance = door ? door->GetGameInstance() : nullptr;
	return gameInstance ? gameInstance->GetSubsystem<UWindlessSaveSubsystem>() : nullptr;
}

ABaseDoor::ABaseDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	doorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
	RootComponent = doorRoot;

	leftHinge = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHinge"));
	leftHinge->SetupAttachment(doorRoot);

	leftLeaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftLeaf"));
	leftLeaf->SetupAttachment(leftHinge);

	rightHinge = CreateDefaultSubobject<USceneComponent>(TEXT("RightHinge"));
	rightHinge->SetupAttachment(doorRoot);

	rightLeaf = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightLeaf"));
	rightLeaf->SetupAttachment(rightHinge);

	interactionSphere = CreateDefaultSubobject<UInteractionSphereComponent>(TEXT("InteractionSphere"));
	interactionSphere->SetupAttachment(doorRoot);
}

void ABaseDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bMoving)
		return;

	const float target = bOpen ? openRatio : 0.f;
	currentAlpha = FMath::FInterpTo(currentAlpha, target, DeltaTime, openSpeed);
	ApplyDoorRotation(currentAlpha);

	if (FMath::IsNearlyEqual(currentAlpha, target, 0.01f)) // 문 목표 상태 도달 시
	{
		currentAlpha = target;
		ApplyDoorRotation(currentAlpha);
		bMoving = false;
		SetActorTickEnabled(false);
	}
}

void ABaseDoor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetupDoor();
}

void ABaseDoor::Interact_Implementation(AActor* interactor)
{
	if (!bCanInteractable)
		return;

	if (!bOpen)
	{
		if (!IsOnOpenSide(interactor))
		{
			UE_LOG(LogWindless, Log, TEXT("문 잠김(반대쪽에서 여는 문): %s"), *GetName());
			return;
		}
		UpdateOpenDirection(interactor);
		OpenDoor();
	}

	else if (!bCanInteractableOnce && doorId.IsNone()) // 지름길 문은 열린 채로 둔다
		CloseDoor();
}

FText ABaseDoor::GetInteractionText_Implementation()
{
	return bLockedSide ? lockedText : interactionText;
}

bool ABaseDoor::CanInteract_Implementation(AActor* interactor)
{
	if (!bCanInteractable || bPlayingOpenEffect)
		return false;

	if ((bCanInteractableOnce || !doorId.IsNone()) && bOpen)
		return false;

	bLockedSide = !bOpen && !IsOnOpenSide(interactor); // 못 여는 쪽에서는 글만 바뀐다
	return true;
}

void ABaseDoor::OpenDoor()
{
	if (bOpen) // 이미 열려 있으면 무시
		return;

	bOpen = true;
	bMoving = true;

	SetActorTickEnabled(true);
	OnDoorStartOpen();
	if (bPlayEffectOnOpen && openEffect)
	{
		bPlayingOpenEffect = true;
		openEffect->Play();
	}


	if (!doorId.IsNone())
	{
		if (UWindlessSaveSubsystem* save = DoorSaveSubsystem(this))
			save->MarkDoorOpened(doorId);
	}

	UE_LOG(LogWindless, Log, TEXT("문 열기: %s"), *GetName());
}

void ABaseDoor::CloseDoor()
{
	if (!bOpen) // 이미 닫혀 있으면 무시
		return;

	bOpen = false;
	bMoving = true;

	SetActorTickEnabled(true);
	OnDoorStartClose();
	UE_LOG(LogWindless, Log, TEXT("문 닫기: %s"), *GetName());
}

void ABaseDoor::BeginPlay()
{
	Super::BeginPlay();
	ApplyDoorRotation(0.f);

	if (!doorId.IsNone())
	{
		// 세이브는 문보다 늦게 읽힐 수 있다. 지금 한 번 맞추고, 불러오기가 끝날 때 다시 맞춘다
		if (UWindlessSaveSubsystem* save = DoorSaveSubsystem(this))
			save->onGameLoaded.AddUniqueDynamic(this, &ABaseDoor::RestoreOpened);
		RestoreOpened();
	}
}

bool ABaseDoor::IsOnOpenSide(const AActor* interactor) const
{
	if (openFrom == EDoorOpenSide::EDOS_Both || !interactor)
		return true;

	const float dot = FVector::DotProduct(GetActorRightVector(), interactor->GetActorLocation() - GetActorLocation());
	return (openFrom == EDoorOpenSide::EDOS_Right) == (dot >= 0.f);
}

void ABaseDoor::RestoreOpened()
{
	const UWindlessSaveSubsystem* save = DoorSaveSubsystem(this);
	if (bOpen || doorId.IsNone() || !save || !save->IsDoorOpened(doorId))
		return;

	openDirection = (openFrom == EDoorOpenSide::EDOS_Left) ? -1.f : 1.f; // 여는 쪽의 반대로 열렸던 방향
	bOpen = true;
	bMoving = false;
	currentAlpha = openRatio;
	ApplyDoorRotation(currentAlpha);
	UE_LOG(LogWindless, Log, TEXT("문 복원(열림): %s [%s]"), *GetName(), *doorId.ToString());
}

void ABaseDoor::SetupDoor()
{
	if (!leftLeaf || !rightLeaf || !leftHinge || !rightHinge)
		return;

	const float scaleMag = FMath::Abs(leftLeaf->GetRelativeScale3D().Z);
	const float actualWidth = doorWidth * scaleMag;

	auto SetMirror = [](UStaticMeshComponent* leaf, bool bMirror)
		{
			FVector s = leaf->GetRelativeScale3D();
			s.X = FMath::Abs(s.X) * (bMirror ? -1.f : 1.f);
			leaf->SetRelativeScale3D(s);
		};

	switch (doorType)
	{
		case EDoorType::EDT_LeftSingle:
			leftHinge->SetRelativeLocation(FVector::ZeroVector);
			SetMirror(leftLeaf, true);
			leftLeaf->SetVisibility(true);
			rightLeaf->SetVisibility(false);
			break;

		case EDoorType::EDT_RightSingle:
			leftHinge->SetRelativeLocation(FVector::ZeroVector);
			SetMirror(leftLeaf, false);
			leftLeaf->SetVisibility(true);
			rightLeaf->SetVisibility(false);
			break;

		case EDoorType::EDT_Double:
			leftHinge->SetRelativeLocation(FVector(-actualWidth, 0.f, 0.f));
			SetMirror(leftLeaf, true);
			leftLeaf->SetVisibility(true);

			rightHinge->SetRelativeLocation(FVector(actualWidth, 0.f, 0.f));
			if (leftLeaf->GetStaticMesh())
				rightLeaf->SetStaticMesh(leftLeaf->GetStaticMesh());
			{
				FVector rs = leftLeaf->GetRelativeScale3D();
				rs.X = FMath::Abs(rs.X);
				rightLeaf->SetRelativeScale3D(rs);
			}
			rightLeaf->SetVisibility(true);
			break;
	}
}

void ABaseDoor::ApplyDoorRotation(float alpha)
{
	const float baseYaw = openAngle * alpha;

	switch (doorType)
	{
	case EDoorType::EDT_LeftSingle:
	case EDoorType::EDT_RightSingle:
	{
		const float mirrorSign = (leftLeaf && leftLeaf->GetRelativeScale3D().X < 0.f) ? -1.f : 1.f;
		leftHinge->SetRelativeRotation(FRotator(0.f, baseYaw * openDirection * mirrorSign, 0.f));
		break;
	}
	case EDoorType::EDT_Double:
		leftHinge->SetRelativeRotation(FRotator(0.f, -baseYaw * openDirection, 0.f));
		rightHinge->SetRelativeRotation(FRotator(0.f, baseYaw * openDirection, 0.f));
		break;
	}
}

void ABaseDoor::UpdateOpenDirection(AActor* interactor)
{
	if (!bOpenAwayFromInteractor || !interactor)
	{
		openDirection = 1.f;
		return;
	}

	const FVector toInteractor = interactor->GetActorLocation() - GetActorLocation();
	const float dot = FVector::DotProduct(GetActorRightVector(), toInteractor);
	openDirection = (dot >= 0.f) ? 1.f : -1.f;
}