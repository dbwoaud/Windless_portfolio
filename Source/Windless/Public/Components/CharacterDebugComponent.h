#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterDebugComponent.generated.h"

class UEnhancedInputComponent;
class UInputAction;
class UAttributeComponent;
class UBodyPartComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WINDLESS_API UCharacterDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterDebugComponent();
	void BindDebugInputs(UEnhancedInputComponent* enhancedInputComponent); // 디버그용 입력 액션을 할당하는 함수

protected:
	virtual void BeginPlay() override;

private:	
	/* 컴포넌트 */
	UPROPERTY()
	UAttributeComponent* ownerAttributes = nullptr; // 같은 액터의 속성 컴포넌트

	UPROPERTY()
	UBodyPartComponent* ownerBodyParts = nullptr; // 같은 액터의 부위 컴포넌트


	/* 혼돈 시스템 */
	void DebugRecoverChaos(); // 정신력을 회복하는 함수(디버그용)
	void DebugLoseChaos(); // 정신력을 감소시키는 함수(디버그용)

	UPROPERTY(EditAnywhere, Category = "Debug Input|Chaos")
	UInputAction* recoverChaosAction; // 정신력 회복 입력 액션(디버그용)

	UPROPERTY(EditAnywhere, Category = "Debug Input|Chaos")
	UInputAction* loseChaosAction;    // 정신력 감소 입력 액션(디버그용)

	UPROPERTY(EditAnywhere, Category = "Debug|Chaos")
	float chaosDebugStep = 20.f; // 정신력 디버그 1회 증감량


	/* 부위 시스템 */
	void DebugDamageSelectedPart(); // 선택 부위에 대미지를 적용하는 함수(디버그용)
	void DebugCyclePart(); // 대미지 적용 부위를 순환 선택하는 함수(디버그용)
	void DebugHealAllParts(); // 모든 부위를 회복하는 함수(디버그용)

	UPROPERTY(EditAnywhere, Category = "Debug Input|BodyPart")
	UInputAction* damagePartAction; // 선택 부위 대미지 적용 입력 액션(디버그용)

	UPROPERTY(EditAnywhere, Category = "Debug Input|BodyPart")
	UInputAction* cyclePartAction; // 대상 부위 변경 입력 액션(디버그용)

	UPROPERTY(EditAnywhere, Category = "Debug Input|BodyPart")
	UInputAction* healPartsAction; // 전체 부위 회복 입력 액션(디버그용)

	UPROPERTY(EditAnywhere, Category = "Debug|BodyPart")
	float partDamageStep = 25.f; // 부위 디버그 1회 대미지량

	int32 selectedPart = 1; // 현재 선택된 디버그 부위
};