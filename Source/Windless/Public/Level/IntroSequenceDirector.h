#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IntroSequenceDirector.generated.h"

class AWindlessCharacter;
class APlayerController;
class UAnimMontage;
class UDataTable;
class UAnimInstance;
class ABaseStagingActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIntroFinishedSignature);

UCLASS()
class WINDLESS_API AIntroSequenceDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	AIntroSequenceDirector();


	/* 인트로 제어 */
	UFUNCTION(BlueprintCallable, Category = "Intro")
	void StartIntro(); // 인트로 시퀀스를 시작하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Intro|Event")
	FOnIntroFinishedSignature onIntroFinished; // 인트로 종료 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;


	/* 시작 설정 */
	UPROPERTY(EditAnywhere, Category = "Intro")
	bool bAutoStartOnBeginPlay = true; // 게임 시작 시 자동으로 인트로를 시작할지 여부


	/* 연출(페이드) */
	UPROPERTY(EditAnywhere, Category = "Intro|Fade", meta = (ClampMin = "0.0"))
	float fadeInDuration = 2.f; // 화면 페이드인 시간

	UPROPERTY(EditAnywhere, Category = "Intro|Fade")
	FLinearColor fadeColor = FLinearColor::Black; // 페이드 색


	/* 연출(애니메이션) */
	UFUNCTION()
	void OnGetUpMontageEnded(UAnimMontage* montage, bool bInterrupted); // 일어서기 애니메이션 종료 시 실행되는 함수

	UPROPERTY(EditAnywhere, Category = "Intro|Animation")
	TObjectPtr<UAnimMontage> getUpMontage; // 일어서기 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "Intro|Animation", meta = (ClampMin = "0.01"))
	float getUpPlayRate = 1.f; // 일어서기 애니메이션 몽타주 재생 속도


	/* 연출(제어) */
	UPROPERTY(EditAnywhere, Category = "Intro|Control", meta = (ClampMin = "0.0"))
	float fallbackGetUpDuration = 3.f; // 조종 잠금 유지 시간


	/* 연출(대화 시스템) */
	UFUNCTION()
	void OnIntroDialogueFinished(); // 인트로 독백 종료 시 실행되는 함수

	UPROPERTY(EditAnywhere, Category = "Intro|Dialogue")
	TObjectPtr<UDataTable> introDialogueTable; // 인트로 독백 대사 테이블


	/* 연출(기타) */
	UPROPERTY(EditInstanceOnly, Category = "Intro|Staging")
	TArray<TObjectPtr<ABaseStagingActor>> introStagingEffects; // 인트로 시작 시 함께 재생할 연출 목록

private:
	/* 캐시 변수 */
	void CachePlayer(); // 플레이어와 컨트롤러를 캐싱하는 함수
	UPROPERTY()
	TObjectPtr<AWindlessCharacter> playerCharacter; // 플레이어 캐릭터

	UPROPERTY()
	TObjectPtr<APlayerController> playerController; // 플레이어 컨트롤러

	UPROPERTY()
	TObjectPtr<UAnimInstance> cachedAnimInstance; // 애니메이션 인스턴스


	/* 인트로 제어 */
	void FinishIntro(); // 인트로를 종료하는 함수

	
	/* 연출(페이드) */
	void StartFadeIn(); // 페이드인을 시작하는 함수


	/* 연출(애니메이션) */
	void PlayGetUp(); // 일어서기 애니메이션을 재생하는 함수
	void OnGetUpComplete(); // 일어서기 애니메이션 재생 완료 후 조종 활성화 및 독백을 진행하는 함수
	FTimerHandle getUpFallbackTimer; // 일어서기 애니메이션 대체 타이머


	/* 연출(제어) */
	void EnablePlayerControl(); // 플레이어 이동 제어를 활성화하는 함수
	void DisablePlayerControl(); // 플레이어 이동 제어를 비활성화하는 함수

	
	/* 연출(기타) */
	void PlayStagingEffects(); // 연출 목록들을 재생하는 함수
};