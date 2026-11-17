#include "Components/CharacterDebugComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/InventoryComponent.h"
#include "Characters/BodyPartTypes.h"
#include "EnhancedInputComponent.h"
#include "Windless/DebugMacro.h"

UCharacterDebugComponent::UCharacterDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterDebugComponent::BindDebugInputs(UEnhancedInputComponent* enhancedInputComponent)
{
	if (!enhancedInputComponent)
		return;

	/* 정신력 디버그 */
	if (recoverChaosAction)
		enhancedInputComponent->BindAction(recoverChaosAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugRecoverChaos);
	if (loseChaosAction)
		enhancedInputComponent->BindAction(loseChaosAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugLoseChaos);

	/* 부위별 대미지 디버그 */
	if (damagePartAction)
		enhancedInputComponent->BindAction(damagePartAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugDamageSelectedPart);
	if (cyclePartAction)
		enhancedInputComponent->BindAction(cyclePartAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugCyclePart);
	if (healPartsAction)
		enhancedInputComponent->BindAction(healPartsAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugHealAllParts);
}

void UCharacterDebugComponent::InvAdd(const FString& itemId, int32 count)
{
	if (ownerInventory)
		ownerInventory->AddItemById(FName(*itemId), count);
}

void UCharacterDebugComponent::InvEquip(const FString& itemId, int32 slot)
{
	if (!ownerInventory)
		return;

	const int64 uid = ownerInventory->FindFirstUnequippedUID(FName(*itemId));
	if (uid == 0)
		return;
	ownerInventory->EquipItem(uid, (EEquipSlot)slot);
}

void UCharacterDebugComponent::InvUnequip(int32 slot)
{
	if (ownerInventory)
		ownerInventory->UnequipSlot((EEquipSlot)slot);
}

void UCharacterDebugComponent::InvUse(const FString& itemId)
{
	if (!ownerInventory)
		return;

	const int64 uid = ownerInventory->FindFirstUnequippedUID(FName(*itemId));
	if (uid != 0)
		ownerInventory->UseItem(uid);
}

void UCharacterDebugComponent::InvDur(int32 slot, float amount)
{
	if (ownerInventory)
		ownerInventory->DamageEquippedDurability((EEquipSlot)slot, amount);
}

void UCharacterDebugComponent::InvRepair(int32 slot, float amount)
{
	if (!ownerInventory)
		return;

	const int64 uid = ownerInventory->GetEquippedInstanceUID((EEquipSlot)slot);
	if (uid == 0)
		return;
	
	ownerInventory->RepairItem(uid, amount);
}

void UCharacterDebugComponent::InvMove(int32 uid, int32 x, int32 y, bool bRot)
{
	if (ownerInventory)
		ownerInventory->MoveItem((int64)uid, x, y, bRot);
}

void UCharacterDebugComponent::InvPrint()
{
	if (!ownerInventory)
		return;

	ownerInventory->PrintInventoryDebug();
	ownerInventory->PrintEquippedDebug();
	ownerInventory->PrintGridDebug();
}

void UCharacterDebugComponent::InvSplit(const FString& itemId, int32 amount)
{
	if (!ownerInventory)
		return;

	const int64 uid = ownerInventory->FindFirstUnequippedUID(FName(*itemId));
	if (uid == 0)
		return;

	ownerInventory->SplitStackAuto(uid, amount);
}

void UCharacterDebugComponent::InvDrop(const FString& itemId, int32 quantity)
{
	if (!ownerInventory)
		return;

	const int64 uid = ownerInventory->FindFirstUnequippedUID(FName(*itemId));
	if (uid == 0)
		return;

	ownerInventory->DropItem(uid, quantity);
}

void UCharacterDebugComponent::InvLimit()
{
	if (ownerInventory)
		ownerInventory->PrintCarryLimitDebug();
}

void UCharacterDebugComponent::InvStats()
{
	if (ownerAttributes)
		UE_LOG(LogWindless, Warning, TEXT("===== 스테이터스 ===== 공격력 %.1f | 혼돈 %.1f | 체온 %.1f"), ownerAttributes->GetFinalAttackPower(), ownerAttributes->GetChaos(), ownerAttributes->GetBodyHeat());

	if (ownerBodyParts)
	{
		UE_LOG(LogWindless, Warning, TEXT("전체 체력 %.0f / %.0f (%.1f%%)"), ownerBodyParts->GetTotalHealth(), ownerBodyParts->GetTotalMaxHealth(), ownerBodyParts->GetTotalHealthPercent() * 100.0f);
		ownerBodyParts->PrintPartsDebug();
	}
}

void UCharacterDebugComponent::InvHurt(int32 part, float amount)
{
	if (ownerBodyParts)
		ownerBodyParts->ApplyFlatDamage((EBodyPart)part, amount);
}

void UCharacterDebugComponent::InvInjure(int32 part, int32 injury)
{
	if (!ownerBodyParts)
		return;

	const EBodyPart targetPart = (EBodyPart)part;
	const EInjuryType targetInjury = (EInjuryType)injury;

	if (targetInjury == EInjuryType::EIT_Bleeding)
	{
		ownerBodyParts->TriggerBleeding(targetPart);
		return;
	}

	ownerBodyParts->TriggerInjury(targetPart, targetInjury);
}

void UCharacterDebugComponent::InvSaveZone(bool bActive)
{
	if (ownerBodyParts)
		ownerBodyParts->SetSaveZoneActive(bActive);
}

void UCharacterDebugComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* owner = GetOwner())
	{
		ownerAttributes = owner->FindComponentByClass<UAttributeComponent>();
		ownerBodyParts = owner->FindComponentByClass<UBodyPartComponent>();
		ownerInventory = owner->FindComponentByClass<UInventoryComponent>();
	}
}

void UCharacterDebugComponent::DebugRecoverChaos()
{
	if (ownerAttributes)
		ownerAttributes->AddChaos(-chaosDebugStep);
}

void UCharacterDebugComponent::DebugLoseChaos()
{
	if (ownerAttributes)
		ownerAttributes->AddChaos(+chaosDebugStep);
}

void UCharacterDebugComponent::DebugDamageSelectedPart()
{
	if (ownerBodyParts)
		ownerBodyParts->ApplyDamageToPart((EBodyPart)selectedPart, partDamageStep);
}

void UCharacterDebugComponent::DebugCyclePart()
{
	selectedPart++;
	if (selectedPart > (int32)EBodyPart::EBP_RightLeg)
		selectedPart = (int32)EBodyPart::EBP_Head;

	static const TCHAR* names[] = { TEXT(""), TEXT("머리"), TEXT("가슴"), TEXT("왼팔"), TEXT("오른팔"), TEXT("왼다리"), TEXT("오른다리") };
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(50, 3.f, FColor::White,
			FString::Printf(TEXT("[디버그] 선택 부위: %s"), names[selectedPart]));
	}
}

void UCharacterDebugComponent::DebugHealAllParts()
{
	if (ownerBodyParts)
		ownerBodyParts->HealAllParts();
}