#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "BaseCharacter.generated.h"

enum class EStaggerResult : uint8;
struct FCombatHitInfo;
struct FStaggerStatus;
class AWeapon;
class UAttributeComponent;
class UAnimMontage;
class UBodyPartComponent;

UCLASS()
class WINDLESS_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	/* 이동 시스템 */
	virtual void Landed(const FHitResult& Hit) override; // 캐릭터 착지 시 실행되는 함수


	/* 낙사 */
	virtual void DieByFall(); // 낙사 영역에 들어와 즉사하는 함수


	/* 전투 시스템(공격) */
	virtual void OnDealtWeaponHit(); // 근접 공격에 적 히트 시 실행되는 함수
	virtual void OnDealtRangedHit(); // 원거리 공격에 적 히트 시 실행되는 함수
	virtual void OnWeaponHitLanded() {} // 무기 공격이 실제로 대미지를 입혔을 때 실행되는 함수
	virtual float GetAttackPower() const; // 최종 공격력을 반환하는 함수
	virtual float GetAttackPowerWith(const AWeapon* weapon) const; // 무기 공격 시 공격력을 반환하는 함수
	void SetPendingHit(const FCombatHitInfo& hitInfo); // 이번 공격의 경직 정보를 설정하는 함수


	/* 전투 시스템(피격) */
	void ApplyBodyPartHit(FName boneName, float damage); // 뼈 이름으로 부위에 대미지를 적용하는 함수
	void ApplyBodyPartHitAt(const USkeletalMeshComponent* mesh, const FVector& impactPoint, float damage); // 충돌 위치로 부위 판정 후 대미지를 적용하는 함수
	virtual bool ReceiveWeaponHit(const FCombatHitInfo& hitInfo); // 무기 피격 시 실행되는 함수
	virtual float GetTenacity() const; // 강인도를 반환하는 함수
	virtual FStaggerStatus GetStaggerStatus() const; // 경직 상태를 반환하는 함수


	/* 전투 시스템(무적) */
	virtual void SetInvincible(bool bEnable); // 무적 상태를 설정하는 함수

protected:
	virtual void BeginPlay() override;
	
	/* 전투 시스템(공격) */
	virtual void Attack(); // 캐릭터 공격 시 실행되는 함수
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd(); // 캐릭터 공격 완료 시 실행되는 함수
	virtual bool CanAttack(); // 캐릭터가 공격 가능 상태인지 확인하는 함수
	virtual void HandleDamage(float damageAmount); // 공격 대미지를 처리하는 함수

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Attack")
	AActor* combatTarget; // 캐릭터의 전투 대상


	/* 전투 시스템(피격) */
	virtual void GetHit_Implementation(const FVector& impactPoint, AActor* hitter) override; // HitInterface를 구현하는 함수
	virtual void OnStaggered(const FVector& impactPoint, AActor* hitter); // 경직이 발동했을 때 실행되는 함수
	virtual void OnEndured(); // 경직을 버텨냈을 때 실행되는 함수
	virtual bool IsKnockbackImmune() const { return false; } // 날아감 면역 여부를 반환하는 함수
	virtual void CancelCurrentAttack(); // 진행 중인 공격을 강제 취소하는 함수
	void DirectionalHitReact(const FVector& impactPoint); // 방향에 따른 캐릭터 피격 시 호출되는 함수
	void PlayHitSound(const FVector& impactPoint); // 피격 사운드를 재생하는 함수
	void SpawnHitParticle(const FVector& impactPoint); // 피격 VFX를 생성하는 함수
	void UpdateFallTracking(); // 캐릭터의 낙하를 추적하는 함수
	virtual void ApplyFallDamage(float fallDistance); // 캐릭터에게 낙하 대미지를 적용하는 함수
	float CalculateFallDamage(float fallDistance) const; // 낙하 거리에 따른 낙하 대미지를 계산하는 함수
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Damage")
	float safeFallDistance = 500.f; // 최소 낙하 대미지가 적용되는 최소 낙하 대미지 거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Damage")
	float lethalFallDistance = 1500.f; // 최대 낙하 대미지가 적용되는 최소 낙하 대미지 거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Damage")
	float landSoundMinDistance = 200.f; // 착지 소리가 나는 최소 낙하 거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Damage")
	TObjectPtr<USoundBase> landSound; // 착지 소리 (비어 있으면 소리 없음)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fall Damage")
	float maxFallDamage = 100.f; // 최대 낙하 대미지


	/* 전투 시스템(사망) */
	UFUNCTION(BluePrintNativeEvent)
	void Die(); // 캐릭터 사망 시 호출되는 함수 
	virtual bool IsAlive(); // 캐릭터가 살아있는지 확인하는 함수
	void DisableCapsule(); // 캐릭터의 콜라이더를 비활성화하는 함수
	void DisableMeshCollision(); // 캐릭터의 메쉬 콜라이더를 비활성화하는 함수


	/* 전투 시스템(구르기) */
	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd(); // 캐릭터 구르기 완료 시 실행되는 함수


	/* 전투 시스템(무기) */
	UFUNCTION(BlueprintCallable)
	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled); // 장착 무기의 콜라이더를 활성화하는 함수

	UPROPERTY(VisibleAnywhere, Category = Weapon);
	AWeapon* rightHandWeapon; // 캐릭터의 오른손 무기


	/* 애니메이션 */
	virtual int32 PlayAttackMontage(); //공격 애니메이션 몽타주를 재생하는 함수
	void StopAttackMontage(); // 공격 애니메이션 몽타주를 정지하는 함수
	void PlayHitReactMontage(const FName& sectionName); // 피격 애니메이션 몽타주를 재생하는 함수
	virtual int32 PlayDeathMontage(); // 죽음 애니메이션 몽타주를 재생하는 함수
	virtual void PlayDodgeMontage(); // 구르기 애니메이션 몽타주를 재생하는 함수

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> deathPose; // 캐릭터의 죽음 애니메이션 포즈 목록


	/* 모션 워핑 */
	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget(); // 적으로부터 MotionWarping 목표의 위치를 얻는 함수
	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget(); // 적으로부터 MotionWarping 목표의 회전을 얻는 함수

	UPROPERTY(EditAnywhere, Category = Combat)
	double warpTargetDistance = 75.f; // MotionWarping 거리


	/* 캐릭터 컴포넌트 */
	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* attributes; // 캐릭터 속성 컴포넌트  

	UPROPERTY(VisibleAnywhere)
	UBodyPartComponent* bodyParts;  // 캐릭터 부위 컴포넌트

private:
	/* 전투 시스템(공격) */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Attack")
	UAnimMontage* attackMontage; // 공격 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat|Attack")
	TArray<FName> attackMontageSections; // 공격 애니메이션 몽타주 섹션 목록


	/* 전투 시스템(피격) */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Hit")
	UAnimMontage* hitReactMontage; // 피격 애니메이션 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Hit")
	UAnimMontage* knockdownMontage; // 강한 경직 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	USoundBase* hitSound; // 피격 사운드

	UPROPERTY(EditAnywhere, Category = "Combat|Hit")
	UParticleSystem* hitParticles; // 피격 VFX

	float fallStartZ = 0.f; // 추락 시작 높이
	bool bIsTrackingFall = false; // 추락 추적을 확인하는 변수
	float pendingHitImpact = 0.f; // 이번 공격의 충격량
	bool bForceStaggerOnHit = false; // 공격의 경직 처리 여부
	float pendingHitWindup = -1.f; // 이번 공격의 예고 길이
	bool bPendingHitCanStaggerStrong = true; // 이번 공격의 강한 경직 허용 여부
	EStaggerResult lastStaggerResult{}; // 마지막 피격의 경직 결정 결과


	/* 전투 시스템(무적) */
	bool bInvincible = false; // 현재 무적 상태 여부


	/* 전투 시스템(사망) */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Death")
	UAnimMontage* deathMontage; // 죽음 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "Combat|Death")
	TArray<FName> deathMontageSections; // 죽음 애니메이션 몽타주 섹션 목록


	/* 전투 시스템(구르기) */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* dodgeMontage; // 구르기 애니메이션 몽타주


	/* 애니메이션 관련 헬퍼 함수 */
	void PlayMontageSection(UAnimMontage* montage, const FName& sectionName); // 애니메이션 몽타주 섹션을 재생하는 함수
	int32 PlayRandomMontageSection(UAnimMontage* montage, const TArray<FName>& sectionNames); // 무작위 애니메이션 몽타주 섹션을 재생하는 함수

public:
	/* 변수의 Getter 함수 */
	FORCEINLINE bool IsInvincible() const { return bInvincible; }
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return deathPose; }
	FORCEINLINE UAttributeComponent* GetAttributes() const { return attributes; }
	FORCEINLINE UBodyPartComponent* GetBodyParts() const { return bodyParts; }
	FORCEINLINE float GetPendingHitImpact() const { return pendingHitImpact; }
	FORCEINLINE UAnimMontage* GetHitReactMontage() const { return hitReactMontage; }
	FORCEINLINE UAnimMontage* GetKnockdownMontage() const { return knockdownMontage; }
	FORCEINLINE EStaggerResult GetLastStaggerResult() const { return lastStaggerResult; }


	/* 변수의 Setter 함수 */
	FORCEINLINE void SetPendingHitImpact(float impact) { pendingHitImpact = impact; } 
	FORCEINLINE void SetForceStaggerOnHit(bool bForce) { bForceStaggerOnHit = bForce; }
};
