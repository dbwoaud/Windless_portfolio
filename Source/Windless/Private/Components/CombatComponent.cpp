#include "Components/CombatComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/BowComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Characters/BodyPartTypes.h"
#include "Windless/DebugMacro.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Weapons/Bow.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h" 

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::HandleAttackPressed(EAttackHand hand, EAttackType attackType)
{
	if (!character)
		return;

	const EAttackHand attackHand = ResolveAttackHand(hand);
	AWeapon* handWeapon = GetHandWeapon(attackHand);

	if (handWeapon && handWeapon->IsBow())
	{
		if (UBowComponent* bow = character->GetBow())
			bow->HandleBowPressed(Cast<ABow>(handWeapon), hand);
		return;
	}

	if (handWeapon && handWeapon->IsShield() && !IsOwnerInAir())
		return;

	/* 공중에서는 점프 공격 */
	if (IsOwnerInAir()) 
		attackType = EAttackType::EAT_Jump;

	/* 공격 중인 경우 콤보 예약 */
	if (character->GetActionState() == EActionState::EAS_Attacking)
	{
		if (bComboWindowOpen && activeCombo.bValid &&
			activeCombo.hand == attackHand && activeCombo.attackType == attackType)
		{
			bComboInputBuffered = true;
		}
		return;
	}

	/* 같은 프레임에 들어온 입력을 모아 다음에 확정 */
	if (!pendingInput.bValid)
	{
		pendingInput.bValid = true;
		pendingInput.hand = attackHand;
		pendingInput.attackType = attackType;

		if (UWorld* world = GetWorld())
			world->GetTimerManager().SetTimerForNextTick(this, &UCombatComponent::ResolvePendingAttack);
		return;
	}

	if (pendingInput.hand == attackHand && attackType == EAttackType::EAT_Heavy) // Shift+클릭으로 약/강 입력이 함께 들어오면 강공격 우선
		pendingInput.attackType = EAttackType::EAT_Heavy;
}

void UCombatComponent::HandleAttackHeld(EAttackHand hand)
{
	if (!character || bIsGuarding)
		return;

	if (character->IsQuickSlotActive() || character->CheckActionDelay())
		return;

	if (IsOwnerInAir()) // 공중에서는 가드 불가
		return;

	AWeapon* handWeapon = GetHandWeapon(ResolveAttackHand(hand));
	if (handWeapon && handWeapon->IsShield())
		StartGuardWithHand(handWeapon);
}

void UCombatComponent::HandleAttackReleased(EAttackHand hand)
{
	if (!character)
		return;

	if (UBowComponent* bow = character->GetBow(); bow && bow->IsDrawing())
	{
		bow->HandleBowReleased(hand);
		return;
	}

	if (!bIsGuarding)
		return;

	if (guardingWeapon == GetHandWeapon(ResolveAttackHand(hand)))
		StopGuard();
}

void UCombatComponent::OnOwnerLanded()
{
	bJumpAttackUsed = false;

	if (!character || !activeCombo.bValid || activeCombo.attackType != EAttackType::EAT_Jump || !comboMontage)
		return;

	if (comboMontage->GetSectionIndex(jumpLandSectionName) == INDEX_NONE)
		return;

	if (character->JumpToAttackSection(comboMontage, jumpLandSectionName))
		UE_LOG(LogWindless, Log, TEXT("[점프공격] 착지 - %s 섹션으로 전환"), *jumpLandSectionName.ToString());
}

void UCombatComponent::OpenComboWindow()
{
	if (!activeCombo.bValid)
		return;

	bComboWindowOpen = true;
	bComboInputBuffered = false;
}

void UCombatComponent::CloseComboWindow()
{
	if (!bComboWindowOpen)
		return;

	bComboWindowOpen = false;
	if (!bComboInputBuffered)
		return;

	bComboInputBuffered = false;
	TryAdvanceCombo();
}

void UCombatComponent::ResetCombo()
{
	activeCombo.Reset();
	comboIndex = 0;
	bComboWindowOpen = false;
	bComboInputBuffered = false;
	comboMontage = nullptr;
	comboWeapon = nullptr;
}

void UCombatComponent::StopGuard()
{
	if (!bIsGuarding) // 방어 중이 아니면 무시
		return;

	bIsGuarding = false;
	guardingWeapon = nullptr;
	if (character)
		character->ExitGuardState();
}

bool UCombatComponent::IsGuardingAgainst(AActor* hitter) const
{
	if (!bIsGuarding || !hitter || !guardingWeapon || !character)
		return false;

	const FVector toHit = (hitter->GetActorLocation() - character->GetActorLocation()).GetSafeNormal2D();
	const float dot = FVector::DotProduct(character->GetActorForwardVector(), toHit);
	const float threshold = FMath::Cos(FMath::DegreesToRadians(guardingWeapon->GetGuardAngle()));
	return dot > threshold;
}

bool UCombatComponent::HandleGuard(AActor* attacker, const FCombatHitInfo& hitInfo, FGuardResult& outResult)
{
	outResult = FGuardResult();
	outResult.residualDamage = hitInfo.damage;

	if (!IsGuardingAgainst(attacker) || !guardingWeapon || !character) // 가드 각도 밖이거나 가드 중이 아니면 전량 피격
		return false;

	if (hitInfo.bUnblockable) // 가드 불가 공격은 가드를 무시하고 강제 피격
	{
		StopGuard();
		return false;
	}

	character->MarkCombatEngaged(); // 막아낸 공격도 전투 상태로 간주

	const float guardValue = guardingWeapon->GetGuardValue();
	const float staminaCost = WindlessCombat::CalcGuardStaminaCost(hitInfo.damage, guardValue);
	guardingWeapon->PlayGuardSound();

	outResult.bBlocked = true;

	if (staminaCost <= 0.f) // 완전 가드
	{
		outResult.residualDamage = 0.f;
		return true;
	}

	const float preStamina = character->GetCurrentStamina();

	if (preStamina > staminaCost) // 스태미나로 대미지 흡수
	{
		character->ConsumeStamina(staminaCost);
		outResult.staminaCost = staminaCost;
		outResult.residualDamage = 0.f;
		return true;
	}

	// 가드 브레이크
	character->ConsumeStamina(preStamina);
	outResult.bGuardBroken = true;
	outResult.staminaCost = preStamina;
	outResult.residualDamage = staminaCost - preStamina;

	StopGuard();
	return true;
}

void UCombatComponent::OnHandWeaponChanged(AWeapon* removedWeapon)
{
	pendingInput.Reset();

	if (!removedWeapon)
		return;

	if (character && character->GetBow())
		character->GetBow()->OnWeaponRemoved(removedWeapon);

	if (guardingWeapon == removedWeapon)
		StopGuard();

	if (comboWeapon == removedWeapon)
		ResetCombo();
}

bool UCombatComponent::IsGuardOnLeftHand() const
{
	return character && guardingWeapon && guardingWeapon == character->GetLeftHandWeapon();
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	character = Cast<AWindlessCharacter>(GetOwner());
}

void UCombatComponent::ResolvePendingAttack()
{
	if (!pendingInput.bValid)
		return;

	const FAttackInputRequest request = pendingInput;
	pendingInput.Reset();
	ExecuteAttack(request.hand, request.attackType);
}

EAttackHand UCombatComponent::ResolveAttackHand(EAttackHand hand) const
{
	if (!character)
		return hand;

	const AWeapon* rightWeapon = character->GetRightHandWeapon();
	const AWeapon* leftWeapon = character->GetLeftHandWeapon();

	if (rightWeapon && rightWeapon->OccupiesBothHands())
		return EAttackHand::Right;

	if (leftWeapon && leftWeapon->OccupiesBothHands())
		return EAttackHand::Left;

	return hand;
}

void UCombatComponent::ExecuteAttack(EAttackHand hand, EAttackType attackType)
{
	if (!character || character->CheckActionDelay())
		return;

	if (character->IsQuickSlotActive())
		return;

	AWeapon* handWeapon = GetHandWeapon(hand);
	if (!handWeapon || handWeapon->IsBow())
		return;

	const bool bShieldAttack = handWeapon->IsShield();
	if (bShieldAttack && !IsOwnerInAir()) // 방패는 점프 공격만 가능
		return;

	if (IsAttackArmDisabled(hand, handWeapon))
		return;

	if (!character->CanAttack())
		return;

	if (attackType == EAttackType::EAT_Jump)
	{
		if (!IsOwnerInAir())
			attackType = EAttackType::EAT_Light;
		else if (bJumpAttackUsed)
			return;
	}
	else if (IsOwnerInAir())
	{
		if (bJumpAttackUsed)
			return;
		attackType = EAttackType::EAT_Jump;
	}

	if (!character->HasEnoughAttackStamina(handWeapon, attackType)) // 스태미나 부족 시 약/강공격 미발동 
		return;
	
	UAnimMontage* montage = nullptr;
	if (bShieldAttack)
		montage = GetShieldJumpMontage(hand);
	else
		montage = GetAttackMontage(GetAttackProfile(hand, handWeapon), attackType);

	if (!montage)
		return;

	character->BeginAttack();
	if (!character->PerformAttack(handWeapon, attackType, montage))
		return;

	if (attackType == EAttackType::EAT_Jump)
		bJumpAttackUsed = true;

	/* 콤보 시작 */
	activeCombo.bValid = true;
	activeCombo.hand = hand;
	activeCombo.attackType = attackType;
	comboIndex = 0;
	bComboWindowOpen = false;
	bComboInputBuffered = false;
	comboMontage = montage;
	comboWeapon = handWeapon;

	const int32 maxHits = (attackType == EAttackType::EAT_Jump) ? 1 : FMath::Min(maxComboCount, montage->GetNumSections());
	UE_LOG(LogWindless, Log, TEXT("[콤보] 1타 시작 %s손 %s (최대 %d타)"),
		hand == EAttackHand::Left ? TEXT("왼") : TEXT("오른"),
		WindlessCombat::GetAttackTypeName(attackType),
		maxHits);
}

AWeapon* UCombatComponent::GetHandWeapon(EAttackHand hand) const
{
	if (!character)
		return nullptr;

	return (hand == EAttackHand::Left) ? character->GetLeftHandWeapon() : character->GetRightHandWeapon();
}

const FAttackProfile& UCombatComponent::GetAttackProfile(EAttackHand hand, const AWeapon* weapon) const
{
	if (weapon && weapon->IsTwoHanded()) // 같은 무기 타입은 같은 모션
		return twoHandedAttacks;

	return (hand == EAttackHand::Left) ? leftHandAttacks : rightHandAttacks;
}

UAnimMontage* UCombatComponent::GetAttackMontage(const FAttackProfile& profile, EAttackType attackType) const
{
	switch (attackType)
	{
		case EAttackType::EAT_Heavy: return profile.strongMontage;
		case EAttackType::EAT_Jump:  return profile.jumpMontage;
		default:                     return profile.normalMontage;
	}
}

UAnimMontage* UCombatComponent::GetShieldJumpMontage(EAttackHand hand) const
{
	if (hand == EAttackHand::Right && rightShieldJumpMontage)
		return rightShieldJumpMontage;

	return leftShieldJumpMontage;
}

bool UCombatComponent::TryAdvanceCombo()
{
	if (!character || !comboMontage || !comboWeapon || !activeCombo.bValid)
		return false;

	if (character->GetActionState() != EActionState::EAS_Attacking)
		return false;

	const int32 comboLength = FMath::Min(maxComboCount, comboMontage->GetNumSections());
	const int32 nextIndex = comboIndex + 1;
	if (nextIndex >= comboLength)
		return false;

	if (IsAttackArmDisabled(activeCombo.hand, comboWeapon))
		return false;

	if (!character->HasEnoughAttackStamina(comboWeapon, activeCombo.attackType))
	{
		UE_LOG(LogWindless, Log, TEXT("[콤보] 스태미나 부족으로 %d타에서 종료"), comboIndex + 1);
		return false;
	}

	const FName sectionName = comboMontage->GetSectionName(nextIndex);
	if (!character->ContinueAttackCombo(comboWeapon, activeCombo.attackType, comboMontage, sectionName))
		return false;

	comboIndex = nextIndex;
	UE_LOG(LogWindless, Log, TEXT("[콤보] %d타 연결 (섹션 %s)"), comboIndex + 1, *sectionName.ToString());
	return true;
}

void UCombatComponent::StartGuardWithHand(AWeapon* handWeapon)
{
	if (!character)
		return;

	if (UBodyPartComponent* bodyPart = character->GetBodyParts()) // 팔 파괴 시 가드 불가
	{
		const bool bLeftShield = (handWeapon == character->GetLeftHandWeapon());
		const EBodyPart shieldArm = bLeftShield ? EBodyPart::EBP_LeftArm : EBodyPart::EBP_RightArm;
		if (bodyPart->IsPartDisabled(shieldArm))
			return;
	}

	if (character->IsOccupied() || character->IsMounted()) // 특정 행동 상태이거나 탑승 중엔 방어 불가
		return;

	bIsGuarding = true;
	guardingWeapon = handWeapon;
	character->EnterGuardState();
}

bool UCombatComponent::IsAttackArmDisabled(EAttackHand hand, const AWeapon* weapon) const
{
	const UBodyPartComponent* bodyPart = character ? character->GetBodyParts() : nullptr;
	if (!bodyPart)
		return false;

	if (weapon && weapon->OccupiesBothHands())
		return bodyPart->IsPartDisabled(EBodyPart::EBP_LeftArm) || bodyPart->IsPartDisabled(EBodyPart::EBP_RightArm);

	const EBodyPart armPart = (hand == EAttackHand::Left) ? EBodyPart::EBP_LeftArm : EBodyPart::EBP_RightArm;
	return bodyPart->IsPartDisabled(armPart);
}

bool UCombatComponent::IsOwnerInAir() const
{
	const UCharacterMovementComponent* movement = character ? character->GetCharacterMovement() : nullptr;
	return movement && movement->IsFalling();
}
