#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Enemy/AI/EnemyAITypes.h"
#include "EnemyCardModRows.generated.h"

/* 몬스터가 읽는 플레이어 상태*/
UENUM(BlueprintType)
enum class EPlayerAIState : uint8
{
	EPS_AttackBegin UMETA(DisplayName = "AttackBegin"), // 휘두르기 시작
	EPS_Whiff       UMETA(DisplayName = "Whiff"),       // 헛방 후딜
	EPS_ItemUse     UMETA(DisplayName = "ItemUse"),     // 회복이나 아이템 사용
	EPS_BowAim      UMETA(DisplayName = "BowAim"),      // 활 조준
	EPS_GuardHold   UMETA(DisplayName = "GuardHold"),   // 가드 2초 이상 유지
	EPS_Behind      UMETA(DisplayName = "Behind")       // 뒤에서 접근
};

USTRUCT(BlueprintType)
struct WINDLESS_API FEnemyPlayerStateModRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EPlayerAIState state = EPlayerAIState::EPS_AttackBegin; // 플레이어 상태

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyCombatRole role = EEnemyCombatRole::ECR_Melee; // 전투 역할

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyCard card = EEnemyCard::EC_LightAttack; // 대상 카드

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card", meta = (ClampMin = "0.0"))
	float multiplier = 1.f; // 배율
};

/* 직전 행동 */
UENUM(BlueprintType)
enum class EEnemyLastAct : uint8
{
	ELA_LightHit            UMETA(DisplayName = "LightHit"),			// 약공격 적중
	ELA_HeavyHit            UMETA(DisplayName = "HeavyHit"),            // 강공격 적중
	ELA_ComboEnd            UMETA(DisplayName = "ComboEnd"),            // 연타 종료
	ELA_Whiff               UMETA(DisplayName = "Whiff"),               // 공격 빗나감
	ELA_Endured             UMETA(DisplayName = "Endured"),             // 피격 버팀
	ELA_GuardSuccess        UMETA(DisplayName = "GuardSuccess"),        // 가드 성공
	ELA_DodgeSuccess        UMETA(DisplayName = "DodgeSuccess"),        // 반응 회피 성공
	ELA_GuardBreakRecovered UMETA(DisplayName = "GuardBreakRecovered"), // 가드 브레이크 회복 직후
	ELA_StaggerRecovered    UMETA(DisplayName = "StaggerRecovered"),    // 경직 회복 직후
	ELA_ConvulsionEnded     UMETA(DisplayName = "ConvulsionEnded"),     // 경련 완료 직후
	ELA_ComboCancelChase    UMETA(DisplayName = "ComboCancelChase"),    // 연타 취소(공격적)
	ELA_ComboCancelWatch    UMETA(DisplayName = "ComboCancelWatch")     // 연타 취소(차분, 기본)
};

/* 직전 행동 보정 */
USTRUCT(BlueprintType)
struct WINDLESS_API FEnemyLastActModRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyLastAct act = EEnemyLastAct::ELA_LightHit; // 직전 행동

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyCard card = EEnemyCard::EC_Rest; // 대상 카드

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	bool bWholeBundle = false; // 전체 묶음 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card")
	EEnemyCardBundle bundle = EEnemyCardBundle::ECB_Attack; // 대상 카드 묶음

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Card", meta = (ClampMin = "0.0"))
	float multiplier = 1.f; // 배율
};

/* 현재 플레이어 스냅샷 */
struct FPlayerAISnapshot
{
	bool bAttackBegin = false; // 공격 시작 여부
	bool bWhiff = false;       // 헛방 후딜 여부
	bool bUsingItem = false;   // 아이템 사용 여부
	bool bAiming = false;      // 활 조준 여부
	float guardHoldTime = 0.f; // 현재 가드 유지 시간
};