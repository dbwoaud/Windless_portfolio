#pragma once

#include "CoreMinimal.h"
#include "Items.h"
#include "WeaponType.h"
#include "Characters/CombatTypes.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;
class ABaseCharacter;
struct FItemData;

UCLASS()
class WINDLESS_API AWeapon : public AItems
{
	GENERATED_BODY()

public:
	AWeapon();

	/* 무기(장착) */
	void Equip(USceneComponent* inParent, FName inSocketName, AActor* newOwner, APawn* newInstigator); // 무기를 장착하는 함수
	void ApplyItemData(const FItemData& data); // 아이템 원본 데이터로 무기 타입/가드수치를 설정하는 함수
	void AttachMeshToSocket(USceneComponent* inParent, const FName& inSocketName); // 소켓에 무기를 붙이는 함수
	void PlayEquipSound(); // 무기 장착 사운드를 재생하는 함수
	void DisableSphereCollision(); // 무기 콜라이더를 비활성화하는 함수
	void DeactivateEmbers(); // 무기 VFX를 비활성화하는 함수


	/* 무기(공격) */
	void PlayGuardSound(); // 가드 성공 사운드를 재생하는 함수
	TArray<AActor*> ignoreActors; // 중복 타격을 방지 목록
	void SetHitOverride(const FWeaponHitOverride& inHitOverride); // 몬스터 데이터 공격으로 판정 값을 덮어쓰는 함수
	void ClearHitOverride(); // 판정값 덮어쓰기를 해제하는 함수

protected:
	virtual void BeginPlay() override;
	// 다른 액터와 overlap 중일 때 실행되는 함수 
	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	

	/* 상태 확인 */
	bool ActorIsSameType(AActor* OtherActor); // 무기 소유 액터가 아군인지 확인하는 함수


	/* 무기(공격) */
	UFUNCTION(BlueprintImplementableEvent) 
	void CreateFields(const FVector& fieldLocation); // 무기 공격 시 필드를 생성하는 함수


	/* 무기(대미지) */
	float GetOwnerAttackPower() const; // 소유 캐릭터의 최종 공격력을 반환하는 함수


	/* 무기(피격) */
	void ExecuteGetHit(FHitResult& boxHit); // 무기 피격 시 호출되는 함수

private:
	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties|Trace")
	UBoxComponent* weaponBox; // 박스 콜라이더 컴포넌트


	/* 무기 정보 */
	FName sourceItemId = NAME_None; // 무기 아이템 ID


	/* 무기(장착) */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties|Socket")
	FName handSocketOverride = NAME_None; // 손 소켓

	UPROPERTY(EditAnywhere, Category = "Weapon Properties|Sound")
	USoundBase* equipSound; // 장착 효과음


	/* 무기(공격) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties|Attack", meta = (AllowPrivateAccess = "true"))
	EWeaponType weaponType = EWeaponType::EWT_OneHanded; // 무기 종류

	EAttackType currentAttackType = EAttackType::EAT_Light; // 현재 공격 타입 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties|Attack", meta = (AllowPrivateAccess = "true"))
	FAttackSpec lightAttack; // 약공격 설정값

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties|Attack", meta = (AllowPrivateAccess = "true"))
	FAttackSpec heavyAttack; // 강공격 설정값

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties|Attack", meta = (AllowPrivateAccess = "true"))
	FAttackSpec jumpAttack; // 점프 공격 설정값

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties|Attack", meta = (AllowPrivateAccess = "true", ClampMin = "-1.0", ClampMax = "1.0"))
	float injuryChance = -1.f; // 공격 기본 부상 확률

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties|Guard", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", ClampMax = "180.0"))
	float guardAngle = 70.f; // 가드 반각

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties|Guard", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float guardValue = 0.f; // 가드 수치

	UPROPERTY(EditAnywhere, Category = "Weapon Properties|Sound")
	USoundBase* guardSound; // 가드 효과음


	/* 무기(대미지) */
	float CalculateCurrentDamage() const; // 현재 공격 종류의 최종 대미지를 계산하는 함수
	FCombatHitInfo BuildHitInfo(const FHitResult& boxHit); // 타격 판정 정보를 구성하는 함수
	TOptional<FWeaponHitOverride> hitOverride; // 판정값 덮어쓰기


	/* 무기(피격) */
	bool ApplyHitToTarget(const FHitResult& boxHit); // 대상에게 피격을 적용하는 함수
	bool ApplyHitToCharacter(ABaseCharacter* hitCharacter, const FHitResult& boxHit); // 캐릭터에게 피격을 적용하는 함수


	/* 무기(추적) */
	void BoxTrace(FHitResult& boxhit); // 박스 추적으로 타격 지점을 계산하는 함수

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties|Trace") // 박스 추적의 시작 지점
	USceneComponent* boxTraceStart;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties|Trace") // 박스 추적의 끝 지점
	USceneComponent* boxTraceEnd;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties|Trace") // 추적 박스의 크기
	FVector boxTraceExtent = FVector(5.f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties|Trace") // 시각화 변수(디버그용)
	bool bShowBoxDebug = false;

public:
	/* 변수의 Getter 함수 */
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return weaponBox; }
	FORCEINLINE void SetCurrentAttackType(EAttackType attackType) { currentAttackType = attackType; }
	FORCEINLINE EAttackType GetCurrentAttackType() const { return currentAttackType; }
	FORCEINLINE const FAttackSpec& GetAttackSpec(EAttackType attackType) const 
	{
		switch (attackType)
		{
			case EAttackType::EAT_Heavy: return heavyAttack;
			case EAttackType::EAT_Jump:  return jumpAttack;
			default:                     return lightAttack;
		}
	}
	FORCEINLINE EWeaponType GetWeaponType() const { return weaponType; }
	FORCEINLINE FName GetSourceItemId() const { return sourceItemId; }
	FORCEINLINE FName GetHandSocketOverride() const { return handSocketOverride; }
	FORCEINLINE bool IsShield() const { return weaponType == EWeaponType::EWT_Shield; }
	FORCEINLINE bool IsTwoHanded() const { return weaponType == EWeaponType::EWT_TwoHanded; }
	FORCEINLINE bool IsBow() const { return weaponType == EWeaponType::EWT_Bow; }
	FORCEINLINE bool OccupiesBothHands() const { return IsTwoHanded() || IsBow(); }
	FORCEINLINE float GetGuardAngle() const { return guardAngle; }
	FORCEINLINE float GetGuardValue() const { return guardValue; }
	FORCEINLINE float GetInjuryChance() const { return injuryChance; }
};
