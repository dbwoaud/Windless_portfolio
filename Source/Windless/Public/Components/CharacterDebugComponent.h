#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterDebugComponent.generated.h"

class UEnhancedInputComponent;
class UInputAction;
class UAttributeComponent;
class UBodyPartComponent;
class UInventoryComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WINDLESS_API UCharacterDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterDebugComponent();
	void BindDebugInputs(UEnhancedInputComponent* enhancedInputComponent); // 디버그용 입력 액션을 할당하는 함수


	/* 스테이터스 시스템 */
	UFUNCTION()
	void InvStats(); // 스테이터스를 출력하는 함수(디버그용)


	/* 부위 시스템 */
	UFUNCTION()
	void InvHurt(int32 part, float amount);// 부위에 데미지를 입히는 함수(디버그용)

	UFUNCTION()
	void InvInjure(int32 part, int32 injury); // 부위에 부상을 확정 발동시키는 함수(디버그용)

	UFUNCTION()
	void InvSaveZone(bool bActive); // 세이브 포인트 구역 상태를 토글하는 함수(디버그용)


	/* 인벤토리 시스템 */
	UFUNCTION()
	void InvAdd(const FString& itemId, int32 count = 1); // 아이템을 획득하는 함수(디버그용)

	UFUNCTION()
	void InvEquip(const FString& itemId, int32 slot = 0); // 장비 슬롯을 지정하여 장비를 장착하는 함수(디버그용)

	UFUNCTION()
	void InvUnequip(int32 slot); // 장비 슬롯을 해제하는 함수(디버그용)

	UFUNCTION()
	void InvUse(const FString& itemId); // 소비 아이템을 사용하는 함수(디버그용)

	UFUNCTION()
	void InvDur(int32 slot, float amount); // 장비의 내구도를 감소시키는 함수(디버그용)

	UFUNCTION()
	void InvRepair(int32 slot, float amount); // 장비의 내구도를 수리하는 함수(디버그용)

	UFUNCTION()
	void InvMove(int32 uid, int32 x, int32 y, bool bRot); // 인벤토리 내 아이템을 이동/회전시키는 함수(디버그용)

	UFUNCTION()
	void InvPrint(); // 인벤토리를 출력하는 함수(디버그용)

	UFUNCTION()
	void InvSplit(const FString& itemId, int32 amount); // 아이템을 나누는 함수(디버그용)

	UFUNCTION()
	void InvDrop(const FString& itemId, int32 quantity); // 아이템을 버리는 함수(디버그용)

	UFUNCTION()
	void InvLimit(); // 소지 개수 제한 현황을 출력하는 함수(디버그용)

protected:
	virtual void BeginPlay() override;

private:	
	/* 컴포넌트 */
	UPROPERTY()
	UAttributeComponent* ownerAttributes = nullptr; // 같은 액터의 속성 컴포넌트

	UPROPERTY()
	UBodyPartComponent* ownerBodyParts = nullptr; // 같은 액터의 부위 컴포넌트

	UPROPERTY()
	UInventoryComponent* ownerInventory = nullptr; // 같은 액터의 인벤토리 컴포넌트


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