#include "Characters/WindlessAnimInstance.h"
#include "Characters/WindlessCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UWindlessAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	windlessCharacter = Cast<AWindlessCharacter>(TryGetPawnOwner());
	if (windlessCharacter)
		windlessCharacterMovement = windlessCharacter->GetCharacterMovement();
}

void UWindlessAnimInstance::NativeUpdateAnimation(float deltaTime)
{
	Super::NativeUpdateAnimation(deltaTime);
	if (windlessCharacter && windlessCharacterMovement)
	{
		groundSpeed = UKismetMathLibrary::VSizeXY(windlessCharacterMovement->Velocity);
		isFalling = windlessCharacterMovement->IsFalling();
		isCrouched = windlessCharacterMovement->IsCrouching();
		isMounted = windlessCharacter->IsMounted();
		bIsStrafing = windlessCharacter->IsStrafing() && !isFalling && !isCrouched && !isMounted;
		const FVector velocity2D(windlessCharacterMovement->Velocity.X, windlessCharacterMovement->Velocity.Y, 0.f);
		if (groundSpeed > 3.f)
			moveDirection = FRotator::NormalizeAxis(velocity2D.Rotation().Yaw - windlessCharacter->GetActorRotation().Yaw);

		bIsGuarding = windlessCharacter->IsGuarding();
		bGuardOnLeft = windlessCharacter->IsGuardOnLeftHand();
		characterState = windlessCharacter->GetCharacterState();
		actionState = windlessCharacter->GetActionState();
		deathPose = windlessCharacter->GetDeathPose();
	}
}
