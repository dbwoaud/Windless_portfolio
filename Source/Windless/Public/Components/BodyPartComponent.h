#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Characters/BodyPartTypes.h"
#include "BodyPartComponent.generated.h"

class UAttributeComponent;
class USkeletalMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnBodyPartChanged, EBodyPart, part, float, healthPercent, bool, bInjured, bool, bDisabled);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WINDLESS_API UBodyPartComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBodyPartComponent();

	/* 피격 처리 */
	void ApplyHitToBone(FName boneName, float damage); // 뼈 이름으로 부위를 판정하고 대미지를 적용하는 함수
	void ApplyDamageToPart(EBodyPart part, float damage); // 특정 부위에 직접 데미지를 적용하는 함수
	void ApplyHitAtLocation(const USkeletalMeshComponent* mesh, const FVector& impactPoint, float damage); // 본과 가장 가까운 부위에 대미지를 적용하는 함수
	EBodyPart GetClosestPart(const USkeletalMeshComponent* mesh, const FVector& impactPoint) const; // 가장 가까운 본의 부위를 반환하는 함수
	FName GetClosestBone(const USkeletalMeshComponent* mesh, const FVector& impactPoint) const; // 가장 가까운 본의 이름 반환하는 함수
	void CalculateIncomingDamage(const USkeletalMeshComponent* mesh, const FVector& impactPoint, float damage, float baseInjuryChance); // 5단계의 최종 대미지를 계산하는 함수
	void DisableAllParts(); // 캐릭터 사망 시 모든 부위를 파괴하고 체력을 0으로 만드는 함수
	void TriggerInjury(EBodyPart part, EInjuryType injury); // 부위에 부상을 발동시키는 함수
	void TriggerBleeding(EBodyPart part); // 부위에 출혈 발동을 하고 출혈 타이머를 시작하는 함수


	/* 상태 확인 */
	bool IsPartDisabled(EBodyPart part) const; // 해당 부위의 HP가 0인지 확인하는 함수
	bool IsPartInjured(EBodyPart part, EInjuryType injury) const; // 특정 부위에 특정 부상이 활성인지 확인하는 함수
	bool IsDeadByParts() const; // 캐릭터가 사망 상태인지 확인하는 함수
	bool HasInjury(EInjuryType injury) const; // 부상이 발생했는지 확인하는 함수
	bool HasEyeDamage() const; // 안구 손상이 발생했는지 확인하는 함수
	bool HasArmFractureSide(EBodyPart arm) const; // 특정 팔에 골절이 발생했는지 확인하는 함수
	bool HasLegFractureSide(EBodyPart legPart) const; // 특정 다리에 골절이 발생했는지 확인하는 함수
	bool CanSprint() const; //  한 다리 골절 상태인지 확인하는 함수
	bool IsCrawling() const; // 양 다리 골절 상태인지 확인하는 함수
	bool IsBleeding() const; // 과다출혈 진행 중인지 확인하는 함수
	UFUNCTION(BlueprintPure, Category = "BodyPart")
	bool IsPartInjuredAny(EBodyPart part) const; // 해당 부위에 활성 부상이 하나라도 있는지 확인하는 함수


	/* 부상 2차 효과 함수 */
	float GetLockOnRangeMultiplier() const; // 안구 손상 발생 시 락온 거리 배율을 반환하는 함수

	float GetStaminaRegenMultiplier() const; // 가슴에 호흡곤란 발생 시 스태미나 회복를 느리게 하는 함수
	float GetActionDelayPenalty() const; // 가슴에 호흡곤란 발생 시 액션 간 추가 딜레이를 반환하는 함수

	float GetAttackRateMultiplier() const; // 일반 공격 속도를 반환 하는 함수
	float GetStrongAttackTimeMultiplier() const; // 강공격 속도를 반환 하는 함수
	void OnOwnerDealtHit(EBodyPart attackingArm); // 팔에 골절 발생 시 공격 시마다 플레이어 생명력을 감소시키는 함수

	float GetDodgeDistanceMultiplier() const; // 구르기 이동 거리 배율을 반환하는 함수
	float GetMoveHpCost() const; // 이동/구르기 중 초당 자해 HP를 반환하는 함수

	void ApplyFlatDamage(EBodyPart part, float damage); // 부상 판정 없이 부위에 직접 대미지를 적용시키는 함수(자해/출혈)


	/* 회복 */
	EBodyPart GetLowestHealthPart() const; // 파괴되지 않은 부위 중 체력 비율이 가장 낮은 부위를 반환하는 함수
	bool HealPart(EBodyPart part, float amount); // 특정 부위 HP를 회복하는 함수
	void HealAllParts(); // 세이브포인트 도달 시 전체 부위와 부상을 회복하는 함수
	void RegenHealth(float deltaTime); // 매 프레임 부위 생명력을 자연 회복하는 함수
	void CalculateFinalHealthRegenRate(); // 부위별 최종 생명력 변동 속도를 계산하는 함수
	bool CurePartInjury(EBodyPart part, EInjuryType injury); // 특정 부위의 특정 부상을 치유하는 함수
	void SetSaveZoneActive(bool bActive); // 세이브 포인트 구역 진입/이탈 시 회복 배율을 토글하는 함수
	bool RevivePart(EBodyPart part, float healthRatio); // 파괴된 부위를 부활시키는 함수


	/* 아이템 효과 */
	UFUNCTION(BlueprintCallable, Category = "BodyPart|Item")
	void SetEquipmentMaxHealthBonus(EBodyPart part, float bonus); // 부위 최대 체력 장비 보정을 설정하는 함수


	/* 탈피(레벨업) */
	UFUNCTION(BlueprintCallable, Category = "BodyPart|Level")
	void ApplyLevelToParts(int32 level); // 레벨에 맞는 부위 최대 체력을 적용하는 함수


	/* 디버그 출력용 */
	void PrintPartsDebug() const; // 부위별 체력 현황을 출력하는 함수(디버그용)


	/* 델리게이트 */
	void BroadcastAllParts(); // 전 부위 상태를 broadcast하는 함수
	UPROPERTY(BlueprintAssignable, Category = "BodyPart")
	FOnBodyPartChanged onBodyPartChanged; // 부위 HP/부상/파괴 변경 시 broadcast하는 델리게이트


	/* 변수 Getter 함수 */
	float GetTotalHealth() const; // 전 부위 현재 HP 합을 반환하는 함수
	float GetTotalMaxHealth() const; // 전 부위 최대 HP 합을 반환하는 함수
	float GetTotalHealthPercent() const; // 전 부위 현재 HP 비율을 퍼센트 단위로 반환하는 함수
	UFUNCTION(BlueprintPure, Category = "BodyPart")
	float GetPartHealthPercent(EBodyPart part) const; // 특정 부위의 체력 비율을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "BodyPart")
	float GetPartHealth(EBodyPart part) const; // 특정 부위의 현재 체력을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "BodyPart")
	float GetPartMaxHealth(EBodyPart part) const; // 특정 부위의 최대 체력을 반환하는 함수

protected:
	virtual void BeginPlay() override;

private:
	/* 부위 시스템 변수 */
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "BodyPart")
	TMap<EBodyPart, FBodyPartState> bodyParts; // 부위별 상태 맵

	UPROPERTY(EditAnywhere, Category = "BodyPart")
	UDataTable* bodyPartData = nullptr; // 모든 부위의 데이터 테이블 

	UPROPERTY(EditAnywhere, Category = "BodyPart")
	TMap<FName, EBodyPart> boneToPartMap; // 뼈 이름과 신체 부위를 연결하는 맵

	UPROPERTY(EditAnywhere, Category = "BodyPart|Injury")
	UDataTable* injuryDataTable = nullptr; // 부상별 2차 효과 데이터 테이블

	UPROPERTY(Transient)
	TMap<EInjuryType, FInjuryData> injuryCache; // 부상별 2차 효과를 저장하는 변수

	/* 컴포넌트 */
	UPROPERTY()
	UAttributeComponent* ownerAttributes = nullptr; // 같은 액터의 속성 컴포넌트


	/* 초기화 헬퍼 함수 */
	void InitializeBodyParts(); // bodyParts를 초기화하는 함수
	void InitializeBoneToPartMap(); // boneToPartMap을 초기화하는 함수 
	void InitializeInjuryCache(); // injuryCache을 초기화하는 함수
	

	/* 5단계의 최종 대미지 계산 헬퍼 함수 */
	EBodyPart DrawDisturbedPart() const; // 머리와 파괴된 부위를 제외하고 가중치 룰렛의 부위를 반환하는 함수
	void ApplyFinalDamage(EBodyPart part, float damage, float baseInjuryChance, bool bRollInjury); // 최종 대미지와 부상을 부위에 적용하는 함수


	/* 부위 부상 */
	void TryApplyInjury(FBodyPartState* state, EBodyPart part, float baseInjuryChance); // 부위에 부상 발동을 시도하는 함수
	void ApplyInjuryEffect(const EInjuryType injury); // 부상 발생 시 즉발 효과를 적용하는 함수
	void ApplySingleInjury(TFunctionRef<void(const FInjuryData&)> injuryData) const; // 같은 종류의 부상를 1회만 적용하도록 막는 함수
	void TryTriggerBleeding(EBodyPart part, float partMaxHealth); // 부위에 출혈 발동을 시도하는 함수
	void ApplyBleed(); // 출혈이 발생한 부위에 출혈 효과를 적용하는 함수

	UPROPERTY(EditAnywhere, Category = "BodyPart|Bleeding")
	float bleedDamage = 2.f; // 초당 HP 감소량

	UPROPERTY(EditAnywhere, Category = "BodyPart|Bleeding")
	float bleedInterval = 1.f; // 출혈 데미지 간격(초)

	UPROPERTY(EditAnywhere, Category = "BodyPart|Bleeding")
	float bleedChance = 0.1f; // 출혈 발동 확률

	TSet<EBodyPart> bleedingParts;;// 현재 출혈 중인 부위 집합

	FTimerHandle bleedTimer; // 출혈 타이머


	/* 부위 파괴 */
	void OnPartDisabled(EBodyPart part); // 부위 파괴 시 호출되는 함수
	UPROPERTY(EditAnywhere, Category = "BodyPart|Penalty")
	float chestDisabledStaminaRegenMultiplier = 0.3f; // 가슴 부위 파괴 시 적용되는 스태미나 회복 배율
	

	/* 회복 */
	void ClearBleedingEffect(); // 출혈 효과를 지우는 함수
	void ReviveDestroyedPartsInSaveZone(); // 세이브포인트 구역에서 파괴 부위를 복구하는 함수

	UPROPERTY(EditAnywhere, Category = "BodyPart|Regen")
	float saveZoneRegenRate = 7.5f; // 세이브 포인트 근처 생명력 회복 속도

	UPROPERTY(EditAnywhere, Category = "BodyPart|Regen", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float saveZoneReviveHealthRatio = 0.1f; // 구역에서 파괴 부위 복구 시 체력 비율

	bool bSaveZoneActive = false; // 세이브 포인트 구역 내 여부


	/* 델리게이트 */
	void BroadcastPart(EBodyPart part); // 한 부위 상태를 broadcast하는 함수

public:
	/* 변수 Getter 함수 */
	FORCEINLINE bool IsInSaveZone() const { return bSaveZoneActive; }

};
