#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ChaosStateData.h" 
#include "Save/WindlessSaveTypes.h" 
#include "AttributeComponent.generated.h"

UENUM(BlueprintType)
enum class EChaosState : uint8
{
	ECS_Awakening3 UMETA(DisplayName = "Awakening3"), // -100 ~ -81 심안의 경지
	ECS_Awakening2 UMETA(DisplayName = "Awakening2"), // -80  ~ -51 통찰의 눈
	ECS_Awakening1 UMETA(DisplayName = "Awakening1"), // -50  ~ -21 직관의 개화
	ECS_Neutral    UMETA(DisplayName = "Neutral"),    // -20  ~ +20 이성 유지
	ECS_Collapse1  UMETA(DisplayName = "Collapse1"),  // +21  ~ +50 인지의 균열
	ECS_Collapse2  UMETA(DisplayName = "Collapse2"),  // +51  ~ +80 지각의 파편화
	ECS_Collapse3  UMETA(DisplayName = "Collapse3")   // +81  ~ +100 내면의 암흑
};

UENUM(BlueprintType)
enum class EChaosZone : uint8
{
	Shade UMETA(Hidden), // 칠하지 않은 곳: 혼돈 변화 없음
	Light, // 빛 구역: 하한까지만 내린다
	Dark   // 어둠 구역: 상한까지만 올린다
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChaosStateChanged, EChaosState, newState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoltingChanged, int32, newLevel, float, expPercent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WINDLESS_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	/* 생명력 */
	void InitializeHealth(float newMaxHealth); // 생명력을 초기화하는 함수
	void ReceiveDamage(float damage); // 대미지를 입어 생명력을 소모하는 함수
	void Heal(float amount); // 생명력을 회복하는 함수
	float GetHealthPercent(); // 퍼센트 단위로 현재 생명력을 반환하는 함수
	bool IsAlive(); // 캐릭터의 사망 판정을 확인하는 함수


	/* 스태미나 */
	void UseStamina(float staminaCost); // 스태미나를 소모하는 함수
	void RegenStamina(float deltaTime, float regenMultiplier); // 스태미나를 회복하는 함수
	float GetStaminaPercent(); // 퍼센트 단위로 현재 스태미나를 반환하는 함수
	void CalculateFinalStaminaRegenRate(); // 최종 스태미나 재생 속도를 계산하는 함수


	/* 혼돈 게이지 */
	void TickChaos(float deltaTime, EChaosZone zone, float zoneLimit); // 프레임마다 혼돈을 계산하는 함수
	void AddChaos(float delta); // 혼돈을 빼거나 더하는 함수
	void ApplyDamageChaos(float dealtDamage, float partMaxHealth); // 피격 대미지 비례 혼돈을 적용하는 함수
	void OnPerfectDodge(); // 완벽한 구르기 성공 시 호출되는 함수
	void OnMentalAttackHit(); // 정신 오염공격 피격 시 호출되는 함수
	void ResetChaos(); // 혼돈을 0으로 초기화하는 함수
	void LoseChaos(float amount); // 즉시 혼돈을 감소시키는 함수
	void RecoverChaos(float amount); // 즉시 혼돈을 회복시키는 함수
	float GetChaosPercent(); // 퍼센트 단위로 현재 혼돈을 반환하는 함수
	bool IsChaosEmpty(); // 혼돈이 0인지 확인하는 함수
	void CalculateFinalChaosRegenRate(EChaosZone zone = EChaosZone::Shade); // 최종 혼돈 변동 게이지 속도를 계산하는 함수
	float GetWillPowerResistRatio() const; // 정신력에 따른 혼돈 증가 저항률을 반환하는 함수
	float ResistChaosGain(float gain) const; // 정신력 저항을 적용한 혼돈 증가량을 반환하는 함수


	/* 공격력 */
	void CalculateFinalAttackPower(); // 최종 공격력을 계산하는 함수
	void SetEquipmentAttackBonus(float value); // 장비 공격력 보정 합계를 대입하는 함수
	float GetAttackPowerWithWeaponBonus(float weaponBonus) const; // 특정 무기 보정치를 더한 공격력을 반환하는 함수


	/* 체온 */
	void SetBodyHeat(float value); // 체온을 설정하는 함수
	void ApplyTimedBodyHeat(float amount, float duration); // 일정 시간 체온을 상승시키는 함수


	/* 재화 */
	UFUNCTION(BlueprintCallable, Category = "Attribute|MoltStone")
	void AddMoltStones(int32 amount); // 탈피석을 획득하는 함수

	UFUNCTION(BlueprintCallable, Category = "Attribute|MoltStone")
	bool SpendMoltStones(int32 amount); // 탈피석을 소모하는 함수


	/* 탈피(레벨업) */
	UFUNCTION(BlueprintPure, Category = "Attribute|Molting")
	bool CanMolt() const; // 탈피 가능 여부를 확인하는 함수

	UFUNCTION(BlueprintCallable, Category = "Attribute|Molting")
	bool Molt(); // 탈피석을 소모해 허물을 1 올리는 함수

	UFUNCTION(BlueprintPure, Category = "Attribute|Molting")
	float GetMoltingExpPercent() const; // 다음 탈피까지의 진행도를 반환하는 함수

	void ApplyLevelUpStats(); // 허물에 따른 스태미나, 공격력, 정신력 증가를 적용하는 함수
	void CaptureLv1Stats(); // Lv1 기준값을 캡처하는 함수
	void ApplyStatsForLevel(int32 level); // 레벨에 맞는 스탯을 계산해 대입하는 함수


	/* 저장/불러오기 */
	void SaveToRecord(FPlayerStatSaveRecord& outRecord) const; // 현재 스테이터스를 저장 레코드로 변환하는 함수
	void LoadFromRecord(const FPlayerStatSaveRecord& record); // 저장 레코드로 스테이터스를 복원하는 함수


	/* 기타 */
	float GetStaminaLockRatio() const; // 현재 혼돈 단계에 따른 스태미나 잠금 비율을 반환하는 함수
	float GetStaminaLockRatio_Internal(EChaosState state) const; // 혼돈 단계의 스태미나 잠금 비율을 반환하는 함수
	float GetUsableMaxStamina() const; // 현재 혼돈 단계에 따른 현재 최대 스태미나를 반환하는 함수
	float GetStaminaRegenDelay() const; // 현재 혼돈 단계에 따른 스태미나 회복 딜레이를 반환하는 함수
	float GetCastSpeedMultiplier() const; // 현재 혼돈 단계에 따른 아이템 시전 속도 비율을 반환하는 함수
	float GetInjuryChanceReduction() const;// 현재 혼돈 단계에 따른 부상 감소 확률을 반환하는 함수
	float GetWeaknessDamageMultiplier() const; // 각성 3단계 시 약점 공격 데미지 비율을 반환하는 함수
	float GetLockOnBreakInterval() const; // 락온 강제 해제 판정 주기를 반환하는 함수
	float GetLockOnBreakChance() const;   // 락온 강제 해제 확률을 반환하는 함수
	bool IsLockOnInputBlocked() const; // 락온 기능을 비활성화 해야하는 혼돈 단계인지 확인하는 함수
	float GetLockOnShakeInterval() const; // 락온 중 카메라 흔들림 주기를 반환하는 함수
	
	
	/* 디버그용 */
	UFUNCTION(BlueprintCallable, Category = "Actor Attributes|Chaos")
	void SetLanternOn(bool bOn) { bLanternOn = bOn; } // 등불을 켜고 끌 때 부르는 함수 (어둠의 증가만 줄인다)

	void DebugSetChaos(float value); // 혼돈 수치를 즉시 지정하는 함수(디버그용)


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Actor Attributes|Chaos")
	FOnChaosStateChanged onChaosStateChanged; // 혼돈 상태 변경 시 실행되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Actor Attributes|Molting")
	FOnMoltingChanged onMoltingChanged; // 허물/탈피 게이지 변경 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;

private:
	/* 생명력 */
	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Health")
	float health; // 현재 체력

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Health")
	float maxHealth; // 최대 체력


	/* 스태미나 */
	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Stamina")
	float stamina; // 현재 스태미나

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Stamina")
	float maxStamina; // 최대 스태미나

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Stamina")
	float dodgeCost = 14.f; // 구르기 사용 시 소모되는 스태미나(횟수 당)

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Stamina")
	float sprintStaminaCost = 10.f; // 달리기 사용 시 소모되는 스태미나(초 당)

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Stamina")
	float baseStaminaRegenRate = 10.f; // 기본 스태미나 재생 속도

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Stamina")
	float flatStaminaRegenAdd = 0.f;  // 합연산

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Stamina")
	float multStaminaRegenAdd = 0.f;  // 곱연산

	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes|Stamina")
	float finalStaminaRegenRate = 0.f; //최종 스태미나 재생 속도

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Stamina")
	float staminaRegenDelay = 0.4f; // 회복을 시작하는 대기시간

	float timeSinceStaminaUse = 999.f; // 마지막 스태미나 소모 후 경과시간
	bool bIsRegen = false; // 현재 스태미나가 회복 중인지 확인하는 변수

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Stamina")
	float exhaustedAttackRate = 0.6f; // 스태미나 소진 시 공격 속도 배율


	/* 혼돈 게이지 */
	EChaosState ComputeState(float value) const; // 혼돈 수치를 단계로 변환하는 함수 
	void UpdateChaosState(); // 혼돈 상태를 업데이트하는 함수
	void InitializeChaosStateCache(); // 혼돈 상태 테이블을 enum 키 맵으로 초기화하는 함수
	const FChaosStateData& GetCurrentChaosData() const; // 현재 혼돈 단계의 데이터를 반환하는 함수

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float chaos = 0.f; // 현재 혼돈 게이지

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float maxChaos = 100.f; // 최대 혼돈 게이지

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float minChaos = -100.f; // 최소 혼돈 게이지

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float baseChaosRegenRate = 0.f; // 기본 혼돈 게이지 변동 속도

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float flatChaosRegenAdd = 0.f; // 합연산

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float multChaosRegenAdd = 0.f; // 곱연산

	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes|Chaos")
	float finalChaosRegenRate = 0.f;  // 최종 혼돈 게이지 변동 속도

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float chaosGainRate = 1.0f; // 혼돈 가중치 배율

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	EChaosState chaosState = EChaosState::ECS_Neutral; // 현재 혼돈 단계

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	UDataTable* chaosStateTable = nullptr; // 혼돈 단계별 수치 테이블

	UPROPERTY(Transient)
	TMap<EChaosState, FChaosStateData> chaosStateCache; // 혼돈 상태 캐시

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float staminaLockRatio = 0.f; // 현재 단계 스태미나 잠금 비율

	/* 혼돈 게이지 (어둠, 빛 구역) */
	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float lanternDarkGainScale = 0.5f; // 등불을 켰을 때 어둠 구역 증가에 곱하는 값

	bool bLanternOn = false; // 등불이 켜져 있는지

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float darkZoneChaosGainRate = 2.0f; // 어둠 구역의 초당 획득 혼돈

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float lightZoneChaosFloor = 0.f; // 하한을 따로 주지 않는 빛(세이브포인트, EnterLightZone)의 하한. 빛마다 다른 하한은 ChaosZoneVolume에 적는다

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos")
	float lightZoneChaosLossRate = 1.5f; // 빛 구역의 초당 감소 혼돈

	/* 혼돈 게이지 (인터럽트 상수) */
	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos|Constant")
	float perfectDodgeChaos = 10.f; // 완벽 구르기 시 감소되는 혼돈량

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos|Constant")
	float mentalAttackChaos = 15.f;   // 정신오염 공격으로 인해 증가되는 혼돈량

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Chaos|Constant")
	float damageChaosWeight = 0.5f;   // 피격 대미지 비례 증가되는 혼돈 가중치


	/* 공격력 */
	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Attack")
	float baseAttackPower = 10.f; // 기본 공격력

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Attack")
	float flatAttackAdd = 0.f; // 합연산

	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Attack")
	float multAttackAdd = 0.f; // 곱연산

	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes|Attack")
	float equipmentAttackAdd = 0.f; // 방어구 보정 전용 합연산

	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes|Attack")
	float finalAttackPower = 0.f; // 최종 공격력


	/* 정신력 */
	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Will")
	float baseWillPower = 0.f; // 기본 정신력

	// 정신력 저항: 혼돈 증가 x 100 / (100 + 정신력). 상한 없음 (사용자 결정 2026-11-17)


	/* 강인도 */
	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Tenacity", meta = (ClampMin = "0.0"))
	float tenacity = 15.f; // 기본 강인함


	/* 체온 */
	void RevertBodyHeat();// 체온 보정을 되돌리는 함수
	UPROPERTY(EditAnywhere, Category = "Actor Attributes|Heat")
	float bodyHeat = 36.f; // 현재 체온
	float activeBodyHeatBonus = 0.f; // 현재 적용중인 체온 보정치
	FTimerHandle bodyHeatTimer; // 체온 지속효과 타이머


	/* 재화 */
	UPROPERTY(EditAnywhere, Category = "Actor Attributes|MoltStone")
	int32 moltStones; // 현재 탈피석

	UPROPERTY(EditAnywhere, Category = "Attribute|MoltStone", meta = (ClampMin = "0"))
	int32 moltStoneReward = 0; // 처치 시 주는 탈피석


	/* 탈피(레벨업) */
	UPROPERTY(VisibleAnywhere, Category = "Attribute|Molting")
	int32 currentLevel = 1; // 현재 레벨

	UPROPERTY(VisibleAnywhere, Category = "Attribute|Molting")
	int32 moltCost = 200; // 다음 탈피에 필요한 탈피석

	UPROPERTY(EditAnywhere, Category = "Attribute|Molting", meta = (ClampMin = "0"))
	int32 moltCostIncrement = 100; // 탈피 당 비용 증가량

	float lv1MaxStamina = 100.f;      // Lv1 최대 스태미나
	float lv1BaseAttackPower = 10.f;  // Lv1 기본 공격력
	float lv1BaseWillPower = 0.f;     // Lv1 기본 정신력
	bool bLv1StatsCaptured = false;   // 스냅샷 캡처 여부

public:
	/* 변수의 Getter 함수 */
	FORCEINLINE float GetStamina() const { return stamina; }
	FORCEINLINE float GetMaxStamina() const { return maxStamina; }
	FORCEINLINE float GetDodgeCost() const { return dodgeCost; }
	FORCEINLINE float GetSprintStaminaCost() const { return sprintStaminaCost; }
	FORCEINLINE float GetChaos() const { return chaos; }
	FORCEINLINE float GetMaxChaos() const { return maxChaos; }
	FORCEINLINE float GetMinChaos() const { return minChaos; }
	FORCEINLINE float GetLightZoneChaosFloor() const { return lightZoneChaosFloor; }
	FORCEINLINE float GetChaosGainRate() const { return chaosGainRate; }
	FORCEINLINE float GetFinalChaosRegenRate() const { return finalChaosRegenRate; }
	FORCEINLINE EChaosState GetChaosState() const { return chaosState; }
	FORCEINLINE float GetBaseAttackPower() const { return baseAttackPower; }
	FORCEINLINE float GetFinalAttackPower() const { return finalAttackPower; }
	FORCEINLINE float GetWillPower() const { return baseWillPower; }
	FORCEINLINE float GetTenacity() const { return tenacity; }
	FORCEINLINE float GetBodyHeat() const { return bodyHeat; }
	UFUNCTION(BlueprintPure, Category = "Attribute|MoltStone")
	FORCEINLINE int32 GetMoltStones() const { return moltStones; }
	UFUNCTION(BlueprintPure, Category = "Attribute|MoltStone")
	FORCEINLINE int32 GetMoltStoneReward() const { return moltStoneReward; }
	UFUNCTION(BlueprintPure, Category = "Attribute|Molting")
	FORCEINLINE int32 GetMoltCost() const { return moltCost; }
	FORCEINLINE	int32 GetCurrentLevel() const { return currentLevel; }
	FORCEINLINE bool IsExhausted() const { return stamina <= 0.f; }
	FORCEINLINE float GetAttackRateMultiplier() const { return IsExhausted() ? exhaustedAttackRate : 1.f; }

};
