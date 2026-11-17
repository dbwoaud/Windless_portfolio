#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/AI/EnemyAITypes.h"
#include "EnemyCardMover.generated.h"

class AAIController;
class ACharacter;

/* 비공격 카드 이동 값 */
struct FEnemyCardMoveParams
{
	float walkSpeed = 1.6f; // 걷기 속도
	float runSpeed = 4.2f;  // 달리기 속도
	float runDistance = 5.f; // 달리기 거리
	float preferredRange = 1.8f; // 다가가기 정지 거리
	float turnRate = 180.f; // 회전 속도
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UEnemyCardMover : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyCardMover();


	/* 초기화 함수 */
	void Setup(const FEnemyCardMoveParams& inParams); // 원형 이동을 설정하는 함수


	/* 상태 확인 */
	bool CanSideStep(const AActor* inTarget) const; // 옆걸음 가능 여부를 확인하는 함수 
	bool CanBackOff(const AActor* inTarget) const; // 물러서기 가능 여부를 확인하는 함수


	/* 카드 */
	bool StartCard(EEnemyCard inCard, AActor* inTarget); // 카드 동작을 시작하는 함수
	bool UpdateCard(float DeltaTime); // 카드 동작을 진행하는 함수
	void StopCard(); // 카드 동작을 멈추는 함수


	/* 노려보기 */
	void StartGlare(AActor* inTarget, const FVector& standPoint, bool bMoveFirst, float glareTime); // 노려보기를 시작하는 함수

private:
	/* 이동 */
	FEnemyCardMoveParams params; // 원형 이동 값
	TWeakObjectPtr<AActor> target; // 대상
	bool IsMoveClear(const FVector& to) const; // 지점 도달 가능 여부를 확인하는 함수


	/* 이동(옆걸음) */
	float sideDirection = 1.f; // 옆걸음 방향
	float sideRadius = 0.f; // 옆걸음 원 반지름
	float nextRetargetTime = 0.f; // 옆걸음 다음 지점 갱신 시각
	bool StartRest(); // 숨 고르기를 시작하는 함수
	bool StartSideStep(); // 옆걸음을 시작하는 함수
	bool RetargetSideStep(); // 옆걸음 다음 지점으로 이동하는 함수
	void SetStrafe(bool bStrafe); // 스트레이프를 설정하는 함수


	/* 이동(물러서기) */
	bool StartBackOff(); // 물러서기를 시작하는 함수


	/* 이동(다가가기) */
	bool bApproachRunning = false; // 다가가기 중 달리기 여부
	bool StartApproach(); // 다가가기를 시작하는 함수
	bool UpdateApproach(); // 다가가기를 진행하는 함수


	/* 이동(노려보기) */
	bool bGlare = false; // 노려보기 여부
	float glareDuration = 4.f; // 노려보기 시간
	bool bGlareMoving = false; // 가장 가까운 지점 이동 여부
	bool UpdateGlare(float DeltaTime); // 노려보기를 진행하는 함수


	/* 이동(회전) */
	void FaceTarget(float DeltaTime); // 대상을 향해 회전하는 함수
	bool MoveToPoint(const FVector& point, float speed); // 이동 지점으로 움직이는 함수
	bool IsMoving() const; // 이동 중인지 확인하는 함수


	/* 카드 */
	EEnemyCard card = EEnemyCard::EC_Rest; // 실행 중인 카드
	void Finish(const TCHAR* reason); // 카드 동작을 끝내는 함수


	/* 상태 확인 */
	float GetTargetDistance() const; // 대상까지 수평 거리를 반환하는 함수


	/* 캐시 변수 */
	AAIController* GetAIController() const; // AI 컨트롤러를 반환하는 함수
	ACharacter* GetCharacter() const; // 캐릭터를 반환하는 함수
	float GetNow() const; // 현재 게임 시간을 반환하는 함수
	bool bRunning = false; // 실행 중 여부
	float endTime = 0.f; // 완료 시각

public:
	FORCEINLINE bool IsRunning() const { return bRunning; }
	FORCEINLINE bool IsGlaring() const { return bGlare; }
	FORCEINLINE EEnemyCard GetCard() const { return card; }
};