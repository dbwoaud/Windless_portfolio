#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/AI/EnemyCardScore.h"
#include "EnemyActionSelector.generated.h"

struct FMonsterArchetypeRow;
struct FMonsterProfile;
struct FEnemyAttackRow;
class UDataTable;
class UEnemyAttackRunner;

/* 카드 문맥 */
struct FEnemyCardContext
{
	float distance = 0.f;           // 대상까지 수평 거리
	float heightDelta = 0.f;        // 대상 높이 - 내 높이
	float angleFromFront = 0.f;     // 내 정면과 대상 방향 사이 각도
	FPlayerAISnapshot player;       // 플레이어가 하는 일
	bool bPlayerStaggered = false;  // 플레이어 경직 여부
	int32 otherAttackers = 0;       // 다른 공격자 수
	bool bLoneSurvivor = false;     // 적 혼자 여부
	bool bFrenzy = false;           // 광증 여부
	bool bSideStepBlocked = false;  // 옆걸음 불가 여부
	bool bBackOffBlocked = false;   // 물러서기 불가 여부
};

/* 고른 결과 */
struct FEnemyCardChoice
{
	EEnemyCard card = EEnemyCard::EC_Rest; // 고른 카드
	FName attackId = NAME_None;            // 공격 ID
	int32 attackIndex = INDEX_NONE;        // 원형 공격 목록의 인덱스
	bool bFallbackRest = false;            // 숨 고르기 여부
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UEnemyActionSelector : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyActionSelector();

	/* 초기화 함수 */
	void Setup(const FMonsterArchetypeRow& inArchetype, const FMonsterProfile& inProfile, const TArray<const FEnemyAttackRow*>& inAttacks, const TArray<FName>& inAttackIds); // 카드 설정기를 초기화하는 함수
	FEnemyCardChoice Select(const FEnemyCardContext& context); // 카드를 선택하는 함수


	/* 카드 설정 */
	void AddNextModifiers(const TArray<FCardModifier>& modifiers); // 다음 선택의 보정을 추가하는 함수
	void RecordLastAct(EEnemyLastAct act); // 직전 행동을 기록하는 함수


	/* 공격 */
	void NotifyAttackCompleted(); // 후딜 완료 시 실행되는 함수
	void NotifyCardStarted(EEnemyCard card); // 비공격 카드가 시작될 때 쿨다운을 시작하는 함수
	void ResetCombatMemory(); // 전투를 벗어날 때 직전 행동을 보정하고 반복 억제 기록을 지우는 함수


	/* 디버그용 */
	void ForceNextCard(EEnemyCard card); // 다음 카드 선택을 강제하는 함수


	/*플레이어 상태 */
	TArray<EPlayerAIState> GetActivePlayerStates(const FEnemyCardContext& context) const; // 활성화된 플레이어 AI 상태를 반환하는 함수

protected:
	virtual void BeginPlay() override;

private:
	/* 카드 설정 */
	UPROPERTY(EditAnywhere, Category = "AI|Card")
	int32 randomSeed = 0; // 무작위 시드

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> cardTable = nullptr; // 카드 기본 점수 테이블

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> playerStateTable = nullptr; // 플레이어 상태 배율 테이블

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> lastActTable = nullptr; // 직전 행동 보정 테이블


	/* 공격 */
	void HandleAttackerChanged(UEnemyAttackRunner* runner, bool bRegistered); // 공격 적 변경을 처리하는 함수
	void BuildCandidates(const FEnemyCardContext& context, FCardBaseInput& outInput) const; // 후보를 설정하는 함수


	/* 조회 */
	const FEnemyAttackRow* FindAttack(FName attackId) const; // 공격 ID로 공격 데이터를 찾는 함수
	bool IsSpecialAttack(FName attackId) const; // 특수 카드 공격인지 확인하는 함수
	bool IsAttackReady(FName attackId) const; // 쿨다운이 끝났는지 확인하는 함수
	float GetNow() const; // 현재 게임 시간을 반환하는 함수


	/* 적 원형 */
	const FMonsterArchetypeRow* archetype = nullptr; // 적 원형 데이터
	TArray<const FEnemyAttackRow*> attacks; // 적 공격 데이터
	TArray<FName> attackIds; // 공격 ID
	EEnemyCombatRole role = EEnemyCombatRole::ECR_Melee; // 적 전투 역할
	EEnemyTemperament temperament = EEnemyTemperament::ETMP_Normal; // 적 원형 겹
	FCardBandBounds bounds; // 거리 구간 경계
	bool bPackAttacker = false; // 무리 공격형 여부
	bool bFixedMover = false; // 고정형 여부


	/* 플레이어 상태 */
	float attackReach = 0.f;       // 가장 긴 공격 사거리
	bool bCounterReaction = true;  // 공격에 대한 카운터 반응 영부
	FPlayerStateRules playerRules; // 플레이어 상태 조건


	/* 런타임 */
	EEnemyRangeBand currentBand = EEnemyRangeBand::ERB_Near; // 현재 거리 구간
	TMap<FName, float> attackReadyTime; // 공격 준비 시간
	float restUntil = 0.f; // 행동 휴식 시간
	FCardLastPick lastPick; // 직전 카드 
	TArray<FCardModifier> nextModifiers; // 다음 보정
	FRandomStream random; // 뽑기 난수
	TMap<EEnemyCard, float> cardReadyTime; // 비공격 카드 쿨타임이 끝나는 시각


	/* 디버그 */
	void LogSelection(const FEnemyCardContext& context, const FEnemyCardChoice& choice, const TArray<FCardModifier>& modifiers) const; // 점수표를 출력하는 함수

	TArray<FCardEntry> lastEntries; // 직전 점수표
	TArray<float> lastChances;      // 직전 카드별 확률
	TOptional<EEnemyCard> forcedCard; // 강제용 카드

public:
	FORCEINLINE bool IsReady() const { return archetype != nullptr && cardTable != nullptr; }
	FORCEINLINE EEnemyRangeBand GetCurrentBand() const { return currentBand; }
	FORCEINLINE const TArray<FCardEntry>& GetLastEntries() const { return lastEntries; }
};