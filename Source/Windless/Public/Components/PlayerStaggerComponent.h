#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/AI/EnemyInterrupt.h"
#include "PlayerStaggerComponent.generated.h"

class UAnimInstance;

DECLARE_DELEGATE(FOnPlayerStrongStaggerEnded);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UPlayerStaggerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStaggerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	/* 강한 경직 */
	void BeginStrongStagger(const FVector& fromLocation); // 강한 경직을 시작하는 함수
	bool TryRollGetUp(); // 다운 중 구르기 입력 시 실행되는 함수
	void CancelStagger(); // 강한 경직을 취소하는 함수


	/* 델리게이트 */
	FOnPlayerStrongStaggerEnded onStrongStaggerEnded; // 강한 경직이 끝났을 때 실행되는 델리게이트

private:
	/* 강한 경직 */
	void EnterPhase(EStrongStaggerPhase newPhase); // 구간에 들어갈 때 실행되는 함수
	void FinishStagger(const TCHAR* reason, float blendOut); // 강한 경직을 종료하는 함수
	void ResetStagger(); // 경직 상태를 초기화하는 함수
	void LaunchOwner(const FVector& fromLocation, float distance); // 공격이 들어온 방향으로 날리는 함수

	FStrongStaggerTimes times; // 구간 시간
	EStrongStaggerPhase phase = EStrongStaggerPhase::ESSP_Done; // 현재 구간
	float elapsed = 0.f; // 현재 시간


	/* 애니메이션 */
	float PlayKnockdownSection(EStrongStaggerPhase targetPhase, float time); // 강한 경직 애니메이션 섹션을 재생하는 함수
	void StopKnockdownMontage(float blendOut); // 강한 경직 몽타주 재생을 멈추는 함수
	UAnimInstance* GetAnimInstance() const; // 캐릭터의 애니메이션 인스턴스를 반환하는 함수
	bool bWarnedKnockdown = false; // 몽타주 없음 경고 출력 여부
	bool bWarnedMontageRate = false; // 재생 속도 경고 출력 여부


	/* 디버그용 */
	void DrawStaggerDebug() const; // 화면에 남은 시간을 표시하는 함수

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bLogStagger = true; // 로그 출력 여부

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDrawStagger = true; // 남은 시간 표시 여부

public:
	/* 변수 Getter 함수 */
	FORCEINLINE bool IsStrongStaggered() const { return phase != EStrongStaggerPhase::ESSP_Done; }
	FORCEINLINE EStrongStaggerPhase GetPhase() const { return phase; }
	FORCEINLINE float GetRemaining() const { return IsStrongStaggered() ? FMath::Max(0.f, times.GetTotal() - elapsed) : 0.f; }
};