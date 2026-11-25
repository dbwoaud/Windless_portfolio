#include "Pawns/Bird.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ABird::ABird()
{
	PrimaryActorTick.bCanEverTick = true;
	capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	capsule->SetCapsuleHalfHeight(20.f);
	capsule->SetCapsuleRadius(15.f);
	SetRootComponent(capsule);

	birdMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BirdMesh"));
	birdMesh->SetupAttachment(GetRootComponent());

	springArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	springArm->SetupAttachment(GetRootComponent());
	springArm->TargetArmLength = 300.f;

	viewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	viewCamera->SetupAttachment(springArm);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ABird::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABird::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(moveAction, ETriggerEvent::Triggered, this, &ABird::Move);
		EnhancedInputComponent->BindAction(lookAction, ETriggerEvent::Triggered, this, &ABird::Look);
	}
}

void ABird::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* playerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
			subsystem->AddMappingContext(birdMappingContext, 0);
	}
}

void ABird::Move(const FInputActionValue& value)
{
	const float directionValue = value.Get<float>();
	if (Controller && (directionValue != 0.f))
	{
		FVector forward = GetActorForwardVector();
		AddMovementInput(forward, directionValue);
	}		
}

void ABird::Look(const FInputActionValue& value)
{
	const FVector2D lookAxisValue = value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(lookAxisValue.X);
		AddControllerPitchInput(lookAxisValue.Y);
	}
}