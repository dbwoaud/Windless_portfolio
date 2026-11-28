#include "Cheats/StatusCheats.h"
#include "Characters/BodyPartTypes.h"
#include "Components/AttributeComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/PlayerProgressionComponent.h"
#include "Windless/DebugMacro.h"

void UStatusCheats::StaAttribute()
{
	if (const UAttributeComponent* attributes = GetPlayerComponent<UAttributeComponent>())
		UE_LOG(LogWindless, Warning, TEXT("===== 스테이터스 ===== 공격력 %.1f | 혼돈 %.1f | 체온 %.1f"), attributes->GetFinalAttackPower(), attributes->GetChaos(), attributes->GetBodyHeat());

	if (UBodyPartComponent* bodyParts = GetPlayerComponent<UBodyPartComponent>())
	{
		UE_LOG(LogWindless, Warning, TEXT("전체 체력 %.0f / %.0f (%.1f%%)"), bodyParts->GetTotalHealth(), bodyParts->GetTotalMaxHealth(), bodyParts->GetTotalHealthPercent() * 100.0f);
		bodyParts->PrintPartsDebug();
	}
}

void UStatusCheats::StaHurt(int32 part, float amount)
{
	if (UBodyPartComponent* bodyParts = GetPlayerComponent<UBodyPartComponent>())
		bodyParts->ApplyFlatDamage((EBodyPart)part, amount);
}

void UStatusCheats::StaInjure(int32 part, int32 injury)
{
	UBodyPartComponent* bodyParts = GetPlayerComponent<UBodyPartComponent>();
	if (!bodyParts)
		return;

	const EBodyPart targetPart = (EBodyPart)part;
	const EInjuryType targetInjury = (EInjuryType)injury;

	if (targetInjury == EInjuryType::EIT_Bleeding)
	{
		bodyParts->TriggerBleeding(targetPart);
		return;
	}

	bodyParts->TriggerInjury(targetPart, targetInjury);
}

void UStatusCheats::StaSaveZone(bool bActive)
{
	if (UBodyPartComponent* bodyParts = GetPlayerComponent<UBodyPartComponent>())
		bodyParts->SetSaveZoneActive(bActive);
}

void UStatusCheats::WLChaos(float value)
{
	UAttributeComponent* attributes = GetPlayerComponent<UAttributeComponent>();
	if (!attributes)
		return;

	attributes->DebugSetChaos(value);
	UE_LOG(LogWindless, Warning, TEXT("[디버그] 혼돈 지정 %.1f (단계 %d)"), attributes->GetChaos(), (int32)attributes->GetChaosState());
}

void UStatusCheats::WLMolt()
{
	UPlayerProgressionComponent* progression = GetPlayerComponent<UPlayerProgressionComponent>();
	if (!progression || progression->TryMolt())
		return;

	const UAttributeComponent* attributes = GetPlayerComponent<UAttributeComponent>();
	UE_LOG(LogWindless, Warning, TEXT("[탈피] 실패 - 탈피석 부족 (보유 %d / 필요 %d)"),
		attributes ? attributes->GetMoltStones() : 0,
		attributes ? attributes->GetMoltCost() : 0);
}

void UStatusCheats::WLAddStone(int32 amount)
{
	if (UAttributeComponent* attributes = GetPlayerComponent<UAttributeComponent>())
		attributes->AddMoltStones(amount);
}