#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Repairs/RepairTypes.h"
#include "RepairComponent.generated.h"

class UInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRepairPerformed, int64, targetUID, float, usedCharge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepairToolDepleted, int64, toolUID);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API URepairComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URepairComponent();

	/* 조회 */
	UFUNCTION(BlueprintPure, Category = "Repair")
	TArray<FRepairToolEntry> GetRepairTools() const; // 보유 수리도구 목록을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Repair")
	TArray<FRepairTargetEntry> GetRepairTargets() const; // 수리 가능한 장착 장비 목록을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Repair")
	bool IsRepairTool(int64 instanceUID) const; // 해당 인스턴스가 수리도구인지 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "Repair")
	float GetRequiredCharge(int64 targetUID) const; // 완전 수리에 필요한 충전량을 반환하는 함수


	/* 실행 */
	UFUNCTION(BlueprintCallable, Category = "Repair")
	bool RepairWithTool(int64 toolUID, int64 targetUID); // 수리도구로 장비를 수리하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Repair|Event")
	FOnRepairPerformed onRepairPerformed; // 수리 성공 시 실행되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Repair|Event")
	FOnRepairToolDepleted onRepairToolDepleted; // 수리도구 충전량 소진 시 실행되는 델리게이트

private:
	UInventoryComponent* GetOwnerInventory() const;
};