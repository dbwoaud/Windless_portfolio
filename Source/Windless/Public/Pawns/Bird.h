#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Bird.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class WINDLESS_API ABird : public APawn
{
	GENERATED_BODY()

public:
	ABird();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // 향상된 입력 기능을 사용하기 위한 함수

protected:
	virtual void BeginPlay() override;

	/* 이동 시스템 */
	void Move(const FInputActionValue& value); // 이동 입력 시 실행되는 함수
	void Look(const FInputActionValue& value); // 회전 입력 시 실행되는 함수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= Input)
	UInputMappingContext* birdMappingContext; // 입력 매핑 컨텍스트

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* moveAction; // 이동 입력 액션

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* lookAction; // 회전 입력 액션

private:
	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* capsule; // 캡슐 콜라이더 컴포넌트

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* birdMesh; // 스켈레탈 메쉬 컴포넌트 

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* springArm; // 스프링 암 컴포넌트

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* viewCamera; // 카메라 컴포넌트
};