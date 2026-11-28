#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerProgressionComponent.generated.h"

class UAttributeComponent;
class UBodyPartComponent;
class UInventoryComponent;

/*
 * 플레이어 성장 및 휴식 컴포넌트
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UPlayerProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerProgressionComponent();


	/* 탈피(레벨업) */
	UFUNCTION(BlueprintCallable, Category = "Molting")
	bool TryMolt(); // 탈피를 시도하는 함수

	UFUNCTION(BlueprintCallable, Category = "Molting")
	void ApplyLevelToAllStats(); // 레벨에 맞는 스탯, 부위 체력, 가방 크기를 적용하는 함수

	UFUNCTION(BlueprintCallable, Category = "Molting")
	void RefreshInventorySizeForLevel(); // 레벨에 맞는 가방 크기를 적용하는 함수


	/* 휴식 */
	UFUNCTION(BlueprintCallable, Category = "SavePoint")
	void RestAtSavePoint(); // 세이브포인트 휴식 효과를 적용하는 함수

	void RefillRecoveryItem(); // 회복 아이템을 채우는 함수

protected:
	virtual void BeginPlay() override;

private:
	/* 캐시 변수 */
	UPROPERTY()
	TObjectPtr<UAttributeComponent> attributes = nullptr; // 플레이어 속성

	UPROPERTY()
	TObjectPtr<UBodyPartComponent> bodyParts = nullptr; // 플레이어 부위 체력

	UPROPERTY()
	TObjectPtr<UInventoryComponent> inventory = nullptr; // 플레이어 인벤토리 


	/* 휴식 */
	int32 GetRecoveryChargesForLevel() const; // 회복 아이템의 최대 충전 횟수를 반환하는 함수

	UPROPERTY(EditDefaultsOnly, Category = "Recovery", meta = (ClampMin = "0"))
	int32 baseRecoveryCharges = 3; // 회복 아이템 기본 충전 횟수

	UPROPERTY(EditDefaultsOnly, Category = "Recovery", meta = (ClampMin = "1"))
	int32 levelsPerExtraCharge = 5; // 충전 횟수가 1 늘어나는 레벨 간격

	UPROPERTY(EditDefaultsOnly, Category = "Recovery", meta = (ClampMin = "1"))
	int32 maxRecoveryCharges = 10; // 회복 아이템 최대 충전 횟수
};