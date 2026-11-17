#include "Components/AttributeComponent.h"
#include "TimerManager.h"
#include "Windless/DebugMacro.h"
#include "Characters/LevelUpTypes.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAttributeComponent::InitializeHealth(float newMaxHealth)
{
	if (newMaxHealth <= 0.f)
		return;

	maxHealth = newMaxHealth;
	health = newMaxHealth;
}

void UAttributeComponent::ReceiveDamage(float damage)
{
	if (damage <= 0.f)
		return;
	health = FMath::Clamp(health - damage, 0.f, maxHealth);
}

void UAttributeComponent::Heal(float amount)
{
	if (amount <= 0.f)
		return;

	health = FMath::Clamp(health + amount, 0.f, maxHealth); // 합연산
}

float UAttributeComponent::GetHealthPercent()
{
	return health / maxHealth;
}

bool UAttributeComponent::IsAlive()
{
	return health > 0.f;
}

void UAttributeComponent::UseStamina(float staminaCost)
{
	stamina = FMath::Clamp(stamina - staminaCost, 0.f, maxStamina);
	timeSinceStaminaUse = 0.f;
	bIsRegen = false;
}

void UAttributeComponent::RegenStamina(float deltaTime, float regenMultiplier)
{
	timeSinceStaminaUse += deltaTime;

	const float delay = GetStaminaRegenDelay();
	if (timeSinceStaminaUse < delay)
		return;

	if (!bIsRegen)
	{
		bIsRegen = true;
		UE_LOG(LogWindless, Log, TEXT("[스태미나] 회복중 | 혼돈상태=%d 적용딜레이=%.2f초"), (int32)chaosState, delay);
	}

	const float usableMax = GetUsableMaxStamina(); 
	stamina = FMath::Clamp(stamina + finalStaminaRegenRate * regenMultiplier * deltaTime, 0.f, usableMax);
}

float UAttributeComponent::GetStaminaPercent()
{
	return stamina / maxStamina;
}

void UAttributeComponent::CalculateFinalStaminaRegenRate()
{
	finalStaminaRegenRate = (baseStaminaRegenRate + flatStaminaRegenAdd) * (1.f + multStaminaRegenAdd);
}

void UAttributeComponent::TickChaos(float deltaTime, EChaosZone zone, float zoneLimit)
{
	CalculateFinalChaosRegenRate(zone);
	const float delta = ResistChaosGain(finalChaosRegenRate * deltaTime);
	float nextChaos = chaos + delta;
	if (zone == EChaosZone::Light && delta < 0.f) // 빛은 하한까지만 내린다 (이미 하한 아래면 더 내리지 않음)
		nextChaos = FMath::Max(nextChaos, FMath::Min(chaos, zoneLimit));
	else if (zone == EChaosZone::Dark && delta > 0.f) // 어둠은 상한까지만 올린다 (이미 상한 위면 더 올리지 않음)
		nextChaos = FMath::Min(nextChaos, FMath::Max(chaos, zoneLimit));
	chaos = FMath::Clamp(nextChaos, minChaos, maxChaos);
	UpdateChaosState();
}

void UAttributeComponent::AddChaos(float delta)
{
	const float applied = (delta > 0.f) ? ResistChaosGain(delta * chaosGainRate) : delta;
	chaos = FMath::Clamp(chaos + applied, minChaos, maxChaos);
	UpdateChaosState();
}

void UAttributeComponent::ApplyDamageChaos(float dealtDamage, float partMaxHealth)
{
	if (partMaxHealth <= 0.f)
		return;

	const float gain = (dealtDamage / partMaxHealth) * 100.f * damageChaosWeight;
	UE_LOG(LogWindless, Log, TEXT("[혼돈+] 피격 누적 +%.2f (정신력 %.0f 저항 %.0f%% -> +%.2f) (차감 %.1f / 부위최대 %.1f), 현재 %.1f"),
		gain, baseWillPower, GetWillPowerResistRatio() * 100.f, ResistChaosGain(gain * chaosGainRate), dealtDamage, partMaxHealth, chaos);
	AddChaos(gain);
}

void UAttributeComponent::OnPerfectDodge()
{
	AddChaos(-perfectDodgeChaos);
}

void UAttributeComponent::OnMentalAttackHit()
{
	AddChaos(mentalAttackChaos);
}

void UAttributeComponent::ResetChaos()
{
	chaos = 0.f;
	UpdateChaosState();
}

void UAttributeComponent::LoseChaos(float amount)
{
	AddChaos(amount);
}

void UAttributeComponent::RecoverChaos(float amount)
{
	AddChaos(-amount);
}

float UAttributeComponent::GetChaosPercent()
{
	const float range = maxChaos - minChaos;
	return range > 0.f ? (chaos - minChaos) / range : 0.5f;
}

bool UAttributeComponent::IsChaosEmpty()
{
	return FMath::IsNearlyZero(chaos);
}

void UAttributeComponent::CalculateFinalChaosRegenRate(EChaosZone zone)
{
	// 그늘(칠하지 않은 곳)은 0: 혼돈이 오르지도 내리지도 않는다
	const float darkRate = darkZoneChaosGainRate * (bLanternOn ? lanternDarkGainScale : 1.f); // 등불은 어둠의 증가만 줄인다
	const float zoneRate = zone == EChaosZone::Light ? -lightZoneChaosLossRate : zone == EChaosZone::Dark ? darkRate : 0.f;
	finalChaosRegenRate = (baseChaosRegenRate + flatChaosRegenAdd + zoneRate) * (1.f + multChaosRegenAdd);
}

float UAttributeComponent::GetWillPowerResistRatio() const
{
	const float willPower = FMath::Max(baseWillPower, 0.f);
	return willPower / (100.f + willPower); // 증가량 x 100 / (100 + 정신력)과 같다
}

float UAttributeComponent::ResistChaosGain(float gain) const
{
	return (gain > 0.f) ? gain * (1.f - GetWillPowerResistRatio()) : gain;
}

void UAttributeComponent::CalculateFinalAttackPower()
{
	finalAttackPower = (baseAttackPower + flatAttackAdd + equipmentAttackAdd) * (1.f + multAttackAdd);
}

void UAttributeComponent::SetEquipmentAttackBonus(float value)
{
	equipmentAttackAdd = value;
	CalculateFinalAttackPower();
}

float UAttributeComponent::GetAttackPowerWithWeaponBonus(float weaponBonus) const
{
	return (baseAttackPower + flatAttackAdd + equipmentAttackAdd + weaponBonus) * (1.f + multAttackAdd);
}

void UAttributeComponent::SetBodyHeat(float value)
{
	bodyHeat = value;
}

void UAttributeComponent::ApplyTimedBodyHeat(float amount, float duration)
{
	UWorld* world = GetWorld();
	if (!world)
		return;

	if (world->GetTimerManager().IsTimerActive(bodyHeatTimer))
	{
		world->GetTimerManager().ClearTimer(bodyHeatTimer);
		bodyHeat -= activeBodyHeatBonus;
	}

	activeBodyHeatBonus = amount;
	bodyHeat += amount;

	if (duration > 0.f)
		world->GetTimerManager().SetTimer(bodyHeatTimer, this, &UAttributeComponent::RevertBodyHeat, duration, false);

	UE_LOG(LogWindless, Log, TEXT("[체온] +%.0f (%.0f초) -> %.1f"), amount, duration, bodyHeat);
}

void UAttributeComponent::AddMoltStones(int32 amount)
{
	if (amount <= 0)
		return;

	moltStones += amount;
	UE_LOG(LogWindless, Log, TEXT("[탈피석] +%d -> 보유 %d (다음 탈피 %d)"), amount, moltStones, moltCost);
	onMoltingChanged.Broadcast(currentLevel, GetMoltingExpPercent());
}

bool UAttributeComponent::SpendMoltStones(int32 amount)
{
	if (amount <= 0 || moltStones < amount)
		return false;

	moltStones -= amount;
	UE_LOG(LogWindless, Log, TEXT("[탈피석] -%d -> 보유 %d"), amount, moltStones);
	onMoltingChanged.Broadcast(currentLevel, GetMoltingExpPercent());
	return true;
}

bool UAttributeComponent::CanMolt() const
{
	return moltStones >= moltCost;
}

bool UAttributeComponent::Molt()
{
	if (!SpendMoltStones(moltCost))
		return false;

	++currentLevel;
	moltCost += moltCostIncrement;

	UE_LOG(LogWindless, Log, TEXT("[탈피] 허물 -> %d (다음 비용 %d, 보유 %d)"), currentLevel, moltCost, moltStones);
	onMoltingChanged.Broadcast(currentLevel, GetMoltingExpPercent());
	return true;
}

float UAttributeComponent::GetMoltingExpPercent() const
{
	return moltCost > 0 ? FMath::Clamp((float)moltStones / (float)moltCost, 0.f, 1.f) : 0.f;
}

void UAttributeComponent::ApplyLevelUpStats()
{
	maxStamina += WindlessLevelUp::StaminaPerLevel;
	stamina = maxStamina;

	baseAttackPower += WindlessLevelUp::AttackPowerPerLevel;
	baseWillPower += WindlessLevelUp::WillPowerPerLevel;

	UE_LOG(LogWindless, Log, TEXT("[탈피] 스탯 증가 - 스태미나 %.0f / 정신력 %.0f"), maxStamina, baseWillPower);
}

void UAttributeComponent::CaptureLv1Stats()
{
	if (bLv1StatsCaptured)
		return;

	lv1MaxStamina = maxStamina;
	lv1BaseAttackPower = baseAttackPower;
	lv1BaseWillPower = baseWillPower;
	bLv1StatsCaptured = true;
}

void UAttributeComponent::ApplyStatsForLevel(int32 level)
{
	CaptureLv1Stats();
	const float steps = static_cast<float>(FMath::Max(0, level - 1));
	const float previousMaxStamina = maxStamina;
	
	maxStamina = lv1MaxStamina + WindlessLevelUp::StaminaPerLevel * steps;
	stamina = FMath::Clamp(stamina + (maxStamina - previousMaxStamina), 0.f, maxStamina);
	baseAttackPower = lv1BaseAttackPower + WindlessLevelUp::AttackPowerPerLevel * steps;
	baseWillPower = lv1BaseWillPower + WindlessLevelUp::WillPowerPerLevel * steps;
	CalculateFinalAttackPower();

	UE_LOG(LogWindless, Log, TEXT("[탈피] Lv%d 스탯 - 스태미나 %.0f / 공격력 %.0f / 정신력 %.0f"),
		level, maxStamina, baseAttackPower, baseWillPower);
}

void UAttributeComponent::SaveToRecord(FPlayerStatSaveRecord& outRecord) const
{
	outRecord.currentLevel = currentLevel;
	outRecord.moltStones = moltStones;
	outRecord.moltCost = moltCost;
	outRecord.bodyHeat = bodyHeat;
}

void UAttributeComponent::LoadFromRecord(const FPlayerStatSaveRecord& record)
{
	currentLevel = FMath::Max(1, record.currentLevel);
	moltStones = FMath::Max(0, record.moltStones);
	moltCost = (record.moltCost > 0) ? record.moltCost : 200;
	bodyHeat = record.bodyHeat;

	ResetChaos();
	stamina = maxStamina;
	onMoltingChanged.Broadcast(currentLevel, GetMoltingExpPercent());
}

float UAttributeComponent::GetStaminaLockRatio() const
{
	return GetStaminaLockRatio_Internal(chaosState);
}

float UAttributeComponent::GetStaminaLockRatio_Internal(EChaosState state) const
{
	if (const FChaosStateData* d = chaosStateCache.Find(state))
		return d->staminaLockRatio;

	return 0.f;
}

float UAttributeComponent::GetUsableMaxStamina() const
{
	return maxStamina * (1.f - GetStaminaLockRatio());
}

float UAttributeComponent::GetStaminaRegenDelay() const
{
	const FChaosStateData& d = GetCurrentChaosData();
	return (d.staminaRegenDelayOverride >= 0.f) ? d.staminaRegenDelayOverride : staminaRegenDelay;
}

float UAttributeComponent::GetCastSpeedMultiplier() const
{
	return GetCurrentChaosData().castSpeedMultiplier;
}

float UAttributeComponent::GetInjuryChanceReduction() const
{
	return GetCurrentChaosData().injuryChanceReduction;
}

float UAttributeComponent::GetWeaknessDamageMultiplier() const
{
	return GetCurrentChaosData().weaknessDamageMultiplier;
}

float UAttributeComponent::GetLockOnBreakInterval() const
{
	return GetCurrentChaosData().lockOnBreakInterval;
}

float UAttributeComponent::GetLockOnBreakChance() const
{
	return GetCurrentChaosData().lockOnBreakChance;
}

bool UAttributeComponent::IsLockOnInputBlocked() const
{
	return GetCurrentChaosData().bLockOnInputBlocked;
}

float UAttributeComponent::GetLockOnShakeInterval() const
{
	return GetCurrentChaosData().lockOnShakeInterval;
}

void UAttributeComponent::DebugSetChaos(float value)
{
	chaos = FMath::Clamp(value, minChaos, maxChaos);
	UpdateChaosState();
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	CaptureLv1Stats();
	InitializeChaosStateCache();
	CalculateFinalStaminaRegenRate();
	CalculateFinalChaosRegenRate();
	CalculateFinalAttackPower();
}

EChaosState UAttributeComponent::ComputeState(float value) const
{
	if (value >= 81.f) 
		return EChaosState::ECS_Collapse3;
	if (value >= 51.f) 
		return EChaosState::ECS_Collapse2;
	if (value >= 21.f) 
		return EChaosState::ECS_Collapse1;
	if (value <= -81.f) 
		return EChaosState::ECS_Awakening3;
	if (value <= -51.f) 
		return EChaosState::ECS_Awakening2;
	if (value <= -21.f) 
		return EChaosState::ECS_Awakening1;

	return EChaosState::ECS_Neutral;
}

void UAttributeComponent::UpdateChaosState()
{
	const EChaosState newState = ComputeState(chaos);
	staminaLockRatio = GetStaminaLockRatio_Internal(newState);
	const float usableMax = maxStamina * (1.f - staminaLockRatio);
	if (stamina > usableMax)
		stamina = usableMax;

	if (newState != chaosState)
	{
		chaosState = newState;
		onChaosStateChanged.Broadcast(chaosState);
	}
}

void UAttributeComponent::InitializeChaosStateCache()
{
	chaosStateCache.Empty();
	if (!chaosStateTable)
		return;

	const UEnum* enumPtr = StaticEnum<EChaosState>();
	const int32 numEnum = enumPtr->NumEnums() - 1;

	for (int32 i = 0; i < numEnum; ++i)
	{
		const EChaosState state = static_cast<EChaosState>(enumPtr->GetValueByIndex(i));
		const FName rowName(*enumPtr->GetNameStringByIndex(i));

		if (const FChaosStateData* data = chaosStateTable->FindRow<FChaosStateData>(rowName, TEXT("InitializeChaosStateCache")))
			chaosStateCache.Add(state, *data);
	}
}

const FChaosStateData& UAttributeComponent::GetCurrentChaosData() const
{
	if (const FChaosStateData* data = chaosStateCache.Find(chaosState))
		return *data;

	static const FChaosStateData defaultState;
	return defaultState;
}

void UAttributeComponent::RevertBodyHeat()
{
	bodyHeat -= activeBodyHeatBonus;
	activeBodyHeatBonus = 0.f;
	UE_LOG(LogWindless, Log, TEXT("[체온] 효과 종료 -> %.1f"), bodyHeat);
}