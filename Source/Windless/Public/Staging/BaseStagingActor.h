#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseStagingActor.generated.h"

class ATriggerVolumeBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStagingPlayedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStagingStoppedSignature);

UCLASS()
class WINDLESS_API ABaseStagingActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseStagingActor();
	/* 연출 제어 */
	UFUNCTION(BlueprintCallable, Category = "Staging")
	void Play(); // 연출을 시작하는 함수

	UFUNCTION(BlueprintCallable, Category = "Staging")
	void Stop(); // 연출을 중지하는 함수

	UFUNCTION(BlueprintCallable, Category = "Staging")
	void ResetStaging(); // 연출을 리셋하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Staging|Event")
	FOnStagingPlayedSignature onStagingPlayed;   // 연출 시작 시 실행되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Staging|Event")
	FOnStagingStoppedSignature onStagingStopped; // 연출 중지 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;
	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Staging")
	USceneComponent* sceneRoot; // 씬 루트 컴포넌트


	/* 연출 제어 */
	virtual void HandlePlay(); // 연출 시작을 처리하는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Staging|Event")
	void OnStagingPlayBP(); // 연출 시작 시 블루프린트에서 실행되는 함수

	virtual void HandleStop(); // 연출 중지를 처리하는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Staging|Event")
	void OnStagingStopBP(); // 연출 중시 시 블루프린트에서 실행되는 함수


	/* 트리거 연동 */
	UFUNCTION()
	void HandleTriggerEnter(AActor* activator); // 연출 발동 트리거 진입 시 실행되는 함수

	UPROPERTY(EditInstanceOnly, Category = "Staging|Trigger")
	TObjectPtr<ATriggerVolumeBase> triggerVolume; // 연출 발동 트리거

	UFUNCTION()
	void HandleStopTriggerEnter(AActor* activator); // 연출 정지 트리거 진입 시 실행되는 함수

	UPROPERTY(EditInstanceOnly, Category = "Staging|Trigger")
	TObjectPtr<ATriggerVolumeBase> stopTriggerVolume; // 연출 정지 트리거 (들어서면 Stop)

	UPROPERTY(EditAnywhere, Category = "Staging|Trigger")
	bool bBindToTrigger = true; // 트리거 진입 델리게이트에 할당할 지 여부


	/* 설정 */
	UPROPERTY(EditAnywhere, Category = "Staging|Settings")
	bool bAutoPlayOnBeginPlay = false; // 게임 시작 시 발동할지 여부

	UPROPERTY(EditAnywhere, Category = "Staging|Settings")
	bool bPlayOnce = true; // 일회용 발동 여부

	UPROPERTY(EditAnywhere, Category = "Staging|Settings", meta = (ClampMin = "0.0"))
	float startDelay = 0.f; // 발동 요청 후 지연 시간


	/* 연쇄 연출 */
	UPROPERTY(EditInstanceOnly, Category = "Staging|Chain")
	TArray<TObjectPtr<ABaseStagingActor>> chainedEffects; // 연쇄 연출 목록 	

private:	
	/* 상태 확인 */
	bool bHasPlayed = false; // 연출이 발동한 적 있는지 여부
	bool bIsActive = false;  // 현재 연출이 진행 중인지 여부
	FTimerHandle startDelayTimer; // 시작 지연 타이머 핸들


	/* 연출 제어 */
	void DoPlay(); // 지연 시간 이후 연출을 실행하는 함수
	void PlayChainedEffects(); // 연쇄 연출을 시작하는 함수

public:
	/* 변수 Getter 함수 */
	FORCEINLINE bool IsStagingActive() const { return bIsActive; }
};