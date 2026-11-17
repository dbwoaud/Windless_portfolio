#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Characters/CombatTypes.h"
#include "ArrowProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class USoundBase;

UCLASS()
class WINDLESS_API AArrowProjectile : public AActor
{
	GENERATED_BODY()

public:
	AArrowProjectile();

	/* 화살(발사) */
	void Launch(const FVector& direction, const FArrowShotParams& inShot, AActor* ignoreActor); // 화살을 지정 방향으로 발사하는 함수

protected:
	virtual void BeginPlay() override;

	// 화살 피격 시 실행되는 함수
	UFUNCTION()
	void OnArrowHit(UPrimitiveComponent* hitComponent, AActor* otherActor, UPrimitiveComponent* otherComp, FVector normalImpulse, const FHitResult& hit); 

private:
	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Arrow")
	TObjectPtr<USphereComponent> collision; // 충돌 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "Arrow")
	TObjectPtr<UStaticMeshComponent> arrowMesh; // 화살 메시 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "Arrow")
	TObjectPtr<UProjectileMovementComponent> projectileMovement; // 투사체 이동 컴포넌트

	/* 상태 확인 */
	bool IsFriendlyTarget(const AActor* target) const; // 아군인지 확인하는 함수


	/* 화살(설정) */
	UPROPERTY(EditDefaultsOnly, Category = "Arrow", meta = (ClampMin = "0.1"))
	float flightLifeSpan = 8.f; // 아무것도 맞히지 못했을 때 소멸 시간

	UPROPERTY(EditDefaultsOnly, Category = "Arrow", meta = (ClampMin = "0.1"))
	float stuckLifeSpan = 10.f; // 박힌 뒤 소멸 시간

	UPROPERTY(EditDefaultsOnly, Category = "Arrow")
	TObjectPtr<USoundBase> hitSound = nullptr; // 적중 사운드


	/* 화살(상태) */
	FArrowShotParams shot; // 발사 정보
	bool bHasHit = false; // 화살 적중 여부

	void StickTo(const FHitResult& hit); // 맞은 위치에 박히는 함수


	/* 화살(공격) */
	void ApplyHit(const FHitResult& hit); // 대상에게 피해를 적용하는 함수
};