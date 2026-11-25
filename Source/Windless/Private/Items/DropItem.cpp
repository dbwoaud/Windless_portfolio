#include "Items/DropItem.h"
#include "Components/SphereComponent.h"
#include "Components/PointLightComponent.h"
#include "TimerManager.h"
#include "Windless/DebugMacro.h"

ADropItem::ADropItem()
{
	presentationScale = 0.45f;
	interactionRadius = 130.f;
	collisionRadius = 14.f;
	baseLightIntensity = 500.f;
	baseLightRadius = 160.f;
	interactionText = FText::FromString(TEXT("줍기"));

	if (itemCollision)
	{
		itemCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		itemCollision->SetSimulatePhysics(false);
		itemCollision->SetEnableGravity(true);
		itemCollision->SetLinearDamping(0.6f);
		itemCollision->SetAngularDamping(2.f);
	}
}

bool ADropItem::CanInteract_Implementation(AActor* interactor)
{
	if (bPickupLocked)
		return false;

	return Super::CanInteract_Implementation(interactor);
}

void ADropItem::DropItem(const FVector& direction)
{
	if (!itemCollision)
		return;

	itemCollision->SetSimulatePhysics(true);

	const FVector dropDir = (direction.GetSafeNormal() + FVector(0.f, 0.f, dropRatio)).GetSafeNormal();
	itemCollision->AddImpulse(dropDir * dropSpeed, NAME_None, true);

	GetWorldTimerManager().SetTimer(settleTimer, this, &ADropItem::CheckSettled, settleCheckInterval, true, settleCheckInterval * 2.f);
}

void ADropItem::BeginPlay()
{
	Super::BeginPlay();
	if (pickupLockDelay > 0.f)
	{
		bPickupLocked = true;
		GetWorldTimerManager().SetTimer(pickupLockTimer, this, &ADropItem::UnlockPickup, pickupLockDelay, false);
	}
}

void ADropItem::CheckSettled()
{
	if (!itemCollision || !itemCollision->IsSimulatingPhysics())
	{
		GetWorldTimerManager().ClearTimer(settleTimer);
		return;
	}

	if (itemCollision->GetPhysicsLinearVelocity().SizeSquared() > FMath::Square(settleSpeedThreshold))
		return;

	itemCollision->SetSimulatePhysics(false);
	itemCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorRotation(FRotator::ZeroRotator);

	GetWorldTimerManager().ClearTimer(settleTimer);
}

void ADropItem::UnlockPickup()
{
	bPickupLocked = false;
}