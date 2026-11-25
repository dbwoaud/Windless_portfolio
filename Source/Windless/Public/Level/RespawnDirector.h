#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnDirector.generated.h"

class AWindlessCharacter;
class APlayerController;
class ASnakeMount;
class UAnimMontage;
class UAnimInstance;
class ASoulMarker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRespawnFinishedSignature);

UCLASS()
class WINDLESS_API ARespawnDirector : public AActor
{
	GENERATED_BODY()

public:
	ARespawnDirector();


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Respawn|Event")
	FOnRespawnFinishedSignature onRespawnFinished; // 부활 완료 시 실행되는 델리게이트 

protected:
	virtual void BeginPlay() override;


	/* 사망 */
	UFUNCTION()
	void HandlePlayerDeath(); // 플레이어 사망 시 실행되는 함수


	/* 부활 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Respawn|Event")
	void OnRespawnStartedBP(); // 부활 연출이 시작될 때 실행되는 블루프린트용 함수

	UFUNCTION(BlueprintImplementableEvent, Category = "Respawn|Event")
	void OnRespawnedBP(); // 부활 완료 시 실행되는 블루프린트용 함수

	UPROPERTY(EditInstanceOnly, Category = "Respawn")
	TObjectPtr<AActor> defaultRespawnPoint;


	/* 연출(애니메이션) */
	UPROPERTY(EditInstanceOnly, Category = "Respawn|Snake")
	TObjectPtr<ASnakeMount> rescueSnake; // 뱀 NPC

	UPROPERTY(EditAnywhere, Category = "Respawn|Snake")
	TObjectPtr<UAnimMontage> snakeGrabMontage; // 뱀 목물기 애니메이션

	UPROPERTY(EditAnywhere, Category = "Respawn|Snake", meta = (ClampMin = "0.0"))
	float snakeCinematicDuration = 3.f; // 연출 지속 시간

	UFUNCTION()
	void OnSnakeMontageEnded(UAnimMontage* montage, bool bInterrupted); // 뱀 애니메이션 재생 완료 시 실행되는 함수


	/* 연출(페이드) */
	UPROPERTY(EditAnywhere, Category = "Respawn|Fade", meta = (ClampMin = "0.0"))
	float fadeOutDuration = 1.5f; // 페이드 아웃 지속 시간

	UPROPERTY(EditAnywhere, Category = "Respawn|Fade", meta = (ClampMin = "0.0"))
	float fadeInDuration = 1.5f; // 페이드 인 지속 시간

	UPROPERTY(EditAnywhere, Category = "Respawn|Fade")
	FLinearColor fadeColor = FLinearColor::Black; // 페이드 색


private:
	/* 캐시 변수 */
	void CachePlayer(); // 플레이어 캐릭터와 컨트롤러를 저장하는 함수
	UPROPERTY() 
	TObjectPtr<AWindlessCharacter> playerCharacter; // 플레이어 캐릭터

	UPROPERTY()
	TObjectPtr<APlayerController> playerController; // 플레이어 컨트롤러 

	UPROPERTY() 
	TObjectPtr<UAnimInstance> cachedSnakeAnim; // 뱀 NPC 애니메이션 인스턴스 


	/* 사망 */
	FVector deathLocation = FVector::ZeroVector; // 플레이어 사망 위치

	UPROPERTY(EditAnywhere, Category = "Respawn")
	TSubclassOf<ASoulMarker> soulMarkerClass; // 손실 마커 클래스

	void DisablePlayerControl(); // 플레이어 제어를 비활성화하는 함수


	/* 부활 */
	void TeleportAndRevive(); // 세이브 포인트로 이동 후 부활을 수행하는 함수
	void ResolveRespawnTransform(FVector& outLocation, FRotator& outRotation) const; // 부활 좌표를 결정하는 함수
	void FinishRespawn(); // 부활 완료 시 실행되는 함수
	void EnablePlayerControl(); // 플레이어 제어를 활성화하는 함수

	FVector  startLocation = FVector::ZeroVector; // PlayerStart 위치
	FRotator startRotation = FRotator::ZeroRotator; // PlayerStart 회전


	/* 연출(애니메이션) */
	void BeginSnakeCinematic(); // 뱀 NPC의 부활 연출을 시작하는 함수 
	void OnSnakeCinematicComplete(); // 뱀 NPC의 부활 연출이 끝나고 실행되는 함수
	FTimerHandle cinematicTimer; // 부활 애니메이션 연출용 타이머


	/* 연출(페이드) */
	void FadeInAndRestore(); // 페이드 인을 실행하는 함수
	void FadeOut(); // 페이드 아웃을 실행하는 함수
	FTimerHandle fadeOutTimer; // 부활 페이드 연출용 타이머
};