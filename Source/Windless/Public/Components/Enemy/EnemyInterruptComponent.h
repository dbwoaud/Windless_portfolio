#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/AI/EnemyInterrupt.h"
#include "EnemyInterruptComponent.generated.h"

class UAnimInstance;

DECLARE_DELEGATE(FOnEnemyInterruptEnded);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UEnemyInterruptComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyInterruptComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	/* 경직 */
	void ApplyStagger(EStaggerResult result, const FVector& fromLocation); // 경직 결정 결과로 경직을 적용하는 함수
	void CancelInterrupt(); // 끼어들기를 취소하는 함수


	/* 회복 */
	bool ConsumeStaggerRecovered(); // 경직 회복을 소비하는 함수


	/* 델리게이트 */
	FOnEnemyInterruptEnded onInterruptEnded; // 끼어들기 완료 시 실행되는 델리게이트

private:
	/* 약한 경직 */
	void StartWeakStagger(EInterruptHitAction action); // 약한 경직을 시작하는 함수
	void EndInterrupt(); // 끼어들기를 끝내는 함수
	void ResetInterrupt(); // 끼어들기 상태를 초기화하는 함수

	EEnemyInterrupt current = EEnemyInterrupt::EIN_None; // 현재 끼어들기 상태
	float duration = 0.f; // 끼어들기 길이
	float elapsed = 0.f; // 현재 끼어들기 시간
	bool bStaggerRecovered = false; // 경직 회복 기록


	/* 강한 경직 */
	void StartStrongStagger(EInterruptHitAction action, const FVector& fromLocation); // 강한 경직을 시작하는 함수
	void TickStrongPhase(); // 강한 경직 구간을 진행하는 함수
	void EnterStrongPhase(EStrongStaggerPhase phase); // 강한 경직 구간에 들어갈 때 실행되는 함수
	void LaunchOwner(const FVector& fromLocation, float distance); // 공격 입력 방향으로 캐릭터를 날리는 함수
	void LockRotation(); // 회전 설정을 잠그는 함수
	void RestoreRotation(); // 회전 설정을 되돌리는 함수

	FStrongStaggerTimes strongTimes; // 강한 경직 구간 시간
	EStrongStaggerPhase strongPhase = EStrongStaggerPhase::ESSP_Done; // 현재 강한 경직 구간
	bool bRotationLocked = false; // 회전 잠금 여부
	bool bSavedOrientToMovement = true; // 잠그기 전 회전 값


	/* 애니메이션 */
	float SyncHitReactMontage(float time); // 피격 애니메이션 속도를 경직 시간에 맞추는 함수
	void StopHitReactMontage(); // 피격 애니메이션 재생을 멈추는 함수
	float PlayKnockdownSection(EStrongStaggerPhase phase, float time); // 강한 경직 애니메이션 섹션을 재생하는 함수
	void StopKnockdownMontage(); // 강한 경직 애니메이션 재생을 멈추는 함수
	UAnimInstance* GetAnimInstance() const; // 캐릭터의 애니메이션 인스턴스를 반환하는 함수
	bool bWarnedMontageRate = false; // 애니메이션 재생 속도 경고 출력 여부
	bool bWarnedKnockdown = false; // 강한 경직 몽타주 경고 출력 여부


	/* 디버그용 */
	void DrawInterruptDebug() const; // 화면에 남은 경직 시간을 표시하는 함수

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bLogInterrupts = true; // 끼어들기 로그 출력 여부

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawInterrupt = true; // 남은 경직 시간 표시 여부

public:
	/* 변수 Getter 함수 */
	FORCEINLINE bool IsInterrupted() const { return current != EEnemyInterrupt::EIN_None; }
	FORCEINLINE EEnemyInterrupt GetInterrupt() const { return current; }
	FORCEINLINE float GetRemaining() const { return IsInterrupted() ? FMath::Max(0.f, duration - elapsed) : 0.f; }
	FORCEINLINE EStrongStaggerPhase GetStrongPhase() const { return strongPhase; }
};