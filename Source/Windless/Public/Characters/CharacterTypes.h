#pragma once

#include "CoreMinimal.h"
#include "CharacterTypes.generated.h"

/* 플레이어 캐릭터 장비 장착 상태 열거형 */
UENUM(BlueprintType)
enum class ECharacterState : uint8 
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One-Handed Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped Two-Handed Weapon")
};

/* 플레이어 캐릭터 상태 열거형 */
UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_HitReaction UMETA(DisplayName = "HitReaction"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_Blocking UMETA(DisplayName = "Blocking"),
	EAS_EquippingWeapon UMETA(DisplayName = "EquippingWeapon"),
	EAS_Dodge UMETA(DisplayName = "Dodge"),
	EAS_Mounting UMETA(DisplayName = "Mounting"),
	EAS_Dismounting UMETA(DisplayName = "Dismounting"),
	EAS_UsingItem UMETA(DisplayName = "UsingItem"),
	EAS_Dead UMETA(DisplayName = "Dead"),
	EAS_Aiming UMETA(DisplayName = "Aiming"),
	EAS_Knockdown UMETA(DisplayName = "Knockdown")
};

/* 플레이어 캐릭터 이동 상태 열거형 */
UENUM(BlueprintType)
enum class EMovementAction : uint8
{
	EMA_None UMETA(DisplayName = "None"),
	EMA_Jump UMETA(DisplayName = "Jump"),
	EMA_Dodge UMETA(DisplayName = "Dodge"),
	EMA_Crouch UMETA(DisplayName = "Crouch")
};

/* 캐릭터 죽음 포즈 열거형 */
UENUM(BlueprintType)
enum EDeathPose
{
	EDP_Death1 UMETA(DisplayName = "Death1"),
	EDP_Death2 UMETA(DisplayName = "Death2"),
	EDP_Death3 UMETA(DisplayName = "Death3"),
	EDP_Death4 UMETA(DisplayName = "Death4"),
	EDP_Death5 UMETA(DisplayName = "Death5"),
	EDP_Death6 UMETA(DisplayName = "Death6"),

	EDP_MAX UMETA(DisplayName = "DefaultMAX")
};

/* 적 상태 열거형 */
UENUM(BlueprintType)
enum class EEnemyState : uint8 
{
	EES_NoState UMETA(DisplayName = "NoState"),
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_Attacking UMETA(DisplayName = "Attacking"),
	EES_Engaged UMETA(DisplayName = "Engaged"),
	EES_Idle UMETA(DisplayName = "Idle"),
	EES_Searching UMETA(DisplayName = "Searching"),
	EES_Returning UMETA(DisplayName = "Returning"),
	EES_Staggered UMETA(DisplayName = "Staggered")
};

/* 적 이동 방식 */
UENUM(BlueprintType)
enum class EEnemyMoveType : uint8
{
	EMT_Wander UMETA(DisplayName = "Wander"), // 배회형(배치 위치 반경 내 무작위 이동)
	EMT_Patrol UMETA(DisplayName = "Patrol"), // 순찰형(정해진 경로 순서대로 이동)
	EMT_Fixed  UMETA(DisplayName = "Fixed")   // 고정형(제자리 공격)
};