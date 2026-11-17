#pragma once

#include "CoreMinimal.h"
#include "Enemy/AI/EnemyAITypes.h"
#include "Enemy/AI/EnemyCardModRows.h"

class UDataTable;
struct FEnemyAttackRow;
using FCardBaseScores = TMap<EEnemyCard, float>;

/* 거리 구간 경계 */
struct FCardBandBounds
{
	float nearMax = 2.5f;    // 가까이 상한
	float midMax = 6.f;      // 중간 상한
	float farMax = 0.f;      // 멀리 상한
	float hysteresis = 0.5f; // 완충
};

/* 일반 공격 1장 */
struct FCardAttackOption
{
	FName attackId = NAME_None;                    // 공격 ID
	EEnemyCard card = EEnemyCard::EC_LightAttack;  // 공격 카드
	float shareWeight = 1.f;                       // 공유 비율
};

/* 특수 카드 1장 */
struct FCardSpecialOption
{
	FName attackId = NAME_None;                             // 공격 ID
	float score = 0.f;                                      // 구간 점수
	EEnemyCardBundle bundle = EEnemyCardBundle::ECB_Attack; // 카드 묶음
};

/* 기본 점수 입력 */
struct FCardBaseInput
{
	EEnemyCombatRole role = EEnemyCombatRole::ECR_Melee;           // 카드 표
	EEnemyRangeBand band = EEnemyRangeBand::ERB_Near;              // 거리 구간
	const FCardBaseScores* baseScores = nullptr;                   // 기본 카드 점수
	const TArray<FEnemyCardScoreOverride>* overrides = nullptr;    // 원형 카드 점수 덮어쓰기
	TArray<FCardAttackOption> attacks;                             // 일반 공격
	TArray<FCardSpecialOption> specials;                           // 특수 카드
};

/* 점수표 */
struct FCardEntry
{
	EEnemyCard card = EEnemyCard::EC_Rest;                 // 카드
	FName attackId = NAME_None;                            // 공격 ID
	EEnemyCardBundle bundle = EEnemyCardBundle::ECB_Probe; // 카드 묶음
	bool bSpecial = false;                                 // 특수 카드 여부
	float baseScore = 0.f;                                 // 기본 점수
	float modifierScale = 1.f;          // 보정 배율
	bool bModifierCapped = false;       // 상한에 걸렸는지 여부
	float modifiedScore = 0.f;          // 보정 점수
	bool bRepeatSuppressed = false;     // 반복 억제 적용 여부
	const TCHAR* blockReason = nullptr; // 차단 이유 
	float finalScore = 0.f;             // 최종 점수
};

/* 카드 보정 정보 */
struct FCardModifier
{
	EEnemyCard card = EEnemyCard::EC_Rest;                  // 대상 카드
	bool bWholeBundle = false;                              // 카드 묶음 전체에 거는지 여부
	EEnemyCardBundle bundle = EEnemyCardBundle::ECB_Attack; // 대상 카드 묶음
	float multiplier = 1.f;                                 // 배율
	const TCHAR* source = TEXT("");                         // 디버그용 출처

	bool Matches(const FCardEntry& entry) const { return bWholeBundle ? entry.bundle == bundle : entry.card == card; }
};

/* 직전에 고른 카드 정보 */
struct FCardLastPick
{
	bool bValid = false;
	EEnemyCard card = EEnemyCard::EC_Rest;
	FName attackId = NAME_None; // 공격 ID

	bool IsSame(const FCardEntry& entry) const
	{
		if (!bValid || entry.card != card)
			return false;
		return card != EEnemyCard::EC_Special || entry.attackId == attackId;
	}
};

/* 차단 및 조절 입력 */
struct FCardBlockInput
{
	FCardLastPick lastPick;          // 직전에 고른 카드
	float repeatSuppression = 0.3f;  // 반복 억제 배율
	bool bRestTime = false;          // 쉬는 시간 여부
	bool bPlayerStaggered = false;   // 플레이어 경직 여부
	bool bLoneSurvivor = false;      // 혼자 남은 적 여부
	bool bFrenzy = false;            // 광증 여부
};

/* 동시 공격자 비율 입력  */
struct FCardRatioInput
{
	int32 otherAttackers = 0;   // 다른 공격 몬스터 수
	bool bPackAttacker = false; // 무리 공격형 여부
};

/* 플레이어 상태에 따라 적이 배율이 걸리는 조건  */
struct FPlayerStateRules
{
	float frontHalfAngle = 60.f;  // 정면 반각
	float counterRange = 3.f;     // 카운터 사거리
	float itemPunishRange = 3.5f; // 회복 응징 거리
	float aimRange = 8.f;         // 활 조준 반응 거리
	float guardHoldTime = 2.f;    // 가드 유지 기준 시간
};

/* 플레이어 상태 판정 입력 */
struct FPlayerStateQuery
{
	FPlayerAISnapshot player; // 현재 플레이어 스냅샷
	float distance = 0.f; // 수평 거리
	float angleFromFront = 0.f; // 몬스터 정면과 플레이어 방향 사이 각도
	float attackReach = 0.f; // 몬스터의 가장 긴 공격 사거리
	EEnemyCombatRole role = EEnemyCombatRole::ECR_Melee; // 카드 표
	bool bCounterReaction = true; // 카운터 외형 여부
};

/* 카드 점수 순수 함수 */
namespace EnemyCardScore
{
	constexpr float MaxModifierProduct = 5.f; // 상황 보정 곱 상한
	constexpr float RatioTwoAttackers = 0.30f; // 다른 공격자 2명(공격 묶음 30%)
	constexpr float RatioManyAttackers = 0.20f; // 다른 공격자 3명 이상(공격 묶음 20%)

	WINDLESS_API EEnemyRangeBand ResolveBand(float distance, EEnemyRangeBand prevBand, const FCardBandBounds& bounds); // 거리 구간을 정하는 함수
	WINDLESS_API const TArray<EEnemyCard>& GetRoleCards(EEnemyCombatRole role); // 역할의 공통 카드 목록을 반환하는 함수
	WINDLESS_API bool IsAttackCard(EEnemyCard card); // 공격 카드인지 확인하는 함수
	WINDLESS_API EEnemyCardBundle GetCardBundle(EEnemyCard card); // 카드의 묶음을 반환하는 함수
	WINDLESS_API FCardBaseScores CollectBaseScores(const UDataTable* table, EEnemyCombatRole role, EEnemyRangeBand band, EEnemyTemperament temperament); // 구간 점수를 뽑는 함수
	WINDLESS_API TArray<FCardEntry> BuildBaseScores(const FCardBaseInput& input); // 기본 점수를 만드는 함수
	WINDLESS_API float GetBundleRatio(const TArray<FCardEntry>& entries, EEnemyCardBundle bundle); // 묶음의 점수 비율을 반환하는 함수
	WINDLESS_API bool IsAttackInReach(const FEnemyAttackRow& attack, float distance, float heightDelta); // 공격 사거리 및 수직 발동 각도를 검사하는 함수
	WINDLESS_API const TCHAR* GetCardName(EEnemyCard card); // 카드 이름을 반환하는 함수
	WINDLESS_API const TCHAR* GetBandName(EEnemyRangeBand band); // 거리 구간 이름을 반환하는 함수
	WINDLESS_API void ApplyModifiers(TArray<FCardEntry>& entries, const TArray<FCardModifier>& modifiers, float maxProduct = MaxModifierProduct); // 상황 보정을 적용하는 함수
	WINDLESS_API void ApplyBlocks(TArray<FCardEntry>& entries, const FCardBlockInput& block); // 차단 및 조절을 적용하는 함수
	WINDLESS_API float GetAttackBundleShare(const FCardRatioInput& ratio); // 공격 묶음 목표 비율을 반환하는 함수
	WINDLESS_API TArray<float> GetPickChances(const TArray<FCardEntry>& entries, const FCardRatioInput& ratio); // 비율까지 적용한 카드별 확률을 반환하는 함수
	WINDLESS_API int32 PickEntry(const TArray<FCardEntry>& entries, const FCardRatioInput& ratio, FRandomStream& random); // 카드를 하나를 뽑는 함수
	WINDLESS_API TArray<EPlayerAIState> GetActivePlayerStates(const FPlayerStateQuery& query, const FPlayerStateRules& rules = FPlayerStateRules()); // 적을 활성화시키는 플레이어 상태를 고르는 함수
	WINDLESS_API void CollectPlayerStateMods(const UDataTable* table, EEnemyCombatRole role, const TArray<EPlayerAIState>& states, TArray<FCardModifier>& outModifiers); // 상태 배율을 모으는 함수
	WINDLESS_API const TCHAR* GetPlayerStateName(EPlayerAIState state); // 플레이어 상태 이름을 반환하는 함수
	WINDLESS_API void CollectLastActMods(const UDataTable* table, EEnemyLastAct act, TArray<FCardModifier>& outModifiers); // 직전 행동 보정을 모으는 함수
	WINDLESS_API const TCHAR* GetLastActName(EEnemyLastAct act); // 직전 행동 이름을 반환하는 함수
}
