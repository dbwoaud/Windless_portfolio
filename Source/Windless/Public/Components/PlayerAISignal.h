#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/AI/EnemyCardModRows.h"
#include "PlayerAISignal.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UPlayerAISignal : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerAISignal();


	/* 플레이어 호출 */
	void NotifyAttackBegin(); // 공격 시작을 알리는 함수
	void NotifyAttackActive(bool bActive); // 무기 판정 활성화/비활성화를 알리는 함수
	void NotifyAttackHit(); // 플레이어 공격 적중을 알리는 함수
	void NotifyAttackEnd(); // 플레이어 공격 완료를 알리는 함수
	void NotifyItemUse(bool bUsing); // 아이템 사용 시작과 끝을 알리는 함수
	void NotifyAim(bool bInAiming); // 활 조준 시작과 끝을 알리는 함수
	void NotifyGuard(bool bGuarding); // 가드 시작과 끝을 알리는 함수


	/* 몬스터 호출 */
	FPlayerAISnapshot GetSnapshot() const; // 플레이어 스냅샷을 반환하는 함수
	float GetGuardHoldTime() const; // 플레이어 가드 유지 시간을 반환하는 함수


	/* 디버그용 */
	void FakeSignal(EPlayerAIState state, float duration); // 가짜 신호를 전송하는 함수

private:
	/* 신호 */
	void PushEvent(EPlayerAIState state); // 재선택 신호를 올리는 함수

	int32 eventSerial = 0; // 신호 번호
	EPlayerAIState lastEvent = EPlayerAIState::EPS_AttackBegin; // 마지막 신호


	/* 플레이어 상태 */
	void HandleGuardHeld(); // 가드 유지를 처리하는 함수

	bool bAttackBegin = false;  // 공격 시작 여부
	bool bAttackActive = false; // 무기 판정 활성화 여부
	bool bHitThisSwing = false; // 공격 적중 여부
	bool bWhiff = false;        // 헛방 후딜 여부
	bool bUsingItem = false;    // 아이템 사용 여부
	bool bAiming = false;       // 활 조준 여부
	float guardStartTime = -1.f; // 가드 시작 시간
	FTimerHandle guardHeldTimer; // 가드 유지 기준 시간 타이머


	/* 헬퍼 함수 */
	float GetNow() const; // 현재 게임 시간을 반환하는 함수


	/* 디버그용 */
	bool IsFaking(EPlayerAIState state) const; // 가짜 신호가 살아 있는지 확인하는 함수

	TMap<uint8, float> fakeUntil; // 가짜 신호가 끝나는 시각

public:
	FORCEINLINE int32 GetEventSerial() const { return eventSerial; }
	FORCEINLINE EPlayerAIState GetLastEvent() const { return lastEvent; }
};