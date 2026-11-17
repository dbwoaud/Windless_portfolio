#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy/AI/EnemyAIProfile.h"
#include "Engine/DataTable.h"
#include "Enemy.generated.h"

struct FMonsterArchetypeRow;
struct FEnemyAttackRow;
class UHealthBarComponent;
class AAIController;
class AWeapon;
class ASoul;
class UWeakPointComponent;
class UEnemyPerceptionComponent;
class UAnimMontage;
class UEnemyAttackRunner;
class UEnemyActionSelector;
class UEnemyInterruptComponent;
class UPlayerAISignal;
struct FEnemyCardContext;
struct FEnemyCardChoice;
class UEnemyCardMover;
class UNavigationPath;
enum class EEnemyReach : uint8;
enum class EPlayerAIState : uint8;

UCLASS()
class WINDLESS_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override; // 적 캐릭터 피격 시 호출되는 함수
	virtual void Destroyed() override; // 적 캐릭터 파괴 시 호출되는 함수
	virtual void GetHit_Implementation(const FVector& impactPoint, AActor* hitter) override; // HitInterface를 구현하기 위한 함수
	virtual bool ReceiveWeaponHit(const FCombatHitInfo& hitInfo) override; // 기습 및 약점 배율을 적용한 뒤 피격 처리하는 함수
	virtual float GetTenacity() const override; // 강인도를 반환하는 함수
	virtual void DieByFall() override; // 낙사로 영구 사망하는 함수


	/* 적 설정 */
	void SetPatrolRoute(const TArray<AActor*>& route); // 스포너에서 순찰 경로를 전달받는 함수
	void SetPlacement(const FDataTableRowHandle& archetype, EEnemyIdleType idleType); // 스포너의 배치 값을 받는 함수
	
	
	/* 디버그용 */
	void DebugForceNextCard(EEnemyCard card); // 다음 카드 선택을 강제해 디버깅하는 함수

protected:
	virtual void BeginPlay() override;

	/* 적 상태 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "AI")
	EEnemyState enemyState = EEnemyState::EES_Idle; // 현재 AI 상태

	UPROPERTY(VisibleAnywhere, Transient, BlueprintReadOnly, Category = "AI|Behavior")
	EEnemyMoveType moveType = EEnemyMoveType::EMT_Wander; // 전투 전 이동 방식

	void SetEnemyState(EEnemyState newState); // 상태를 전환하는 함수
	bool IsDead() const; // 사망 상태인지 확인하는 함수
	bool IsInCombat() const; // 전투 상태인지 확인하는 함수


	/* 전투 시스템(공격) */
	virtual void Attack() override; // 공격 1타를 실행하는 함수
	virtual void AttackEnd() override; // 공격 1타가 끝났을 때 실행되는 함수
	virtual bool CanAttack() override; // 공격 가능 여부를 확인하는 함수
	virtual void CancelCurrentAttack() override; // 공격을 취소하는 함수
	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled) override; // 무기 충돌을 설정하는 함수


	/* 전투 시스템(피격) */
	virtual void OnDealtWeaponHit() override; // 무기 피격 시 실행되는 함수
	virtual void OnWeaponHitLanded() override; // 무기 공격이 실제로 대미지를 입혔을 때 실행되는 함수
	virtual void HandleDamage(float damageAmount) override; // 공격 대미지를 처리하는 함수
	virtual void OnStaggered(const FVector& impactPoint, AActor* hitter) override; // 경직 발동 시 실행되는 함수
	virtual void OnEndured() override; // 경직을 버텼을 때 실행되는 함수
	virtual bool IsKnockbackImmune() const override; // 날아감 면역 여부를 반환하는 함수


	/* 전투 시스템(사망) */
	virtual void Die_Implementation() override; // 적 캐릭터 사망 시 호출되는 함수
	void SpawnSouls(); // 소울을 생성하는 함수
	void GrantDropsToPlayer(); // 적 낙사 시 아이템과 경험치를 플레이어에게 주는 함수


	/* UI */
	virtual void HideHealthBar(); // 적 체력 바를 비활성화하는 함수
	virtual void ShowHealthBar(); // 적 체력 바를 활성화하는 함수

private:
	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* healthBarWidget; // 체력바 컴포넌트

	UPROPERTY(VisibleAnywhere)
	UEnemyPerceptionComponent* perception; // 감지 컴포넌트

	UPROPERTY(VisibleAnywhere)
	UWeakPointComponent* weakPoints; // 약점 부위 컴포넌트

	UPROPERTY(VisibleAnywhere)
	UEnemyAttackRunner* attackRunner; // 공격 실행기 컴포넌트

	UPROPERTY(VisibleAnywhere)
	UEnemyActionSelector* actionSelector; // 행동 선택기 컴포넌트

	UPROPERTY(VisibleAnywhere)
	UEnemyInterruptComponent* interrupt; // 끼어들기 컴포넌트

	UPROPERTY(VisibleAnywhere)
	UEnemyCardMover* cardMover; // 비공격 카드 실행기 컴포넌트

	UPROPERTY()
	AAIController* enemyController; // 적 AI 컨트롤러


	/* 초기화 */
	void InitializeEnemy(); // 게임 시작 시 적을 초기화하는 함수
	void SpawnDefaultWeapon(); // 적의 무기를 장착시키는 함수


	/* 적 상태 */
	void UpdateIdle(float DeltaTime); // 대기 상태로 갱신하는 함수
	void UpdatePatrolling(); // 순찰 상태로 갱신하는 함수
	void UpdateChasing(float DeltaTime); // 추적 상태로 갱신하는 함수
	void UpdateAttacking(float DeltaTime); // 공격 대기 상태로 갱신하는 함수
	void UpdateSearching(float DeltaTime); // 마지막 위치 탐색 상태로 갱신하는 함수
	void UpdateReturning(); // 복귀 상태로 갱신하는 함수


	/* 상태 전환 */
	bool TryEngageSightedTarget(bool bRollEncounter); // 감지된 대상이 있으면 교전을 시작하는 함수
	void EngageTarget(AActor* target, bool bRollEncounter); // 플레이어 조우 반응을 거쳐 교전을 시작하는 함수
	void StartChase(); // 추적을 시작하는 함수
	void StartSearch(const FVector& location, float waitTime, bool bHesitate); // 탐색을 시작하는 함수
	void StartReturn(const TCHAR* reason); // 배치 위치로 복귀를 시작하는 함수
	void ResetToIdle(); // 대기 상태로 초기화하는 함수
	void HandleNoTarget(); // 대상이 사라졌을 때 실행되는 함수

	UFUNCTION()
	void HandleTargetLost(FVector lastKnownLocation); // 감지 컴포넌트의 시야 상실 이벤트를 처리하는 함수


	/* 판정 헬퍼 함수 */
	bool IsTargetValid() const; // 전투 대상이 유효한지 확인하는 함수
	bool IsTargetInSafeZone(const AActor* target) const; // 대상이 세이브포인트 안전지대 안인지 확인하는 함수
	bool ShouldGiveUpChase() const; // 추적 포기 조건을 확인하는 함수
	bool CanReengageWhileReturning() const; //  복귀 중 재추적이 가능한지 확인하는 함수
	bool IsWithin(const AActor* target, float radius) const; // 대상이 반경 안에 있는지 확인하는 함수
	bool HasArrived() const; // 현재 이동 요청이 끝났는지 확인하는 함수
	float GetNow() const; // 현재 월드 시간을 반환하는 함수
	bool IsTargetStaggered(); // 대상이 경직 중인지 확인하는 함수


	/* 이동 */
	void MoveToActor(AActor* target, float speed); // 액터를 향해 이동하는 함수
	void MoveToLocation(const FVector& location, float speed); // 위치로 이동하는 함수
	void StopMovement(); // 이동을 멈추는 함수
	void FaceTarget(float DeltaTime); // 전투 대상을 바라보도록 회전하는 함수


	/* 순찰/배회 */
	void StartNextPatrol(); // 다음 순찰 지점으로 이동을 시작하는 함수
	bool ChooseWanderLocation(FVector& outLocation) const; // 순찰 목적지를 무작위로 고르는 함수
	AActor* ChooseNextPatrolTarget(); // 순찰 경로의 다음 지점을 반환하는 함수


	/* 공격 */
	void BeginAttackSequence(); // 콤보 공격을 시작하는 함수
	void FinishAttackSequence(); // 콤보 종료 후 후딜을 시작하는 함수
	bool UsesDataAttacks() const; // 원형 공격 데이터로 공격하는지 확인하는 함수
	void StartDataAttack(); // 공격 데이터 1개로 공격을 시작하는 함수
	void OnDataAttackFinished(EEnemyAttackEnd endReason); // 공격 데이터 1개로 공격 완료 시 실행되는 함수
	void SetAttackHitbox(bool bEnable); // 무기 히트 박스를 설정하는 함수
	void ApplyAttackHitData(); // 무기에 공격 데이터를 적용하는 함수
	float GetAttackRadius() const; // 공격 사거리를 반환하는 함수
	float GetCombatTurnRate() const; // 전투 회전 속도를 반환하는 함수


	/* 경직 */
	void EndStagger(); // 경직을 끝내고 전투를 재개하는 함수


	/* AI 데이터 */
	void ResolveArchetypeData(); // 적에 따른 원형과 공격 데이터를 찾아 저장하는 함수
	void ApplyArchetypeWeakPoints(); // 원형의 약점 규칙을 적용하는 함수

	UPROPERTY(EditAnywhere, Category = "AI|Data", meta = (RowType = "/Script/Windless.MonsterArchetypeRow"))
	FDataTableRowHandle archetypeRow; // 원형 겹

	UPROPERTY(EditInstanceOnly, Category = "AI|Data")
	EEnemyIdleType idleTypeOverride = EEnemyIdleType::EIT_None; // 대기 유형 종류

	FMonsterProfile profile; // 5겹 데이터의 확정 결과 및 출처 데이터
	const FMonsterArchetypeRow* archetypeData = nullptr; // 원형 데이터
	TArray<const FEnemyAttackRow*> attackData; // 공격 데이터
	TArray<FName> attackDataIds; // 공격 데이터의 행 이름


	/* AI 설정 (행동) */
	UPROPERTY(EditAnywhere, Category = "AI|Behavior")
	bool bAggressive = true; // 선공형 여부

	UPROPERTY(EditAnywhere, Category = "AI|Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float encounterImmediateChance = 0.85f; // 조우 시 즉시 공격 확률

	UPROPERTY(EditAnywhere, Category = "AI|Behavior", meta = (ClampMin = "0.0"))
	float encounterHesitateMin = 1.f; // 관망 최소 시간

	UPROPERTY(EditAnywhere, Category = "AI|Behavior", meta = (ClampMin = "0.0"))
	float encounterHesitateMax = 2.f; // 관망 최대 시간

	UPROPERTY(EditAnywhere, Category = "AI|Behavior")
	bool bLogStateChanges = true; // 상태 전환 로그 출력 여부


	/* AI 설정 (순찰) */
	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol", meta = (EditCondition = "moveType == EEnemyMoveType::EMT_Patrol", EditConditionHides))
	TArray<AActor*> patrolTargets; // 순찰 경로

	UPROPERTY(EditAnywhere, Category = "AI|Patrol", meta = (ClampMin = "0.0", EditCondition = "moveType == EEnemyMoveType::EMT_Wander", EditConditionHides))
	float wanderRadius = 600.f; // 배회 반경

	UPROPERTY(EditAnywhere, Category = "AI|Patrol", meta = (ClampMin = "0.0", EditCondition = "moveType != EEnemyMoveType::EMT_Fixed", EditConditionHides))
	float patrolWaitMin = 2.f; // 지점 도착 후 최소 대기 시간

	UPROPERTY(EditAnywhere, Category = "AI|Patrol", meta = (ClampMin = "0.0", EditCondition = "moveType != EEnemyMoveType::EMT_Fixed", EditConditionHides))
	float patrolWaitMax = 5.f; // 지점 도착 후 최대 대기 시간

	UPROPERTY(EditAnywhere, Category = "AI|Patrol", meta = (ClampMin = "0.0", EditCondition = "moveType != EEnemyMoveType::EMT_Fixed", EditConditionHides))
	float patrollingSpeed = 125.f; // 순찰 이동 속도

	UPROPERTY(EditAnywhere, Category = "AI|Patrol", meta = (ClampMin = "0.0"))
	float acceptanceRadius = 50.f; // 이동 도착 판정 반경


	/* AI 설정 (추적/탐색) */
	UPROPERTY(EditAnywhere, Category = "AI|Chase", meta = (ClampMin = "0.0"))
	float chasingSpeed = 300.f; // 추적 이동 속도

	UPROPERTY(EditAnywhere, Category = "AI|Chase", meta = (ClampMin = "0.0"))
	float chaseGiveUpDistance = 3000.f; // 대상과 이 거리 이상 벌어지면 추적 포기

	UPROPERTY(EditAnywhere, Category = "AI|Chase", meta = (ClampMin = "0.0"))
	float leashDistance = 3000.f; // 배치 위치에서 이 거리 이상 멀어지면 추적 포기

	UPROPERTY(EditAnywhere, Category = "AI|Chase", meta = (ClampMin = "0.0"))
	float searchDuration = 3.f; // 마지막 위치 도착 후 탐색 시간

	UPROPERTY(EditAnywhere, Category = "AI|Chase", meta = (ClampMin = "0.0"))
	float faceTargetSpeed = 8.f; // 대상 바라보기 회전 속도


	/* AI 설정 (공격) */
	UPROPERTY(EditAnywhere, Category = "AI|Attack", meta = (ClampMin = "0.0"))
	float attackRadius = 150.f; // 공격 사거리

	UPROPERTY(EditAnywhere, Category = "AI|Attack", meta = (ClampMin = "1.0"))
	float attackRangeExitMultiplier = 1.2f; // 공격 상태 유지 반경 배율

	UPROPERTY(EditAnywhere, Category = "AI|Attack", meta = (ClampMin = "0.0"))
	float attackStartDelay = 0.2f; // 사거리 진입 후 첫 공격까지 대기 시간

	UPROPERTY(EditAnywhere, Category = "AI|Attack", meta = (ClampMin = "1"))
	int32 minComboHits = 2; // 콤보 최소 타수

	UPROPERTY(EditAnywhere, Category = "AI|Attack", meta = (ClampMin = "1"))
	int32 maxComboHits = 3; // 콤보 최대 타수

	UPROPERTY(EditAnywhere, Category = "AI|Attack", meta = (ClampMin = "0.0"))
	float attackCooldown = 1.5f; // 콤보 종료 후 후딜 시간


	/* AI 설정 (기습) */
	UPROPERTY(EditAnywhere, Category = "AI|Stealth", meta = (ClampMin = "1.0"))
	float sneakAttackMultiplier = 1.5f; // 비전투 상태에서 웅크린 공격자에게 받는 기습 피해 배율


	/* AI 설정 (카드) */
	bool UsesCardSelection() const; // 카드 선택기 사용 여부를 확인하는 함수
	void UpdateCardCombat(float DeltaTime); // 전투 카드를 갱신하는 함수
	void SelectNextCard(); // 다음 공격 카드를 선택하고 실행하는 함수
	void EndCurrentCard(); // 공격이 아닌 카드를 끝내는 함수
	bool TryStartGlare(const FEnemyCardContext& context); // 노려보기를 시작하는 함수
	bool IsTargetFalling() const; // 대상이 공중에 있는지 확인하는 함수

	EEnemyCard currentCard = EEnemyCard::EC_Rest; // 실행 중인 카드
	bool bCardRunning = false; // 공격이 아닌 카드 실행 여부
	EEnemyReach JudgeTargetReach(const FEnemyCardContext& context, UNavigationPath*& outPath); // 대상까지 도달 가능한지 판정하는 함수
	float glareRecheckTimer = 0.f; // 노려보기 중 경로 재확인 타이머
	bool bGlaring = false; // 노려보기 여부
	int32 pendingAttackIndex = INDEX_NONE; // 선택기가 고른 공격 인덱스
	int32 seenSignalSerial = 0; // 마지막으로 확인한 플레이어 신호 번호
	bool bAttackTouched = false; // 공격이 닿았는지 여부
	bool bAttackLanded = false; // 공격에 대미지를 입었는지 여부
	bool bCurrentAttackHeavy = false; // 강공격 여부
	TMap<uint8, float> signalReselectTime; // 신호별 마지막 즉시 다시 고르기 시각

	
	/* AI 설정(신호) */
	FEnemyCardContext BuildCardContext(); // 카드 상황을 모으는 함수
	void ExecuteCardChoice(const FEnemyCardChoice& choice); // 선택 카드를 실행하는 함수
	bool CheckPlayerSignals(); // 플레이어 신호로 재선택하는 함수
	void RecordAttackResult(); // 공격이 끝났을 때 적중이나 헛방을 기록하는 함수
	UPlayerAISignal* GetTargetSignals() const; // 대상의 플레이어 신호 컴포넌트를 찾는 함수


	/* 사망 */
	UPROPERTY(EditAnywhere, Category = "Combat|Death")
	float deathLifeSpan = 8.f; // 사망 시 시체가 남아있는 시간

	UPROPERTY(EditAnywhere, Category = "Combat|Death")
	TSubclassOf<ASoul> soulClass; // 사망 시 생성되는 소울


	/* 무기 */
	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<AWeapon> weaponClass; // 장착 무기


	/* 런타임 */
	FVector homeLocation = FVector::ZeroVector; // 배치 위치
	FRotator homeRotation = FRotator::ZeroRotator; // 배치 회전
	int32 patrolIndex = INDEX_NONE; // 현재 순찰 지점 인덱스
	bool bWarnedNoPatrolRoute = false; // 순찰 경로 없음 경고 출력 여부
	float idleWaitRemaining = 0.f; // 대기 남은 시간
	bool bProvoked = false; // 비선공형이 공격받아 적대 상태인지 여부
	bool bForcedChase = false; // 강제 추적 여부
	bool bHesitating = false; // 조우 관망 중인지 여부 
	float searchWaitRemaining = 0.f; // 탐색남은 시간
	bool bHasPendingSearch = false; // 시야 상실로 탐색이 예약됐는지 여부
	FVector pendingSearchLocation = FVector::ZeroVector; // 예약된 탐색 위치
	int32 comboHitsRemaining = 0; // 남은 콤보 타수
	float nextAttackTime = 0.f; // 다음 공격 가능 시각
	bool bHoldingForTargetStagger = false; // 대상 경직으로 새 공격을 보류 중인지 여부
	bool bFellToDeath = false; // 낙사 여부


	/* 디버그용 */
	UPROPERTY(EditInstanceOnly, Category = "AI|Debug")
	FName debugForcedAttackId = NAME_None; // 실행할 공격

public:
	/* 변수 Getter 함수 */
	FORCEINLINE UWeakPointComponent* GetWeakPoints() const { return weakPoints; }
	FORCEINLINE UEnemyPerceptionComponent* GetPerception() const { return perception; }
	FORCEINLINE UEnemyAttackRunner* GetAttackRunner() const { return attackRunner; }
	FORCEINLINE UEnemyInterruptComponent* GetInterrupt() const { return interrupt; }
	FORCEINLINE EEnemyState GetEnemyState() const { return enemyState; }
	FORCEINLINE const FMonsterProfile& GetProfile() const { return profile; }
	FORCEINLINE const FMonsterArchetypeRow* GetArchetypeData() const { return archetypeData; }
	FORCEINLINE const TArray<const FEnemyAttackRow*>& GetAttackData() const { return attackData; }
	FORCEINLINE bool HasFallenToDeath() const { return bFellToDeath; }
};