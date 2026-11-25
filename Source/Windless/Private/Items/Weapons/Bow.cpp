#include "Items/Weapons/Bow.h"
#include "Items/Weapons/Projectiles/ArrowProjectile.h"
#include "Components/StaticMeshComponent.h"

ABow::ABow()
{
	shotSpec.staminaCost = 15.f;
	shotSpec.impact = 15.f;
}

FVector ABow::GetArrowSpawnLocation() const
{
	if (ItemMesh && ItemMesh->DoesSocketExist(arrowSocketName))
		return ItemMesh->GetSocketLocation(arrowSocketName);

	return GetActorLocation();
}

FArrowShotParams ABow::BuildShotParams(float chargeRatio) const
{
	FArrowShotParams params;
	params.chargeRatio = FMath::Clamp(chargeRatio, 0.f, 1.f);
	params.damage = WindlessCombat::CalcChargedDamage(GetOwnerAttackPower(), params.chargeRatio, minDamageMultiplier, maxDamageMultiplier);
	params.impact = shotSpec.impact;
	params.injuryChance = GetInjuryChance();
	params.bUnblockable = shotSpec.bUnblockable;
	params.speed = FMath::Lerp(minArrowSpeed, maxArrowSpeed, params.chargeRatio);
	params.gravityScale = arrowGravityScale;
	return params;
}

float ABow::GetChargeRatio(float heldTime) const
{
	if (fullChargeTime <= minDrawTime)
		return 1.f;

	return FMath::Clamp((heldTime - minDrawTime) / (fullChargeTime - minDrawTime), 0.f, 1.f);
}
