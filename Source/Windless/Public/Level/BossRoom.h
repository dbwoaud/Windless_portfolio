#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossRoom.generated.h"

class ATriggerVolumeBase;
class ABoss;
class AWindlessCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossRoomSealedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossRoomUnsealedSignature);

UCLASS()
class WINDLESS_API ABossRoom : public AActor
{
	GENERATED_BODY()
	
public:
	ABossRoom();

	/* 봉쇄 제어 */
	UFUNCTION(BlueprintCallable, Category = "BossRoom")
	void SealRoom();   // 보스방을 봉쇄하는 함수

	UFUNCTION(BlueprintCallable, Category = "BossRoom")
	void UnsealRoom(); // 보스방 봉쇄를 해제하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "BossRoom|Event")
	FOnBossRoomSealedSignature onRoomSealed;     // 봉쇄 시 실행되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "BossRoom|Event")
	FOnBossRoomUnsealedSignature onRoomUnsealed; // 해제 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;


	/* 진입 트리거 */
	UFUNCTION()
	void HandleEntryTriggerEnter(AActor* activator); // 진입 트리거 감지 시 실행되는 함수

	UPROPERTY(EditInstanceOnly, Category = "BossRoom|Trigger")
	TObjectPtr<ATriggerVolumeBase> entryTrigger; // 보스방 진입 감지 트리거


	/* 보스 */
	UFUNCTION()
	void HandleBossDeath(ABoss* deadBoss); // 보스 사망 시 실행되는 함수

	UPROPERTY(EditInstanceOnly, Category = "BossRoom|Boss")
	TObjectPtr<ABoss> boss; // 이 방의 보스 액터


	/* 플레이어 */
	UFUNCTION()
	void HandlePlayerDeath(); // 플레이어 사망 시 실행되는 함수


	/* 봉쇄 */
	UFUNCTION(BlueprintImplementableEvent, Category = "BossRoom|Event")
	void OnRoomSealedBP();   // 봉쇄 시 실행되는 블루프린트용 함수


	/* 봉쇄 해제 */
	UFUNCTION(BlueprintImplementableEvent, Category = "BossRoom|Event")
	void OnRoomUnsealedBP(); // 봉쇄 해제 시 실행되는 블루프린트용 함수


	/* 봉쇄 설정 */
	UPROPERTY(EditInstanceOnly, Category = "BossRoom|Settings")
	TArray<TObjectPtr<AActor>> barrierActors; // 봉쇄 시 활성화할 장애물 목록

	UPROPERTY(EditAnywhere, Category = "BossRoom|Settings")
	bool bStartUnsealed = true; // 시작 시 봉쇄 해제 상태 여부

private:
	/* 상태 확인 */
	bool bSealed = false; // 현재 봉쇄 상태 여부


	/* 보스 */
	bool bBossDefeated = false; // 보스 처치 여부
	UPROPERTY()


	/* 플레이어 */
	TObjectPtr<AWindlessCharacter> cachedPlayer; // 봉쇄 대상 플레이어


	/* 봉쇄 설정 */
	void SetBarriersActive(bool bActive); // 봉쇄 장애물을 활성화/비활성화하는 함수


	/* 델리게이트 */
	void BindCombatEndEvents(); // 보스,플레이어 사망 델리게이트를 구독하는 함수
	void UnbindCombatEndEvents(); // 보스, 플레이어 사망 델리게이트를 구독 해제하는 함수

public:
	/* 변수 Getter 함수 */
	FORCEINLINE bool IsSealed() const { return bSealed; }
};