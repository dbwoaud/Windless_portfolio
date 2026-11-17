#include "Components/BodyPartComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Windless/DebugMacro.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Characters/LevelUpTypes.h"

UBodyPartComponent::UBodyPartComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    InitializeBoneToPartMap();
}

void UBodyPartComponent::ApplyHitToBone(FName boneName, float damage)
{
    const EBodyPart* foundPart = boneToPartMap.Find(boneName);
    if (!foundPart) // 뼈 이름에 매핑된 부위가 없으면 부위 데미지 없이 무시
        return;
    
    ApplyDamageToPart(*foundPart, damage);
}

void UBodyPartComponent::ApplyDamageToPart(EBodyPart part, float damage)
{
    ApplyFinalDamage(part, damage, -1.f, true);
}

void UBodyPartComponent::ApplyHitAtLocation(const USkeletalMeshComponent* mesh, const FVector& impactPoint, float damage)
{
    CalculateIncomingDamage(mesh, impactPoint, damage, -1.f);
}

EBodyPart UBodyPartComponent::GetClosestPart(const USkeletalMeshComponent* mesh, const FVector& impactPoint) const
{
    EBodyPart best = EBodyPart::EBP_Chest;
    if (!mesh)
        return best;

    float bestDistSq = TNumericLimits<float>::Max();
    for (const TPair<FName, EBodyPart>& pair : boneToPartMap)
    {
        if (mesh->GetBoneIndex(pair.Key) == INDEX_NONE) // 스켈레톤에 없는 본 무시
            continue;

        if (IsPartDisabled(pair.Value))
            continue;

        const float d = FVector::DistSquared(mesh->GetBoneLocation(pair.Key), impactPoint);
        if (d < bestDistSq)
        {
            bestDistSq = d;
            best = pair.Value;
        }
    }
    return best;
}

FName UBodyPartComponent::GetClosestBone(const USkeletalMeshComponent* mesh, const FVector& impactPoint) const
{
    FName bestBone = NAME_None;
    if (!mesh)
        return bestBone;

    float bestDistSq = TNumericLimits<float>::Max();
    for (const TPair<FName, EBodyPart>& pair : boneToPartMap)
    {
        if (mesh->GetBoneIndex(pair.Key) == INDEX_NONE)
            continue;
        if (IsPartDisabled(pair.Value))
            continue;

        const float d = FVector::DistSquared(mesh->GetBoneLocation(pair.Key), impactPoint);
        if (d < bestDistSq)
        {
            bestDistSq = d;
            bestBone = pair.Key;
        }
    }
    return bestBone;
}

void UBodyPartComponent::CalculateIncomingDamage(const USkeletalMeshComponent* mesh, const FVector& impactPoint, float damage, float baseInjuryChance)
{
    if (damage <= 0.f)
        return;

    // [1단계] 원래 피격 부위 확정
    EBodyPart finalPart = GetClosestPart(mesh, impactPoint);

    // [2단계] 혼돈 게이지 체크
    const float chaos = ownerAttributes ? ownerAttributes->GetChaos() : 0.f;
    const bool bSensoryDisruption = ownerAttributes && ownerAttributes->GetChaosState() >= EChaosState::ECS_Collapse2;

    if (bSensoryDisruption)
    {
        if (FMath::FRandRange(0.f, 100.f) <= chaos) // 교란 성공
        {
            // [3, 4단계] 머리 제외 + 파괴부위 무작위 추첨 및 가중치 적용
            const EBodyPart disturbed = DrawDisturbedPart();
            if (disturbed == EBodyPart::EBP_None) // 머리 제외 전 부위 파괴 = 사망(조건1)
            {
                BroadcastAllParts();
                return;
            }
            UE_LOG(LogWindless, Log, TEXT("[감각교란] 발동 (혼돈 %.1f) 부위 %d -> %d"), chaos, (int32)finalPart, (int32)disturbed);
            finalPart = disturbed;
        }
        // 교란 실패(이성 방어): 원래 피격 부위 유지
    }

    UE_LOG(LogWindless, Log, TEXT("[부위] 입력대미지=%.1f 혼돈=%.1f 확정부위=%d"),
        damage, chaos, (int32)finalPart);

    // [5단계] 최종 대미지 차감 및 부위별 피격 배율 적용
    ApplyFinalDamage(finalPart, damage, baseInjuryChance, true);
}

void UBodyPartComponent::DisableAllParts()
{
    for (TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        pair.Value.health = 0.f;
        pair.Value.bDisabled = true;
    }

    BroadcastAllParts();
}

void UBodyPartComponent::TriggerInjury(EBodyPart part, EInjuryType injury)
{
    FBodyPartState* state = bodyParts.Find(part);
    if (!state)
        return;

    FInjuryEntry* entry = state->injuries.FindByPredicate( // 부상 정보 찾기
        [injury](const FInjuryEntry& e) { return e.type == injury; });

    if (!entry || entry->bActive)
        return;

    entry->bActive = true;
    ApplyInjuryEffect(injury);
    UE_LOG(LogWindless, Warning, TEXT("[부상발동] 부위=%d 부상=%d"), (int32)part, (int32)injury);

    BroadcastPart(part);
}

void UBodyPartComponent::TriggerBleeding(EBodyPart part)
{
    if (bleedingParts.Contains(part))
        return;

    bleedingParts.Add(part);
    if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(bleedTimer))
        GetWorld()->GetTimerManager().SetTimer(bleedTimer, this, &UBodyPartComponent::ApplyBleed, bleedInterval, true);

    UE_LOG(LogWindless, Warning, TEXT("[부상발동] 부위=%d 과다출혈 시작"), (int32)part);
    BroadcastPart(part);
}

bool UBodyPartComponent::IsPartDisabled(EBodyPart part) const
{
    const FBodyPartState* state = bodyParts.Find(part);
    return state ? state->bDisabled : false;
}

bool UBodyPartComponent::IsPartInjured(EBodyPart part, EInjuryType injury) const
{
    if (injury == EInjuryType::EIT_Bleeding)
        return bleedingParts.Contains(part);

    const FBodyPartState* state = bodyParts.Find(part);
    if (!state)
        return false;

    for (const FInjuryEntry& e : state->injuries)
    {
        if (e.type == injury && e.bActive)
            return true;
    }
    return false;
}

bool UBodyPartComponent::IsDeadByParts() const
{
    if (IsPartDisabled(EBodyPart::EBP_Head))
        return true;

    return IsPartDisabled(EBodyPart::EBP_Chest)
        && IsPartDisabled(EBodyPart::EBP_LeftArm)
        && IsPartDisabled(EBodyPart::EBP_RightArm)
        && IsPartDisabled(EBodyPart::EBP_LeftLeg)
        && IsPartDisabled(EBodyPart::EBP_RightLeg);
}

bool UBodyPartComponent::HasInjury(EInjuryType injury) const
{
    for (const TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        for (const FInjuryEntry& e : pair.Value.injuries)
        {
            if (e.bActive && e.type == injury)
                return true;
        }
    }
    return false;
}

bool UBodyPartComponent::HasEyeDamage() const
{
    return HasInjury(EInjuryType::EIT_EyeDamage);
}

bool UBodyPartComponent::HasArmFractureSide(EBodyPart arm) const
{
    const FBodyPartState* s = bodyParts.Find(arm);
    if (!s)
        return false;

    for (const FInjuryEntry& e : s->injuries)
    {
        if (e.bActive && e.type == EInjuryType::EIT_ArmFracture)
            return true;
    }

    return false;
}

bool UBodyPartComponent::HasLegFractureSide(EBodyPart legPart) const
{
    const FBodyPartState* s = bodyParts.Find(legPart);
    if (!s)
        return false;
    for (const FInjuryEntry& e : s->injuries)
        if (e.bActive && e.type == EInjuryType::EIT_LegFracture)
            return true;
    return false;
}

bool UBodyPartComponent::CanSprint() const
{
    const bool leftBad = IsPartDisabled(EBodyPart::EBP_LeftLeg) || HasLegFractureSide(EBodyPart::EBP_LeftLeg);
    const bool rightBad = IsPartDisabled(EBodyPart::EBP_RightLeg) || HasLegFractureSide(EBodyPart::EBP_RightLeg);
    return !(leftBad || rightBad);
}

bool UBodyPartComponent::IsCrawling() const
{
    const bool leftBad = IsPartDisabled(EBodyPart::EBP_LeftLeg) || HasLegFractureSide(EBodyPart::EBP_LeftLeg);
    const bool rightBad = IsPartDisabled(EBodyPart::EBP_RightLeg) || HasLegFractureSide(EBodyPart::EBP_RightLeg);
    return leftBad && rightBad;
}

bool UBodyPartComponent::IsBleeding() const
{
    return bleedingParts.Num() > 0;
}

bool UBodyPartComponent::IsPartInjuredAny(EBodyPart part) const
{
    if (bleedingParts.Contains(part))
        return true;

    const FBodyPartState* state = bodyParts.Find(part);
    if (!state)
        return false;

    for (const FInjuryEntry& e : state->injuries)
    {
        if (e.bActive)
            return true;
    }

    return false;
}

float UBodyPartComponent::GetLockOnRangeMultiplier() const
{
    float m = 1.f;
    ApplySingleInjury([&m](const FInjuryData& d) { m *= d.lockOnRangeMultiplier; });
    return m;
}

float UBodyPartComponent::GetStaminaRegenMultiplier() const
{
    if (IsPartDisabled(EBodyPart::EBP_Chest))
        return chestDisabledStaminaRegenMultiplier;

    float m = 1.f;
    ApplySingleInjury([&m](const FInjuryData& d) { m *= d.staminaRegenMultiplier; });
    return m;
}

float UBodyPartComponent::GetActionDelayPenalty() const
{
    float delay = 0.f;
    ApplySingleInjury([&delay](const FInjuryData& d) { delay = FMath::Max(delay, d.actionDelay); });
    return delay;
}

float UBodyPartComponent::GetAttackRateMultiplier() const
{
    float m = 1.f;
    ApplySingleInjury([&m](const FInjuryData& d) { m *= d.attackRateMultiplier; });
    return m;
}

float UBodyPartComponent::GetStrongAttackTimeMultiplier() const
{
    float m = 1.f;
    ApplySingleInjury([&m](const FInjuryData& d) { m *= d.dodgeDistanceMultiplier; });
    return m;
}

void UBodyPartComponent::OnOwnerDealtHit(EBodyPart attackingArm)
{
    if (attackingArm != EBodyPart::EBP_LeftArm && attackingArm != EBodyPart::EBP_RightArm)
        return;

    if (!HasArmFractureSide(attackingArm))
        return;

    const FInjuryData* d = injuryCache.Find(EInjuryType::EIT_ArmFracture);
    const float cost = d ? d->onHitSelfHpCost : 0.f;
    if (cost > 0.f)
        ApplyFlatDamage(attackingArm, cost);
}

float UBodyPartComponent::GetDodgeDistanceMultiplier() const
{
    float m = 1.f;
    ApplySingleInjury([&m](const FInjuryData& d) { m *= d.dodgeDistanceMultiplier; });
    return m;
}

float UBodyPartComponent::GetMoveHpCost() const
{
    float cost = 0.f;
    ApplySingleInjury([&cost](const FInjuryData& d) { cost += d.moveHpCost; });
    return cost;
}

void UBodyPartComponent::ApplyFlatDamage(EBodyPart part, float damage)
{
    if (damage > 0.f)
        ApplyFinalDamage(part, damage, -1.f, false);
}

EBodyPart UBodyPartComponent::GetLowestHealthPart() const
{
    EBodyPart lowest = EBodyPart::EBP_None;
    float lowestPercent = TNumericLimits<float>::Max();

    for (const TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        const FBodyPartState& s = pair.Value;
        if (s.bDisabled || s.maxHealth <= 0.f)
            continue;

        if (s.health >= s.maxHealth)
            continue;

        const float percent = s.health / s.maxHealth;
        if (percent < lowestPercent)
        {
            lowestPercent = percent;
            lowest = pair.Key;
        }
    }
    return lowest;
}

bool UBodyPartComponent::HealPart(EBodyPart part, float amount)
{
    if (amount <= 0.f)
        return false;

    FBodyPartState* state = bodyParts.Find(part);
    if (!state || state->bDisabled) // 파괴된 부위 회복 불가
        return false;

    if (state->health >= state->maxHealth) // 체력이 가득 찬 부위 회복 불가
        return false;

    state->health = FMath::Min(state->maxHealth, state->health + amount);
    BroadcastPart(part);
    return true;
}

void UBodyPartComponent::HealAllParts()
{
    for (TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        pair.Value.health = pair.Value.maxHealth;
        pair.Value.bDisabled = false;
        for (FInjuryEntry& e : pair.Value.injuries)
            e.bActive = false;
    }

    ClearBleedingEffect();
    BroadcastAllParts();
}

void UBodyPartComponent::RegenHealth(float deltaTime)
{
    if (deltaTime <= 0.f)
        return;

    if (bSaveZoneActive)
        ReviveDestroyedPartsInSaveZone();

    if (IsBleeding())
        return;
    
    CalculateFinalHealthRegenRate();

    for (TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        FBodyPartState& s = pair.Value;
        if (s.bDisabled || s.health <= 0.f) // 파괴된 부위는 자연 회복 불가
            continue;

        if (s.health >= s.maxHealth) // 이미 가득 찬 부위는 무시
            continue;

        float rate = s.finalHealthRegenRate;
        if (bSaveZoneActive)
            rate += saveZoneRegenRate;

        if (rate <= 0.f) // 재생 속도가 없으면 회복 없음
            continue;

        const float before = s.health;
        s.health = FMath::Min(s.maxHealth, s.health + rate * deltaTime);
        if (!FMath::IsNearlyEqual(before, s.health))
            BroadcastPart(pair.Key);
    }
}

void UBodyPartComponent::CalculateFinalHealthRegenRate()
{
    for (TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        FBodyPartState& s = pair.Value;
        s.finalHealthRegenRate = (s.baseHealthRegenRate + s.flatHealthRegenAdd) * (1.f + s.multHealthRegenAdd);
    }
}

bool UBodyPartComponent::CurePartInjury(EBodyPart part, EInjuryType injury)
{
    if (injury == EInjuryType::EIT_None)
        return false;

    FBodyPartState* state = bodyParts.Find(part);
    if (!state)
        return false;

    bool bCured = false;

    if (injury == EInjuryType::EIT_Bleeding)
    {
        if (bleedingParts.Contains(part))
        {
            bleedingParts.Remove(part);
            if (bleedingParts.Num() == 0 && GetWorld())
                GetWorld()->GetTimerManager().ClearTimer(bleedTimer);
            bCured = true;
        }
    }
    else
    {
        FInjuryEntry* entry = state->injuries.FindByPredicate(
            [injury](const FInjuryEntry& e) { return e.type == injury; });

        if (entry && entry->bActive)
        {
            entry->bActive = false;
            bCured = true;
        }
    }

    if (bCured)
        BroadcastPart(part);

    return bCured;
}

void UBodyPartComponent::SetSaveZoneActive(bool bActive)
{
    bSaveZoneActive = bActive;
}

bool UBodyPartComponent::RevivePart(EBodyPart part, float healthRatio)
{
    FBodyPartState* state = bodyParts.Find(part);
    if (!state || !state->bDisabled)
        return false;

    state->bDisabled = false;
    state->health = state->maxHealth * FMath::Clamp(healthRatio, 0.01f, 1.f);

    for (FInjuryEntry& e : state->injuries) 
        e.bActive = false;

    if (bleedingParts.Contains(part))
    {
        bleedingParts.Remove(part);
        if (bleedingParts.Num() == 0 && GetWorld())
            GetWorld()->GetTimerManager().ClearTimer(bleedTimer);
    }

    BroadcastPart(part);
    return true;
}

void UBodyPartComponent::SetEquipmentMaxHealthBonus(EBodyPart part, float bonus)
{
    FBodyPartState* state = bodyParts.Find(part);
    if (!state)
        return;

    const float newMax = state->baseMaxHealth + bonus;
    if (FMath::IsNearlyEqual(state->maxHealth, newMax))
        return;

    state->maxHealth = newMax;
    state->health = FMath::Clamp(state->health, 0.f, state->maxHealth);

    UE_LOG(LogWindless, Log, TEXT("[부위체력] %d : 최대 %.0f (장비 +%.0f), 현재 %.0f"),
        (int32)part, state->maxHealth, bonus, state->health);
}

void UBodyPartComponent::ApplyLevelToParts(int32 level)
{
    const float steps = static_cast<float>(FMath::Max(0, level - 1));
    for (TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        FBodyPartState& state = pair.Value;

        float perLevel = WindlessLevelUp::LimbHealthPerLevel;
        if (pair.Key == EBodyPart::EBP_Head)
            perLevel = WindlessLevelUp::HeadHealthPerLevel;
        else if (pair.Key == EBodyPart::EBP_Chest)
            perLevel = WindlessLevelUp::ChestHealthPerLevel;

        const float equipmentBonus = state.maxHealth - state.baseMaxHealth;
        const float previousMax = state.maxHealth;

        state.baseMaxHealth = state.dataMaxHealth + perLevel * steps;
        state.maxHealth = state.baseMaxHealth + equipmentBonus;
        state.health = FMath::Clamp(state.health + (state.maxHealth - previousMax), 0.f, state.maxHealth);
    }

    BroadcastAllParts();
}

void UBodyPartComponent::PrintPartsDebug() const
{
    static const TCHAR* names[] = { TEXT("None"), TEXT("머리"), TEXT("가슴"),
    TEXT("왼팔"), TEXT("오른팔"), TEXT("왼다리"), TEXT("오른다리") };

    UE_LOG(LogWindless, Warning, TEXT("===== 부위 체력 ====="));
    for (int32 i = (int32)EBodyPart::EBP_Head; i <= (int32)EBodyPart::EBP_RightLeg; ++i)
    {
        const FBodyPartState* s = bodyParts.Find((EBodyPart)i);
        if (!s)
            continue;

        UE_LOG(LogWindless, Warning, TEXT("%-8s 현재 %6.1f / 최대 %6.1f (원본 %.0f, 장비 %+.0f)%s"),
            names[i], s->health, s->maxHealth, s->baseMaxHealth,
            s->maxHealth - s->baseMaxHealth,
            s->bDisabled ? TEXT(" [파괴]") : TEXT(""));
    }
}

void UBodyPartComponent::BroadcastAllParts()
{
    for (const TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
        BroadcastPart(pair.Key);
}

float UBodyPartComponent::GetTotalHealth() const
{
    float sum = 0.f;
    for (const TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
        sum += pair.Value.health;

    return sum;
}

float UBodyPartComponent::GetTotalMaxHealth() const
{
    float sum = 0.f;
    for (const TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
        sum += pair.Value.maxHealth;

    return sum;
}

float UBodyPartComponent::GetTotalHealthPercent() const
{
    const float maxSum = GetTotalMaxHealth();
    const float currentSum = GetTotalHealth();
    return maxSum > 0.f ?  currentSum / maxSum : 0.f;
}

float UBodyPartComponent::GetPartHealthPercent(EBodyPart part) const
{
    const FBodyPartState* state = bodyParts.Find(part);
    if (!state || state->maxHealth <= 0.f)
        return 0.f;

    return FMath::Clamp(state->health / state->maxHealth, 0.f, 1.f);
}

float UBodyPartComponent::GetPartHealth(EBodyPart part) const
{
    const FBodyPartState* state = bodyParts.Find(part);
    return state ? state->health : 0.f;
}

float UBodyPartComponent::GetPartMaxHealth(EBodyPart part) const
{
    const FBodyPartState* state = bodyParts.Find(part);
    return state ? state->maxHealth : 0.f;
}

void UBodyPartComponent::BeginPlay()
{
    Super::BeginPlay();
    if (AActor* owner = GetOwner())
        ownerAttributes = owner->FindComponentByClass<UAttributeComponent>();

    InitializeBodyParts();
    InitializeInjuryCache();
}

void UBodyPartComponent::InitializeBodyParts()
{
    bodyParts.Empty();
    if (!bodyPartData)
        return;
    
    const UEnum* enumPtr = StaticEnum<EBodyPart>();
    const int32 numEnum = enumPtr->NumEnums() - 1;
    for (int32 i = 0; i < numEnum; ++i)
    {
        const EBodyPart part = static_cast<EBodyPart>(enumPtr->GetValueByIndex(i));
        if (part == EBodyPart::EBP_None)
            continue;

        const FName rowName(*enumPtr->GetNameStringByIndex(i));
        const FBodyPartData* row = bodyPartData->FindRow<FBodyPartData>(rowName, TEXT("InitializeBodyParts"));

        if (!row)
            continue;

        FBodyPartState s;
        s.maxHealth = row->maxHealth;
        s.baseMaxHealth = row->maxHealth;
        s.dataMaxHealth = row->maxHealth;
        s.health = row->maxHealth;
        s.partDamageMultiplier = row->partDamageMultiplier;
        s.disturbWeight = row->disturbWeight;
        s.baseHealthRegenRate = row->baseHealthRegenRate;
        s.flatHealthRegenAdd = row->flatHealthRegenAdd;
        s.multHealthRegenAdd = row->multHealthRegenAdd;
        s.injuries = row->injuries;
        bodyParts.Add(part, s);
    }

    CalculateFinalHealthRegenRate();
}

void UBodyPartComponent::InitializeBoneToPartMap()
{
    boneToPartMap.Add(FName("head"), EBodyPart::EBP_Head);
    boneToPartMap.Add(FName("neck_01"), EBodyPart::EBP_Head);
    boneToPartMap.Add(FName("spine_01"), EBodyPart::EBP_Chest);
    boneToPartMap.Add(FName("spine_02"), EBodyPart::EBP_Chest);
    boneToPartMap.Add(FName("spine_03"), EBodyPart::EBP_Chest);
    boneToPartMap.Add(FName("pelvis"), EBodyPart::EBP_Chest);
    boneToPartMap.Add(FName("clavicle_l"), EBodyPart::EBP_LeftArm);
    boneToPartMap.Add(FName("upperarm_l"), EBodyPart::EBP_LeftArm);
    boneToPartMap.Add(FName("lowerarm_l"), EBodyPart::EBP_LeftArm);
    boneToPartMap.Add(FName("hand_l"), EBodyPart::EBP_LeftArm);
    boneToPartMap.Add(FName("clavicle_r"), EBodyPart::EBP_RightArm);
    boneToPartMap.Add(FName("upperarm_r"), EBodyPart::EBP_RightArm);
    boneToPartMap.Add(FName("lowerarm_r"), EBodyPart::EBP_RightArm);
    boneToPartMap.Add(FName("hand_r"), EBodyPart::EBP_RightArm);
    boneToPartMap.Add(FName("thigh_l"), EBodyPart::EBP_LeftLeg);
    boneToPartMap.Add(FName("calf_l"), EBodyPart::EBP_LeftLeg);
    boneToPartMap.Add(FName("foot_l"), EBodyPart::EBP_LeftLeg);
    boneToPartMap.Add(FName("thigh_r"), EBodyPart::EBP_RightLeg);
    boneToPartMap.Add(FName("calf_r"), EBodyPart::EBP_RightLeg);
    boneToPartMap.Add(FName("foot_r"), EBodyPart::EBP_RightLeg);
}

void UBodyPartComponent::InitializeInjuryCache()
{
    injuryCache.Empty();
    if (!injuryDataTable)
        return;

    const UEnum* enumPtr = StaticEnum<EInjuryType>();
    const int32 numEnum = enumPtr->NumEnums() - 1;

    for (int32 i = 0; i < numEnum; ++i)
    {
        const EInjuryType type = static_cast<EInjuryType>(enumPtr->GetValueByIndex(i));
        if (type == EInjuryType::EIT_None)
            continue;

        const FName rowName(*enumPtr->GetNameStringByIndex(i));
        if (const FInjuryData* data = injuryDataTable->FindRow<FInjuryData>(rowName, TEXT("InitializeInjuryCache")))
            injuryCache.Add(type, *data);
    }
}

EBodyPart UBodyPartComponent::DrawDisturbedPart() const
{
    TArray<EBodyPart> parts;
    TArray<float> weights;
    float total = 0.f;

    for (const TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        if (pair.Key == EBodyPart::EBP_Head)
            continue;
        if (pair.Value.bDisabled || pair.Value.health <= 0.f)
            continue;

        const float w = FMath::Max(0.f, pair.Value.disturbWeight);
        if (w <= 0.f)
            continue;

        parts.Add(pair.Key);
        weights.Add(w);
        total += w;
    }

    if (parts.Num() == 0 || total <= 0.f)
        return EBodyPart::EBP_None;

    float roll = FMath::FRandRange(0.f, total);
    for (int32 i = 0; i < parts.Num(); ++i)
    {
        roll -= weights[i];
        if (roll <= 0.f)
            return parts[i];
    }
    return parts.Last();
}

void UBodyPartComponent::ApplyFinalDamage(EBodyPart part, float damage, float baseInjuryChance, bool bRollInjury)
{
    FBodyPartState* state = bodyParts.Find(part);
    if (!state || state->bDisabled)
        return;

    const float mult = (state->partDamageMultiplier > 0.f) ? state->partDamageMultiplier : 1.f;
    const float before = state->health;
    state->health = FMath::Max(0.f, state->health - damage * mult);
    const float dealt = before - state->health;
    const bool bJustDestroyed = (state->health <= 0.f);

    if (bRollInjury && !bJustDestroyed)
        TryApplyInjury(state, part, baseInjuryChance);

    if (bRollInjury)
        TryTriggerBleeding(part, state->maxHealth);

    if (bJustDestroyed && !state->bDisabled)
        OnPartDisabled(part);

    if (ownerAttributes && dealt > 0.f)
        ownerAttributes->ApplyDamageChaos(dealt, state->maxHealth);

    UE_LOG(LogWindless, Log, TEXT("[부위] %d 차감: %.1f→%.1f (배율 %.2f, 실차감 %.1f)%s"),
        (int32)part, before, state->health, mult, dealt, bJustDestroyed ? TEXT(" [파괴]") : TEXT(""));
    BroadcastPart(part);
}

void UBodyPartComponent::TryApplyInjury(FBodyPartState* state, EBodyPart part, float baseInjuryChance)
{
    if (!state)
        return;

    const float reduction = ownerAttributes ? ownerAttributes->GetInjuryChanceReduction() : 0.f;
    for (FInjuryEntry& entry : state->injuries)
    {
        if (entry.bActive || entry.type == EInjuryType::EIT_None)
            continue;

        float chance = (baseInjuryChance >= 0.f) ? baseInjuryChance : entry.chance;
        chance = FMath::Clamp(chance - reduction, 0.f, 1.f);

        const float roll = FMath::FRand();
        UE_LOG(LogWindless, Log, TEXT("[부상] 부위=%d 종류=%d 기본=%.2f 각성차감=%.2f 최종=%.2f roll=%.2f -> %s"),
            (int32)part, (int32)entry.type, (baseInjuryChance >= 0.f ? baseInjuryChance : entry.chance),
            reduction, chance, roll, (roll <= chance) ? TEXT("발동") : TEXT("실패"));
        if (roll <= chance)
            TriggerInjury(part, entry.type);
    }
}

void UBodyPartComponent::ApplyInjuryEffect(const EInjuryType injury)
{
    const FInjuryData* data = injuryCache.Find(injury);
    if (!data)
        return;

    if (ownerAttributes && data->onActivateChaosLossRatio > 0.f)
        ownerAttributes->LoseChaos(ownerAttributes->GetMaxChaos() * data->onActivateChaosLossRatio);
}

void UBodyPartComponent::ApplySingleInjury(TFunctionRef<void(const FInjuryData&)> injuryData) const
{
    TSet<EInjuryType> seen;
    for (const TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        for (const FInjuryEntry& e : pair.Value.injuries)
        {
            if (!e.bActive || e.type == EInjuryType::EIT_None)
                continue;

            if (seen.Contains(e.type))
                continue;

            seen.Add(e.type);
            if (const FInjuryData* data = injuryCache.Find(e.type))
                injuryData(*data);
        }
    }
}

void UBodyPartComponent::TryTriggerBleeding(EBodyPart part, float partMaxHealth)
{
    if (bleedingParts.Contains(part) || partMaxHealth <= 0.f)
        return;

    float chance = bleedChance;
    if (ownerAttributes)
        chance = FMath::Clamp(chance - ownerAttributes->GetInjuryChanceReduction(), 0.f, 1.f);

    if (FMath::FRand() <= chance)
        TriggerBleeding(part);
}

void UBodyPartComponent::ApplyBleed()
{
    if (!GetOwner())
        return;

    TArray<EBodyPart> activeBleeds = bleedingParts.Array();
    for (EBodyPart part : activeBleeds)
    {
        const FBodyPartState* s = bodyParts.Find(part);
        if (!s || s->bDisabled)
        {
            bleedingParts.Remove(part);
            continue;
        }
        ApplyFlatDamage(part, bleedDamage);
    }

    if (bleedingParts.Num() == 0 && GetWorld())
        GetWorld()->GetTimerManager().ClearTimer(bleedTimer);
}

void UBodyPartComponent::OnPartDisabled(EBodyPart part)
{
    FBodyPartState* state = bodyParts.Find(part);
    if (!state)
        return;

    state->bDisabled = true;
    UE_LOG(LogWindless, Warning, TEXT("[부위파괴] 부위=%d"), (int32)part);

    BroadcastPart(part);
}

void UBodyPartComponent::ClearBleedingEffect()
{
    bleedingParts.Empty();
    if (GetWorld())
        GetWorld()->GetTimerManager().ClearTimer(bleedTimer);
}

void UBodyPartComponent::ReviveDestroyedPartsInSaveZone()
{
    if (IsDeadByParts())
        return;

    TArray<EBodyPart> destroyedParts;
    for (const TPair<EBodyPart, FBodyPartState>& pair : bodyParts)
    {
        if (pair.Value.bDisabled)
            destroyedParts.Add(pair.Key);
    }

    for (const EBodyPart part : destroyedParts)
    {
        if (RevivePart(part, saveZoneReviveHealthRatio))
            UE_LOG(LogWindless, Log, TEXT("[세이브포인트] 파괴 부위 복구: %d (체력 %.0f%%)"), (int32)part, saveZoneReviveHealthRatio * 100.f);
    }
}

void UBodyPartComponent::BroadcastPart(EBodyPart part)
{
    const FBodyPartState* bodyPart = bodyParts.Find(part);
    if (!bodyPart) // 해당 부위가 존재하지 않으면 무시
        return;

    const float percent = (bodyPart->maxHealth > 0.f) ? (bodyPart->health / bodyPart->maxHealth) : 0.f;
    bool bInjured = false;
    for (const FInjuryEntry& e : bodyPart->injuries)
    {
        if (e.bActive) 
        { 
            bInjured = true; 
            break; 
        }
    }

    if (bleedingParts.Contains(part))
        bInjured = true;

    onBodyPartChanged.Broadcast(part, percent, bInjured, bodyPart->bDisabled);
}
