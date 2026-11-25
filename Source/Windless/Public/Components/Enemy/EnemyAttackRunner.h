#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/AI/EnemyAttackTiming.h"
#include "EnemyAttackRunner.generated.h"

struct FEnemyAttackRow;
class UEnemyAttackRunner;
class UAnimInstance;
class UAnimMontage;

DECLARE_DELEGATE_OneParam(FOnEnemyAttackHitbox, bool); 
DECLARE_DELEGATE_OneParam(FOnEnemyAttackFinished, EEnemyAttackEnd);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEnemyAttackerChanged, UEnemyAttackRunner*, bool);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UEnemyAttackRunner : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyAttackRunner();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	/* 공격 */
	bool StartAttack(const FEnemyAttackRow& attack, FName attackId, AActor* target, float inTurnRate); // 공격을 시작하는 함수
	void CancelAttack(const TCHAR* reason); // 공격을 취소하는 함수


	/* 델리게이트 */
	FOnEnemyAttackHitbox onHitboxChanged;    // 무기의 히트박스 변경 시 실행되는 델리게이트
	FOnEnemyAttackFinished onAttackFinished; // 공격 완료 시 실행되는 델리게이트
	FOnEnemyAttackerChanged onAttackerChanged; // 공격 적 변경 시 실행되는 델리게이트

private:
	/* 예고 */
	void CommitAttack(); // 예고 시작 후 공격을 확정하는 함수


	/* 공격 */
	void EnterPhase(int32 index); // 공격 구간 진입 시 실행되는 함수
	void ExitCurrentPhase(); // 공격 구간 이탈 시 실행되는 함수
	void FinishAttack(EEnemyAttackEnd endReason); // 공격 완료 시 실행되는 함수
	const FEnemyAttackRow* currentAttack = nullptr; // 실행 중인 공격 데이터
	FName currentAttackId = NAME_None; // 실행 중인 공격 행 이름
	TArray<FEnemyAttackPhaseSpan> timeline; // 공격 구간 이름
	int32 phaseIndex = INDEX_NONE; // 현재 공격 구간
	float elapsed = 0.f; // 공격 시작부터 흐른 시간
	FFloatInterval rollWindow = FFloatInterval(0.f, 0.f); // 구르기 성공 구간


	/* 선회 및 추적 */
	void TickPreTurn(float DeltaTime); // 선회를 진행하는 함수
	void TickTracking(float DeltaTime); // 구간별로 회전하는 함수
	void TurnOwnerTowardTarget(float maxDegrees); // 캐릭터를 대상 쪽으로 회전하는 함수
	float GetYawToTarget() const; // 대상까지의 수평 각도를 반환하는 함수
	float GetActiveStartTime() const; // 판정 시작 시각을 반환하는 함수

	TWeakObjectPtr<AActor> currentTarget; // 현재 추적 대상
	float turnRate = 180.f; // 전투 회전 속도
	bool bPreTurning = false; // 선회 중 여부
	float preTurnElapsed = 0.f; // 선회 시간
	bool bLockLogged = false; // 방향 고정 로그 출력 여부


	/* 애니메이션 */
	void StartMontage(); // 애니메이션을 재생하는 함수
	void SyncPhaseMontage(int32 index); // 공격 구간에 맞는 애니메이션을 동기화하는 함수
	UAnimInstance* GetAnimInstance() const; // 캐릭터의 애니메이션 인스턴스를 반환하는 함수 

	UPROPERTY(Transient)
	UAnimMontage* currentMontage = nullptr; // 재생 중인 애니메이션

	bool bPhaseSections = false; // 공격 구간마다 섹션 사용 여부


	/* 연출 */
	void PlayWindupSound(); // 예고 효과음을 재생하는 함수


	/* 디버그용 */
	void DrawFacingDebug(float turnScale) const; // 방향 화살표를 그리는 함수
	void DrawPhaseBarDebug() const; // 머리 위 구간 막대와 화면 글자를 그리는 함수

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bLogPhases = true; // 구간 로그 출력 여부

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawFacing = true; // 방향 화살표 표시 여부

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawPhaseBar = true; // 구간 막대 및 구르기 성공 구간 표시 여부

public:
	/* 변수 Getter 함수 */
	FORCEINLINE const FEnemyAttackRow* GetCurrentAttack() const { return currentAttack; }
	FORCEINLINE bool IsCommitted() const { return IsRunning() && !bPreTurning; }
	FORCEINLINE bool IsRunning() const { return currentAttack != nullptr; }
	FORCEINLINE float GetElapsed() const { return elapsed; }
	FORCEINLINE FName GetCurrentAttackId() const { return currentAttackId; }
	FORCEINLINE float GetCurrentWindup() const { return GetActiveStartTime(); }
	EEnemyAttackPhase GetPhase() const;
};