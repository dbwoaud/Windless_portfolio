#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemTypes.h"
#include "ItemDropComponent.generated.h"

class ADropItem;
class UAnimMontage;
class UInventoryComponent;

DECLARE_DELEGATE_RetVal(bool, FDropMontageQuery);

/*
 * 아이템 버리기 컴포넌트
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UItemDropComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UItemDropComponent();


	/* 아이템 버리기 */
	UFUNCTION(BlueprintCallable, Category = "Item|Drop")
	void SpawnPendingDrop(); // 버린 아이템을 생성하는 함수


	/* 델리게이트 */
	FDropMontageQuery canPlayDropMontage; // 버리기 몽타주 재생 가능 여부를 반환하는 델리게이트

protected:
	virtual void BeginPlay() override;

private:
	/* 아이템 버리기 */
	UFUNCTION()
	void HandleItemDropped(FItemInstance droppedInstance); // 인벤토리에서 아이템을 버렸을 때 실행되는 함수

	UPROPERTY(EditDefaultsOnly, Category = "Item|Drop", meta = (ClampMin = "0.0"))
	float dropForwardDistance = 50.f; // 버리기 후 아이템 생성 거리

	UPROPERTY(EditDefaultsOnly, Category = "Item|Drop")
	float dropSpawnHeightOffset = 0.f; // 생성 높이 보정

	UPROPERTY(EditDefaultsOnly, Category = "Item|Drop", meta = (ClampMin = "0.1"))
	float dropFallbackDelay = 1.5f; // 버려진 아이템 생성 대기 시간

	FTimerHandle dropFallbackTimer; // 아이템 버리기 타이머


	/* 상태 확인 */
	bool CanPlayDropMontage() const; // 아이템 버리기 몽타주 재생 가능 여부를 확인하는 함수


	/* 아이템 정보 */
	UPROPERTY(EditDefaultsOnly, Category = "Item|Drop")
	TSubclassOf<ADropItem> dropItemClass; // 버린 아이템 액터 클래스

	FItemInstance pendingDropInstance; // 버린 아이템 정보


	/* 연출 */
	UPROPERTY(EditDefaultsOnly, Category = "Item|Drop")
	TObjectPtr<UAnimMontage> dropMontage; // 버리기 애니메이션 몽타주

};