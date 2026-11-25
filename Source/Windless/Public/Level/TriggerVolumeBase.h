#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TriggerVolumeBase.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerEnterSignature, AActor*, activator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTriggerExitSignature, AActor*, activator);


UCLASS()
class WINDLESS_API ATriggerVolumeBase : public AActor
{
	GENERATED_BODY()

public:
	ATriggerVolumeBase();


	/* 트리거 설정 */
	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void ResetTrigger(); // 트리거를 리셋하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Trigger|Event")
	FOnTriggerEnterSignature onTriggerEnter; // 트리거 박스 진입 시 실행되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Trigger|Event")
	FOnTriggerExitSignature onTriggerExit; // 트리거 박스 이탈 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;


	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	UBoxComponent* triggerBox; // 박스 컴포넌트


	/* 트리거 설정 */
	UPROPERTY(EditAnywhere, Category = "Trigger|Settings")
	bool bTriggerOnce = true; // 최초 1회만 트리거를 발동할 지 여부

	UPROPERTY(EditAnywhere, Category = "Trigger|Settings")
	bool bPlayerOnly = true; // 플레이어 캐릭터만 감지할 지 여부

	UPROPERTY(EditAnywhere, Category = "Trigger|Settings")
	FName requiredActorTag = NAME_None; // 트리거를 감지할 액터 이름


	/* 트리거 진입 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trigger|Event")
	void OnTriggerEnterBP(AActor* activator); // 트리거 진입 시 실행되는 블루프린트용 함수

	virtual void HandleTriggerEnter(AActor* activator); // 트리거 진입 시 실행되는 C++ 코드용 함수


	/* 트리거 이탈 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Trigger|Event")
	void OnTriggerExitBP(AActor* activator); // 트리거 이탈 시 실행되는 블루프린트용 함수

	virtual void HandleTriggerExit(AActor* activator); // 트리거 이탈 시 실행되는 C++ 코드용 함수


	/* 오버랩 */
	UFUNCTION()
	// 박스 컴포넌트에 오버랩 시작 시 실행되는 함수
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 박스 컴포넌트에 오버랩 완료 시 실행되는 함수
	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool IsValidActivator(AActor* other) const; // 감지 대상 액터인지 검사하는 함수

	bool bHasTriggered = false; // 트리거 발동 여부
};