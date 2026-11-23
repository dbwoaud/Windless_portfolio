#include "Cheats/CombatCheats.h"
#include "Characters/WindlessCharacter.h"
#include "Characters/CombatTypes.h"
#include "Components/WeaponLoadoutComponent.h"
#include "Components/PlayerAISignal.h"
#include "Items/Weapons/Weapon.h"
#include "Windless/DebugMacro.h"

void UCombatCheats::WLCombat()
{
	const AWindlessCharacter* character = GetPlayerCharacter();
	if (!character)
		return;

	UE_LOG(LogWindless, Warning, TEXT("[전투 상태] %s (남은 %.1f초)"),
		character->IsInCombatState() ? TEXT("전투 중") : TEXT("해제"), character->GetTimeSinceLastDamage());

	UE_LOG(LogWindless, Warning, TEXT("[전투 수치] 공격력(무기 제외)=%.1f | 스태미나 %.1f"),
		character->GetAttackPower(), character->GetCurrentStamina());

	auto logWeapon = [character](const TCHAR* handLabel, const AWeapon* weapon)
		{
			if (!weapon)
			{
				UE_LOG(LogWindless, Warning, TEXT("[무기] %s: 없음"), handLabel);
				return;
			}

			const float attackPower = character->GetAttackPowerWith(weapon);
			const FAttackSpec& light = weapon->GetAttackSpec(EAttackType::EAT_Light);
			const FAttackSpec& heavy = weapon->GetAttackSpec(EAttackType::EAT_Heavy);
			const FAttackSpec& jump = weapon->GetAttackSpec(EAttackType::EAT_Jump);
			UE_LOG(LogWindless, Warning, TEXT("[무기] %s: %s 타입=%s | 공격력=%.1f (약 %.1f / 강 %.1f / 점프 %.1f) | 약(%.0f/%.0f) 강(%.0f/%.0f) 점프(%.0f/%.0f) [스태미나/충격량] | 가드수치=%.0f"),
				handLabel, *weapon->GetName(), *UEnum::GetValueAsString(weapon->GetWeaponType()),
				attackPower,
				WindlessCombat::CalcHitDamage(attackPower, EAttackType::EAT_Light),
				WindlessCombat::CalcHitDamage(attackPower, EAttackType::EAT_Heavy),
				WindlessCombat::CalcHitDamage(attackPower, EAttackType::EAT_Jump),
				light.staminaCost, light.impact, heavy.staminaCost, heavy.impact, jump.staminaCost, jump.impact,
				weapon->GetGuardValue());
		};

	logWeapon(TEXT("왼손"), character->GetLeftHandWeapon());
	logWeapon(TEXT("오른손"), character->GetRightHandWeapon());

	if (const UWeaponLoadoutComponent* weaponLoadout = GetPlayerComponent<UWeaponLoadoutComponent>())
		weaponLoadout->PrintLoadoutDebug();
}

void UCombatCheats::WLStagger()
{
	AWindlessCharacter* character = GetPlayerCharacter();
	if (!character)
		return;

	if (character->ForceStrongStagger(character->GetActorLocation() + character->GetActorForwardVector() * 100.f))
		UE_LOG(LogWindless, Warning, TEXT("[디버그] 플레이어 강한 경직 시험"));
}

void UCombatCheats::WLSignal(const FString& signalName, float duration)
{
	UPlayerAISignal* aiSignals = GetPlayerComponent<UPlayerAISignal>();
	if (!aiSignals)
		return;

	struct FSignalName { const TCHAR* name; EPlayerAIState state; };
	static const FSignalName names[] = {
		{ TEXT("attack"), EPlayerAIState::EPS_AttackBegin },
		{ TEXT("whiff"),  EPlayerAIState::EPS_Whiff },
		{ TEXT("item"),   EPlayerAIState::EPS_ItemUse },
		{ TEXT("aim"),    EPlayerAIState::EPS_BowAim },
		{ TEXT("guard"),  EPlayerAIState::EPS_GuardHold },
	};

	for (const FSignalName& entry : names)
	{
		if (signalName.Equals(entry.name, ESearchCase::IgnoreCase))
		{
			aiSignals->FakeSignal(entry.state, duration > 0.f ? duration : 1.5f);
			return;
		}
	}

	UE_LOG(LogWindless, Warning, TEXT("[AI 신호] 사용법: WLSignal attack | whiff | item | aim | guard"));
}