#include "Components/WeaponLoadoutComponent.h"
#include "Components/InventoryComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Items/Weapons/Weapon.h"
#include "Windless/DebugMacro.h"

UWeaponLoadoutComponent::UWeaponLoadoutComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponLoadoutComponent::InitializeLoadout()
{
	character = Cast<AWindlessCharacter>(GetOwner());
	inventory = character ? character->GetInventory() : nullptr;
	if (!character || !inventory)
		return;

	inventory->OnEquipmentChanged.RemoveDynamic(this, &UWeaponLoadoutComponent::HandleEquipmentChanged);
	inventory->OnEquipmentChanged.AddDynamic(this, &UWeaponLoadoutComponent::HandleEquipmentChanged);

	if (bGrantStartingHandItems)
		GrantStartingHandItems();

	SyncHandWeapons();
}

void UWeaponLoadoutComponent::SyncHandWeapons()
{
	SyncHand(EEquipSlot::EES_LeftHand);
	SyncHand(EEquipSlot::EES_RightHand);
}

void UWeaponLoadoutComponent::PrintLoadoutDebug() const
{
	auto logHand = [](const TCHAR* label, const AWeapon* weapon, int64 uid)
		{
			UE_LOG(LogWindless, Warning, TEXT("[로드아웃] %s: %s (아이템 %s, UID %lld)"),
				label, weapon ? *weapon->GetName() : TEXT("없음"),
				weapon ? *weapon->GetSourceItemId().ToString() : TEXT("-"), uid);
		};

	logHand(TEXT("왼손"), leftWeapon, leftInstanceUID);
	logHand(TEXT("오른손"), rightWeapon, rightInstanceUID);
}

void UWeaponLoadoutComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (leftWeapon)
		leftWeapon->Destroy();
	if (rightWeapon)
		rightWeapon->Destroy();

	Super::EndPlay(EndPlayReason);
}

void UWeaponLoadoutComponent::HandleEquipmentChanged()
{
	SyncHandWeapons();
}

void UWeaponLoadoutComponent::SyncHand(EEquipSlot slot)
{
	if (!inventory || !character)
		return;

	const int64 equippedUID = inventory->GetEquippedInstanceUID(slot);
	int64& cachedUID = (slot == EEquipSlot::EES_LeftHand) ? leftInstanceUID : rightInstanceUID;

	if (equippedUID == cachedUID && (equippedUID == 0 || GetHandWeapon(slot)))
		return;

	DestroyHandWeapon(slot);
	cachedUID = 0;

	if (equippedUID == 0)
		return;

	const FItemInstance instance = inventory->FindInstance(equippedUID);
	FItemData data;
	if (!inventory->GetItemData(instance.itemId, data))
		return;

	AWeapon* weapon = SpawnHandWeapon(data, slot);
	if (!weapon)
		return;

	if (slot == EEquipSlot::EES_LeftHand)
		leftWeapon = weapon;
	else
		rightWeapon = weapon;

	cachedUID = equippedUID;
	character->SetHandWeapon(slot, weapon);

	UE_LOG(LogWindless, Log, TEXT("[로드아웃] %s <- %s (%s, 가드수치 %.0f)"),
		slot == EEquipSlot::EES_LeftHand ? TEXT("왼손") : TEXT("오른손"),
		*data.itemId.ToString(), *UEnum::GetValueAsString(data.weaponType), data.guardValueBonus);
}

AWeapon* UWeaponLoadoutComponent::SpawnHandWeapon(const FItemData& data, EEquipSlot slot)
{
	UWorld* world = GetWorld();
	const TSubclassOf<AWeapon> weaponClass = data.weaponActorClass.LoadSynchronous();
	if (!world || !weaponClass)
		return nullptr;

	FActorSpawnParameters params;
	params.Owner = character;
	params.Instigator = character;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeapon* weapon = world->SpawnActor<AWeapon>(weaponClass, character->GetActorTransform(), params);
	if (!weapon)
		return nullptr;

	weapon->ApplyItemData(data);

	const FName socketName = weapon->GetHandSocketOverride().IsNone()
		? GetSocketForSlot(slot) : weapon->GetHandSocketOverride();
	weapon->Equip(character->GetMesh(), socketName, character, character);
	return weapon;
}

FName UWeaponLoadoutComponent::GetSocketForSlot(EEquipSlot slot) const
{
	return slot == EEquipSlot::EES_LeftHand ? leftHandSocket : rightHandSocket;
}

void UWeaponLoadoutComponent::DestroyHandWeapon(EEquipSlot slot)
{
	AWeapon*& weapon = (slot == EEquipSlot::EES_LeftHand) ? leftWeapon : rightWeapon;
	if (!weapon)
		return;

	if (character)
		character->SetHandWeapon(slot, nullptr);

	weapon->Destroy();
	weapon = nullptr;
}

void UWeaponLoadoutComponent::GrantStartingHandItems()
{
	for (const FStartingHandItem& entry : startingHandItems)
	{
		if (entry.itemId.IsNone() || inventory->GetEquippedInstanceUID(entry.slot) != 0)
			continue;

		if (!inventory->AddItemById(entry.itemId, 1))
			continue;

		const int64 uid = inventory->FindFirstUnequippedUID(entry.itemId);
		if (uid != 0)
			inventory->EquipItem(uid, entry.slot);
	}
}