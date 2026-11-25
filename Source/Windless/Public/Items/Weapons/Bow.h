#pragma once

#include "CoreMinimal.h"
#include "Items/Weapons/Weapon.h"
#include "Bow.generated.h"

class AArrowProjectile;

UCLASS()
class WINDLESS_API ABow : public AWeapon
{
	GENERATED_BODY()

public:
	ABow();
	/* 활(화살) */
	FVector GetArrowSpawnLocation() const; // 화살 생성 위치를 반환하는 함수


	/* 활(차징)*/
	FArrowShotParams BuildShotParams(float chargeRatio) const; // 차징 비율로 화살 발사 정보를 구성하는 함수
	float GetChargeRatio(float heldTime) const; // 당긴 시간을 차징 비율로 변환하는 함수

protected:
	/* 활(화살) */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Arrow")
	TSubclassOf<AArrowProjectile> arrowClass; // 발사할 화살 클래스

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Arrow")
	FName ammoItemId = FName("Ammo_SplitArrow"); // 소모할 화살 아이템 ID

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Arrow")
	FName arrowSocketName = FName("ArrowSocket"); // 활 메시의 화살 생성 소켓


	/* 활(차징) */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Charge", meta = (ClampMin = "0.0"))
	float minDrawTime = 0.3f; // 발사 가능한 최소 당김 시간

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Charge", meta = (ClampMin = "0.0"))
	float fullChargeTime = 1.2f; // 최대 차징까지 걸리는 시간

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Charge", meta = (ClampMin = "0.0"))
	float minDamageMultiplier = 0.5f; // 최소 차징 대미지 배율

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Charge", meta = (ClampMin = "0.0"))
	float maxDamageMultiplier = 1.5f; // 최대 차징 대미지 배율

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Charge", meta = (ClampMin = "0.0"))
	float minArrowSpeed = 1500.f; // 최소 차징 화살 속도

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Charge", meta = (ClampMin = "0.0"))
	float maxArrowSpeed = 5000.f; // 최대 차징 화살 속도

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Charge", meta = (ClampMin = "0.0"))
	float arrowGravityScale = 0.5f; // 화살 중력 배율


	/* 활(발사) */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Shot")
	FAttackSpec shotSpec; // 발사 1회 설정

public:
	/* 변수 Getter 함수 */
	FORCEINLINE TSubclassOf<AArrowProjectile> GetArrowClass() const { return arrowClass; }
	FORCEINLINE FName GetAmmoItemId() const { return ammoItemId; }
	FORCEINLINE float GetShotStaminaCost() const { return shotSpec.staminaCost; }
	FORCEINLINE float GetMinDrawTime() const { return minDrawTime; }
};