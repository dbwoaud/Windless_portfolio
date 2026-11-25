#include "Items/Weapons/Projectiles/ArrowProjectile.h"
#include "Characters/BaseCharacter.h"
#include "Interfaces/HitInterface.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Windless/DebugMacro.h"

AArrowProjectile::AArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	collision->InitSphereRadius(3.f);
	collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	collision->SetCollisionObjectType(ECC_WorldDynamic);
	collision->SetCollisionResponseToAllChannels(ECR_Block);
	collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SetRootComponent(collision);

	arrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	arrowMesh->SetupAttachment(collision);
	arrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	projectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	projectileMovement->UpdatedComponent = collision;
	projectileMovement->bRotationFollowsVelocity = true;
	projectileMovement->InitialSpeed = 0.f;
	projectileMovement->MaxSpeed = 0.f;
	projectileMovement->bAutoActivate = false;
}

void AArrowProjectile::Launch(const FVector& direction, const FArrowShotParams& inShot, AActor* ignoreActor)
{
	shot = inShot;

	if (AActor* owner = GetOwner())
		collision->IgnoreActorWhenMoving(owner, true);
	if (ignoreActor)
		collision->IgnoreActorWhenMoving(ignoreActor, true);

	projectileMovement->ProjectileGravityScale = shot.gravityScale;
	projectileMovement->Velocity = direction.GetSafeNormal() * shot.speed;
	projectileMovement->Activate(true);
}

void AArrowProjectile::BeginPlay()
{
	Super::BeginPlay();
	collision->OnComponentHit.AddDynamic(this, &AArrowProjectile::OnArrowHit);
	SetLifeSpan(flightLifeSpan);
}

void AArrowProjectile::OnArrowHit(UPrimitiveComponent* hitComponent, AActor* otherActor, UPrimitiveComponent* otherComp, FVector normalImpulse, const FHitResult& hit)
{
	if (bHasHit || !otherActor || otherActor == GetOwner())
		return;

	bHasHit = true;

	if (!IsFriendlyTarget(otherActor))
		ApplyHit(hit);

	if (hitSound)
		UGameplayStatics::PlaySoundAtLocation(this, hitSound, hit.ImpactPoint);

	StickTo(hit);
}

bool AArrowProjectile::IsFriendlyTarget(const AActor* target) const
{
	const AActor* owner = GetOwner();
	return owner && target && owner->ActorHasTag(FName("Enemy")) && target->ActorHasTag(FName("Enemy"));
}

void AArrowProjectile::StickTo(const FHitResult& hit)
{
	projectileMovement->StopMovementImmediately();
	projectileMovement->Deactivate();
	collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (USceneComponent* hitComp = hit.GetComponent())
		AttachToComponent(hitComp, FAttachmentTransformRules::KeepWorldTransform, hit.BoneName);

	SetLifeSpan(stuckLifeSpan);
}

void AArrowProjectile::ApplyHit(const FHitResult& hit)
{
	AActor* target = hit.GetActor();
	if (!target)
		return;

	AController* instigatorController = GetInstigator() ? GetInstigator()->GetController() : nullptr;

	ABaseCharacter* hitCharacter = Cast<ABaseCharacter>(target);
	if (!hitCharacter)
	{
		UGameplayStatics::ApplyDamage(target, shot.damage, instigatorController, this, UDamageType::StaticClass());
		return;
	}

	FCombatHitInfo hitInfo;
	hitInfo.attacker = GetOwner();
	hitInfo.causer = this;
	hitInfo.instigatorController = instigatorController;
	hitInfo.hitMesh = Cast<USkeletalMeshComponent>(hit.GetComponent());
	hitInfo.hitBoneName = hit.BoneName;
	hitInfo.impactPoint = hit.ImpactPoint;
	hitInfo.attackType = EAttackType::EAT_Ranged;
	hitInfo.damage = shot.damage;
	hitInfo.impact = shot.impact;
	hitInfo.injuryChance = shot.injuryChance;
	hitInfo.bUnblockable = shot.bUnblockable;

	UE_LOG(LogWindless, Log, TEXT("[활] 적중 %s -> %s 차징=%.0f%% 대미지=%.1f 충격량=%.1f"),
		*GetNameSafe(GetOwner()), *GetNameSafe(target), shot.chargeRatio * 100.f, shot.damage, shot.impact);

	if (ABaseCharacter* shooter = Cast<ABaseCharacter>(GetOwner()))
		shooter->OnDealtRangedHit();

	if (hitCharacter->ReceiveWeaponHit(hitInfo))
	{
		if (IHitInterface* hitInterface = Cast<IHitInterface>(target))
			hitInterface->Execute_GetHit(target, hit.ImpactPoint, GetOwner());
	}
}
