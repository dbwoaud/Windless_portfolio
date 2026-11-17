#pragma once

#include "CoreMinimal.h"
#include "Characters/BodyPartTypes.h"
#include "Items/Weapons/WeaponType.h" 
#include "Items/QuickSlotTypes.h"
#include "ItemTypes.generated.h"

class UTexture2D;
class UStaticMesh;
class AWeapon;

/* 아이템 대분류 */
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    EIC_None       UMETA(DisplayName = "None"),
    EIC_Equipment  UMETA(DisplayName = "Equipment"), // 장비 아이템
    EIC_Consumable UMETA(DisplayName = "Consumable"), // 소모성 아이템
    EIC_Material   UMETA(DisplayName = "Material"), // 제작 재료 아이템
    EIC_Etc        UMETA(DisplayName = "Etc"), // 기타 아이템
    EIC_Currency   UMETA(DisplayName = "Currency") // 재화 아이템
};

/* 아이템 등급 */
UENUM(BlueprintType)
enum class EItemGrade : uint8
{
    EIG_Common    UMETA(DisplayName = "Common"), // 일반(연두)
    EIG_Uncommon  UMETA(DisplayName = "Uncommon"), // 고급(연하늘)
    EIG_Rare      UMETA(DisplayName = "Rare"), // 희귀(연파랑)
    EIG_Unique      UMETA(DisplayName = "Unique"), // 유일(연보라)
    EIG_Legendary UMETA(DisplayName = "Legendary") // 전설(연주황)
};

inline FColor GetItemGradeColor(EItemGrade grade)
{
    switch (grade)
    {
        case EItemGrade::EIG_Uncommon:  return FColor(219, 238, 239); // 고급(연하늘)
        case EItemGrade::EIG_Rare:      return FColor(221, 235, 255); // 희귀(연파랑)
        case EItemGrade::EIG_Unique:    return FColor(230, 207, 242); // 유일(연보라)
        case EItemGrade::EIG_Legendary: return FColor(255, 200, 170); // 전설(연주황)
        case EItemGrade::EIG_Common: // 일반(연두)
        default:                        return FColor(220, 233, 203);
    }
}

/* 장비 장착 부위 */
UENUM(BlueprintType)
enum class EEquipSlot : uint8
{
    EES_None      UMETA(DisplayName = "None"), // 0 미장착
    EES_Head      UMETA(DisplayName = "Head"), // 1 머리
    EES_Chest     UMETA(DisplayName = "Chest"), // 2 가슴
    EES_LeftArm   UMETA(DisplayName = "LeftArm"), // 3 왼팔
    EES_RightArm  UMETA(DisplayName = "RightArm"), // 4 오른팔
    EES_LeftLeg   UMETA(DisplayName = "LeftLeg"), // 5 왼다리
    EES_RightLeg  UMETA(DisplayName = "RightLeg"), // 6 오른다리
    EES_LeftHand  UMETA(DisplayName = "LeftHand"), // 7 왼손(무기)
    EES_RightHand UMETA(DisplayName = "RightHand"), // 8 오른손(무기)
    EES_MAX       UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EItemEffect : uint8
{
    EIE_None          UMETA(DisplayName = "None"),
    EIE_HealHP        UMETA(DisplayName = "HealHP"), // 체력 회복
    EIE_RevivePart    UMETA(DisplayName = "RevivePart"), // 파괴 부위 부활
    EIE_RecoverChaos  UMETA(DisplayName = "RecoverChaos"), // 혼돈 게이지 회복
    EIE_BodyHeat      UMETA(DisplayName = "BodyHeat"), // 체온 상승
    EIE_CureInjury    UMETA(DisplayName = "CureInjury"), // 부상 치료
    EIE_Throw         UMETA(DisplayName = "Throw"), // 투척물 발사
    EIE_WeaponEnchant UMETA(DisplayName = "WeaponEnchant"), // 무기 인챈트
    EIE_SpawnObject   UMETA(DisplayName = "SpawnObject"), // 오브젝트 소환
    EIE_Detect        UMETA(DisplayName = "Detect"), // 적 탐지
    EIE_GiveMoney     UMETA(DisplayName = "GiveMoney"), // 재화 지급
    EIE_RepairTool    UMETA(DisplayName = "RepairTool"), // 수리 도구
    EIE_Ammo          UMETA(DisplayName = "Ammo") // 탄약
};


/* 아이템 데이터 */
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
    GENERATED_BODY()

    /* 공통 설정 값 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common")
    FName itemId = NAME_None; // 아이템의 ID

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common")
    FText itemName; // 아이템 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common", meta = (MultiLine = "true"))
    FText itemDescription; // 아이템 설명

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common")
    TObjectPtr<UTexture2D> icon = nullptr; // 아이템 UI 아이콘

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common")
    EItemCategory category = EItemCategory::EIC_None; // 대분류

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common")
    int32 subType = 0; // 소분류

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common")
    EItemGrade grade = EItemGrade::EIG_Common; // 등급

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common")
    EEquipSlot equipSlot = EEquipSlot::EES_None; // 기본 장착 슬롯

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common")
    bool bDroppable = true; // 판매/버리기 가능 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common")
    TSoftObjectPtr<UStaticMesh> fieldMesh; // 필드 표시용 정적 메시

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Common", meta = (ClampMin = "0"))
    int32 carryLimit = 0; // 최대 소지 개수


    /* 그리드 크기 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Grid", meta = (ClampMin = "1"))
    int32 width = 1; // 인벤토리 가로 점유 칸

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Grid", meta = (ClampMin = "1"))
    int32 height = 1; // 인벤토리 세로 점유 칸


    /* 중첩 설정 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stack")
    bool bStackable = false; // 중첩 가능 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stack")
    int32 maxStack = 1; // 최대 겹침 한도


    /* 장비 전용 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment")
    float attackPowerBonus = 0.f; // 공격력 보정치

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment")
    float maxLimbHPBonus = 0.f; // 생명력 보정치

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment")
    float maxDurability = 0.f; // 최대 내구도(0이면 내구도 없음)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment|Weapon")
    TSoftClassPtr<AWeapon> weaponActorClass; // 장착 중인 무기 액터 클래스

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment|Weapon")
    EWeaponType weaponType = EWeaponType::EWT_OneHanded; // 무기 종류

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Equipment|Weapon", meta = (ClampMin = "0.0"))
    float guardValueBonus = 0.f; // 가드수치 보정치


    /* 소모성 전용 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    EItemEffect effectType = EItemEffect::EIE_None; // 효과 종류

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    float effectAmount = 0.f; // 효과량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    float effectDuration = 0.f; // 효과 지속시간

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    float useTime = 0.f; // 시전 시간

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    float cooldown = 0.f; // 재사용 대기시간

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    FName effectPayload = NAME_None; // 투척물/인챈트/소환 오브젝트 ID 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    bool bChanneled = false; // 사용 중지까지 지속되는 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    FName useAnim = NAME_None; // 사용 애니메이션 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    float moveSpeedRateOnUse = 1.f; // 사용 중 이동속도 배율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    int32 conditionCode = 0; // 사용 조건 검증값

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    bool bConsumedOnUse = true; // 사용 후 소모 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    bool bRequiresBodyPart = false; // 부위 지정 필요 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable")
    EInjuryType targetInjury = EInjuryType::EIT_None; // 부상 치유 아이템이 치료하는 부상 종류

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Consumable", meta = (ClampMin = "0"))
    int32 maxEquipCount = 0; // 소모품 슬롯에 장착 가능한 최대 중첩 수량

    /* 아이템 ID 유효성 확인 함수 */
    bool IsValid() const { return itemId != NAME_None; }
};

USTRUCT(BlueprintType)
struct FItemInstance
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    int64 instanceUID = 0; // 개체 고유 번호

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    FName itemId = NAME_None; // 원본 아이템 ID

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    int32 gridPosX = 0; // 그리드 X좌표(좌측 상단 기준)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    int32 gridPosY = 0; // 그리드 Y좌표(좌측 상단 기준)

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    bool bIsRotated = false; // 회전 여부

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    int32 currentStackCount = 1; // 현재 수량

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    float currentDurability = 0.f; // 현재 내구도

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    bool bIsEquipped = false; // 장착 여부

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    EEquipSlot equipSlot = EEquipSlot::EES_None; // 장착 슬롯

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Instance")
    EQuickSlot quickSlot = EQuickSlot::EQS_None; // 장착 퀵슬롯

    /* 인스턴스 유효성 확인 함수 */
    bool IsValid() const { return instanceUID != 0 && itemId != NAME_None; }
};