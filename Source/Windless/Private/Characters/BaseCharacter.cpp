#include "Characters/BaseCharacter.h"
#include "Characters/BodyPartTypes.h"
#include "Characters/CombatTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Items/Weapons/Weapon.h"
#include "Windless/DebugMacro.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	bodyParts = CreateDefaultSubobject<UBodyPartComponent>(TEXT("BodyParts"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateFallTracking();
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (!bIsTrackingFall) // 추락 추적 중이 아니면 무시
		return;

	const float currentZ = GetActorLocation().Z;
	const float fallDistance = fallStartZ - currentZ;

	if (fallDistance >= landSoundMinDistance) // 피해가 없는 착지에도 소리를 낸다
	{
		UE_LOG(LogWindless, Log, TEXT("[착지] %s %.1f m 낙하, 착지 소리 %s"), *GetName(), fallDistance / 100.f, landSound ? TEXT("재생") : TEXT("없음(칸이 빔)"));
		if (landSound)
			UGameplayStatics::PlaySoundAtLocation(this, landSound, GetActorLocation());
	}

	ApplyFallDamage(fallDistance); 
	bIsTrackingFall = false;
	fallStartZ = 0.f;
}

void ABaseCharacter::DieByFall()
{
	if (!IsAlive())
		return;

	if (attributes)
		attributes->ReceiveDamage(TNumericLimits<float>::Max());

	Die();
}

void ABaseCharacter::OnDealtWeaponHit()
{

}

void ABaseCharacter::OnDealtRangedHit()
{

}

float ABaseCharacter::GetAttackPower() const
{
	return attributes ? attributes->GetFinalAttackPower() : 0.f;
}

float ABaseCharacter::GetAttackPowerWith(const AWeapon* weapon) const
{
	return GetAttackPower();
}

void ABaseCharacter::SetPendingHit(const FCombatHitInfo& hitInfo)
{
	pendingHitImpact = hitInfo.impact;
	pendingHitWindup = hitInfo.windup;
	bPendingHitCanStaggerStrong = hitInfo.bCanStaggerStrong;
}

void ABaseCharacter::ApplyBodyPartHit(FName boneName, float damage)
{
	if (bodyParts)
		bodyParts->ApplyHitToBone(boneName, damage);
}

void ABaseCharacter::ApplyBodyPartHitAt(const USkeletalMeshComponent* mesh, const FVector& impactPoint, float damage)
{
	if (bodyParts)
		bodyParts->ApplyHitAtLocation(mesh, impactPoint, damage);
}

bool ABaseCharacter::ReceiveWeaponHit(const FCombatHitInfo& hitInfo)
{
	if (bInvincible)
		return false;

	SetPendingHit(hitInfo);
	UGameplayStatics::ApplyDamage(this, hitInfo.damage, hitInfo.instigatorController, hitInfo.causer, UDamageType::StaticClass());
	return true;
}

float ABaseCharacter::GetTenacity() const
{
	return attributes ? attributes->GetTenacity() : 0.f;
}

FStaggerStatus ABaseCharacter::GetStaggerStatus() const
{
	return FStaggerStatus();
}

void ABaseCharacter::SetInvincible(bool bEnable)
{
	bInvincible = bEnable;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseCharacter::Attack()
{
	if (combatTarget && combatTarget->ActorHasTag(FName("Dead")))
		combatTarget = nullptr;
}

void ABaseCharacter::AttackEnd()
{

}

bool ABaseCharacter::CanAttack()
{
	return false;
}

void ABaseCharacter::HandleDamage(float damageAmount)
{
	if (attributes)
		attributes->ReceiveDamage(damageAmount);
}

void ABaseCharacter::GetHit_Implementation(const FVector& impactPoint, AActor* hitter)
{
	PlayHitSound(impactPoint);
	SpawnHitParticle(impactPoint);

	if (!IsAlive())
	{
		Die();
		return;
	}

	/* 경직 판정 */
	const bool bForced = bForceStaggerOnHit;
	bForceStaggerOnHit = false;

	FStaggerInput input;
	input.impact = pendingHitImpact;
	input.toughness = GetTenacity();
	input.attackWindup = pendingHitWindup;
	input.bCanStaggerStrong = bPendingHitCanStaggerStrong;
	input.bKnockbackImmune = IsKnockbackImmune();
	const EStaggerResult result = bForced ? EStaggerResult::ESR_Weak : WindlessCombat::ResolveStagger(input);
	lastStaggerResult = result;

	switch (result)
	{
	case EStaggerResult::ESR_Strong:
		UE_LOG(LogWindless, Log, TEXT("[경직] 강한 경직 %s 충격량=%.1f >= 강인도=%.1f x%.0f"),
			*GetName(), pendingHitImpact, GetTenacity(), WindlessCombat::StrongStaggerMultiplier);
		OnStaggered(impactPoint, hitter);
		break;

	case EStaggerResult::ESR_Weak:
		UE_LOG(LogWindless, Log, TEXT("[경직] 약한 경직 %s 충격량=%.1f 강인도=%.1f%s"), *GetName(), pendingHitImpact, GetTenacity(),
			bForced ? TEXT(" (가드 브레이크)") : (pendingHitImpact >= GetTenacity() * WindlessCombat::StrongStaggerMultiplier ? TEXT(" (강한 경직 불가/날아감 면역)") : TEXT("")));
		OnStaggered(impactPoint, hitter);
		break;

	case EStaggerResult::ESR_ShortWindup:
		UE_LOG(LogWindless, Log, TEXT("[경직] 경직 없음 %s 예고 %.2f초 < %.2f초 (충격량=%.1f)"), *GetName(), pendingHitWindup, WindlessCombat::StaggerMinWindup, pendingHitImpact);
		OnEndured();
		break;

	default:
		UE_LOG(LogWindless, Log, TEXT("[경직] 버팀 %s 충격량=%.1f <= 강인도=%.1f"), *GetName(), pendingHitImpact, GetTenacity());
		OnEndured();
		break;
	}
}

void ABaseCharacter::OnStaggered(const FVector& impactPoint, AActor* hitter)
{
	CancelCurrentAttack(); // 공격 모션 중이었다면 강제 취소

	if (hitter)
		DirectionalHitReact(hitter->GetActorLocation());
}

void ABaseCharacter::OnEndured()
{
}

void ABaseCharacter::CancelCurrentAttack()
{
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	StopAttackMontage();
}

void ABaseCharacter::DirectionalHitReact(const FVector& impactPoint)
{
	const FVector forward = GetActorForwardVector();
	const FVector impactLowered(impactPoint.X, impactPoint.Y, GetActorLocation().Z);
	const FVector toHit = (impactLowered - GetActorLocation()).GetSafeNormal();
	const double cosTheta = FVector::DotProduct(forward, toHit);
	double theta = FMath::Acos(cosTheta);
	theta = FMath::RadiansToDegrees(theta);

	const FVector crossProduct = FVector::CrossProduct(forward, toHit);

	/* 피격 각도에 따른 애니메이션 재생 */
	if (crossProduct.Z < 0)
		theta *= -1.f;

	FName section("FromBack");
	if (theta >= -45.f && theta < 45.f)
		section = FName("FromFront");

	else if (theta >= -135.f && theta < -45.f)
		section = FName("FromLeft");

	else if (theta >= 45.f && theta < 135.f)
		section = FName("FromRight");

	PlayHitReactMontage(section);
}

void ABaseCharacter::PlayHitSound(const FVector& impactPoint)
{
	if (hitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			hitSound,
			impactPoint
		);
	}
}

void ABaseCharacter::SpawnHitParticle(const FVector& impactPoint)
{
	if (hitParticles && GetWorld())
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			hitParticles,
			impactPoint
		);
	}
}


void ABaseCharacter::UpdateFallTracking()
{
	if (!GetCharacterMovement())
		return;

	const bool bIsFalling = GetCharacterMovement()->IsFalling();

	if (bIsFalling && !bIsTrackingFall)
	{
		fallStartZ = GetActorLocation().Z;
		bIsTrackingFall = true;
	}

	if (bIsFalling && bIsTrackingFall)
	{
		const float currentZ = GetActorLocation().Z;
		if (currentZ > fallStartZ)
			fallStartZ = currentZ;
	}
}

void ABaseCharacter::ApplyFallDamage(float fallDistance)
{
	if (fallDistance <= safeFallDistance) // 낙하 거리가 최소 낙하 데미지 적용 거리보다 작은 경우 무시
		return;

	if (!IsAlive()) // 캐릭터 사망 시 무시
		return;

	const float damage = CalculateFallDamage(fallDistance);
	if (bodyParts && IsPlayerControlled()) // 플레이어 캐릭터면 낙하 데미지를 양 다리에 대미지 절반씩 적용
	{
		bodyParts->ApplyDamageToPart(EBodyPart::EBP_LeftLeg, damage * 0.5f);
		bodyParts->ApplyDamageToPart(EBodyPart::EBP_RightLeg, damage * 0.5f);
	}
	else // 적이면 낙하 데미지를 바로 적용
	{
		UGameplayStatics::ApplyDamage(this, damage, GetController(), this, UDamageType::StaticClass());
	}

	/* VFX 및 SFX 연출 */
	const FVector impactPoint = GetActorLocation() - FVector(0.f, 0.f, GetSimpleCollisionHalfHeight());
	PlayHitSound(impactPoint);
	SpawnHitParticle(impactPoint);

	/* 애니메이션 연출 */
	if (!IsAlive())
		Die();
	else
		PlayHitReactMontage(FName("FromFront"));
}

float ABaseCharacter::CalculateFallDamage(float fallDistance) const
{
	if (fallDistance >= lethalFallDistance) // 낙하 거리가 최대 낙하 데미지 적용 거리보다 큰 경우 
		return maxFallDamage;

	const float fallRange = lethalFallDistance - safeFallDistance;
	const float fallOverSafe = fallDistance - safeFallDistance;
	const float damageRatio = fallOverSafe / fallRange;
	return maxFallDamage * damageRatio; // 거리에 따른 데미지 비례 적용
}

void ABaseCharacter::Die_Implementation()
{
	Tags.Add(FName("Dead"));
	PlayDeathMontage();
}

bool ABaseCharacter::IsAlive()
{
	return attributes && attributes->IsAlive();
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DisableMeshCollision()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseCharacter::DodgeEnd()
{

}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (rightHandWeapon && rightHandWeapon->GetWeaponBox())
	{
		rightHandWeapon->ignoreActors.Empty();
		rightHandWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
	}
}

int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(attackMontage, attackMontageSections);
}

void ABaseCharacter::StopAttackMontage()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance)
		animInstance->Montage_Stop(0.25f, attackMontage);
}

void ABaseCharacter::PlayHitReactMontage(const FName& sectionName)
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && hitReactMontage)
	{
		animInstance->Montage_Play(hitReactMontage);
		animInstance->Montage_JumpToSection(sectionName, hitReactMontage);
	}
}

int32 ABaseCharacter::PlayDeathMontage()
{
	const int selection = PlayRandomMontageSection(deathMontage, deathMontageSections);
	TEnumAsByte<EDeathPose> pose(selection);
	if (pose < EDeathPose::EDP_MAX)
		deathPose = pose;

	return selection;
}

void ABaseCharacter::PlayDodgeMontage()
{
	PlayMontageSection(dodgeMontage, FName("Default"));
}

FVector ABaseCharacter::GetTranslationWarpTarget()
{
	if (combatTarget == nullptr)
		return FVector();

	const FVector combatTargetLocation = combatTarget->GetActorLocation();
	const FVector location = GetActorLocation();
	FVector targetToMe = (location - combatTargetLocation).GetSafeNormal();
	targetToMe *= warpTargetDistance;
	return combatTargetLocation + targetToMe;
}

FVector ABaseCharacter::GetRotationWarpTarget()
{
	if (combatTarget == nullptr)
		return FVector();
	return combatTarget->GetActorLocation();
}

void ABaseCharacter::PlayMontageSection(UAnimMontage* montage, const FName& sectionName)
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && montage)
	{
		animInstance->Montage_Play(montage);
		animInstance->Montage_JumpToSection(sectionName, montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* montage, const TArray<FName>& sectionNames)
{
	if (sectionNames.Num() <= 0)
		return -1;

	const int32 maxSectionIndex = sectionNames.Num() - 1;
	const int32 selection = FMath::RandRange(0, maxSectionIndex);
	PlayMontageSection(montage, sectionNames[selection]);
	return selection;
}
