#pragma once

#include "CoreMinimal.h"
#include "Items/WorldItem.h"
#include "DropItem.generated.h"

UCLASS()
class WINDLESS_API ADropItem : public AWorldItem
{
	GENERATED_BODY()

public:
	ADropItem();
	virtual bool CanInteract_Implementation(AActor* interactor) override;


	/* 아이템 버리기 */
	UFUNCTION(BlueprintCallable, Category = "DroppedItem")
	void DropItem(const FVector& direction); // 시선 방향으로 아이템을 버리는 함수

protected:
	virtual void BeginPlay() override;


	/* 버리기 설정 */
	UPROPERTY(EditAnywhere, Category = "DroppedItem|Launch", meta = (ClampMin = "0.0"))
	float dropSpeed = 320.f; // 투척 속도

	UPROPERTY(EditAnywhere, Category = "DroppedItem|Launch", meta = (ClampMin = "0.0"))
	float dropRatio = 0.35f; // 위쪽 성분 비율

	UPROPERTY(EditAnywhere, Category = "DroppedItem|Launch", meta = (ClampMin = "0.0"))
	float settleCheckInterval = 0.25f; // 아이템 정지 판정 주기

	UPROPERTY(EditAnywhere, Category = "DroppedItem|Launch", meta = (ClampMin = "0.0"))
	float settleSpeedThreshold = 8.f; // 아이템 정지 판정 속도 임계값


	/* 획득 잠금 */
	UPROPERTY(EditAnywhere, Category = "DroppedItem|Pickup", meta = (ClampMin = "0.0"))
	float pickupLockDelay = 0.8f; // 버린 직후 재획득 방지 시간

private:
	/* 버리기 설정 */
	void CheckSettled(); // 물리 정지 여부를 검사하는 함수

	FTimerHandle settleTimer; // 정지 판정 타이머


	/* 획득 잠금 */
	void UnlockPickup(); // 획득 잠금을 해제하는 함수

	bool bPickupLocked = false; // 획득 잠금 상태
	FTimerHandle pickupLockTimer; // 획득 잠금 해제 타이머
};