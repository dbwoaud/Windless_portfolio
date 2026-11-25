#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemTypes.h"
#include "WeaponLoadoutComponent.generated.h"

class AWindlessCharacter;
class UInventoryComponent;
class AWeapon;

/* 시작 손 장비 */
USTRUCT(BlueprintType)
struct FStartingHandItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Loadout")
	FName itemId = NAME_None; // 지급할 아이템 ID

	UPROPERTY(EditAnywhere, Category = "Loadout")
	EEquipSlot slot = EEquipSlot::EES_RightHand; // 장착할 손 슬롯
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UWeaponLoadoutComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponLoadoutComponent();


	/* 초기화 함수 */
	void InitializeLoadout(); // 무기 슬롯과 캐릭터가 실제 들고 있는 무기를 동기화하는 함수
	void SyncHandWeapons(); // 양손 슬롯을 인벤토리 상태에 맞추는 함수


	/* 디버그용 */
	void PrintLoadoutDebug() const; // 손 무기 현황을 출력하는 함수(디버그용)

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/* 캐시 변수 */
	UPROPERTY()
	AWindlessCharacter* character = nullptr; // 소유 캐릭터

	UPROPERTY()
	UInventoryComponent* inventory = nullptr; // 소유 캐릭터의 인벤토리

	UPROPERTY()
	AWeapon* leftWeapon = nullptr; // 왼손 무기 액터

	UPROPERTY()
	AWeapon* rightWeapon = nullptr; // 오른손 무기 액터

	int64 leftInstanceUID = 0; // 왼손 무기를 만든 아이템 인스턴스
	int64 rightInstanceUID = 0; // 오른손 무기를 만든 아이템 인스턴스


	/* 무기 장착 및 해제 */
	UPROPERTY(EditDefaultsOnly, Category = "Loadout|Socket")
	FName leftHandSocket = FName("LeftHandSocket"); // 왼손 소켓

	UPROPERTY(EditDefaultsOnly, Category = "Loadout|Socket")
	FName rightHandSocket = FName("RightHandSocket"); // 오른손 소켓

	UFUNCTION()
	void HandleEquipmentChanged(); // 인벤토리 장착 변경 시 실행되는 함수
	void SyncHand(EEquipSlot slot); // 한 손 슬롯을 동기화하는 함수
	AWeapon* SpawnHandWeapon(const FItemData& data, EEquipSlot slot); // 아이템 데이터로 무기 액터를 소환/부착하는 함수
	FName GetSocketForSlot(EEquipSlot slot) const; // 손 슬롯에 맞는 소켓 이름을 반환하는 함수
	void DestroyHandWeapon(EEquipSlot slot); // 손에 든 무기 액터를 제거하는 함수


	/* 디버그용 */
	UPROPERTY(EditAnywhere, Category = "Loadout|Starting")
	bool bGrantStartingHandItems = false; // 시작 시 무기 장비 지급 여부

	UPROPERTY(EditAnywhere, Category = "Loadout|Starting", meta = (EditCondition = "bGrantStartingHandItems"))
	TArray<FStartingHandItem> startingHandItems; // 시작 무기 장비 목록

	void GrantStartingHandItems(); // 시작 무기 장비를 지급/장착하는 함수

public:
	/* 변수 Getter 함수 */
	FORCEINLINE AWeapon* GetHandWeapon(EEquipSlot slot) const { return slot == EEquipSlot::EES_LeftHand ? leftWeapon : rightWeapon; }
};