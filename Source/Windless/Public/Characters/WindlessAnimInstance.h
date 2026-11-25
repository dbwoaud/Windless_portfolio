#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterTypes.h"
#include "WindlessAnimInstance.generated.h"

class AWindlessCharacter;
class UCharacterMovementComponent;

UCLASS()
class WINDLESS_API UWindlessAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float deltaTime) override;

	UPROPERTY(BlueprintReadOnly)
	AWindlessCharacter* windlessCharacter; // 플레이어 캐릭터 인스턴스

	UPROPERTY(BlueprintReadOnly, Category = State)
	ECharacterState characterState; // 플레이어 캐릭터의 무기 장착 상태

	UPROPERTY(BlueprintReadOnly, Category = State)
	EActionState actionState; // 플레이어 캐릭터의 상태

	UPROPERTY(BlueprintReadOnly, Category = "State|Death")
	TEnumAsByte<EDeathPose> deathPose; // 플레이어 캐릭터의 죽음 애니메이션 포즈 목록

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	UCharacterMovementComponent* windlessCharacterMovement; //플레이어 캐릭터 인스턴스의 이동 컴포넌트

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float groundSpeed; // 플레이어 캐릭터의 현재 속도

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool isFalling; // 플레이어 캐릭터가 추락 중인지 확인하는 변수

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool isCrouched; // 플레이어 캐릭터가 숙인 상태인지 확인하는 변수

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool isMounted; // 플레이어 캐릭터가 탑승 상태인지 확인하는 변수

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsStrafing; // 락온 상태인지 확인하는 변수

	UPROPERTY(BlueprintReadOnly, Category = Movement)
	float moveDirection; // 캐릭터 이동 방향

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	bool bIsGuarding; // 플레이어 캐릭터가 가드 중인지 확인하는 변수

	UPROPERTY(BlueprintReadOnly, Category = Combat)
	bool bGuardOnLeft; // 플레이어 캐릭터의 가드 손이 왼손인지 확인하는 변수
};
