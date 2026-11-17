#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RefineComponent.generated.h"

class UInventoryComponent;
class UAttributeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRefined, int32, gainedStones, int32, totalStones);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API URefineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URefineComponent();
	
	
	/* 정제 */
	UFUNCTION(BlueprintPure, Category = "Refine")
	bool CanRefine(UInventoryComponent* inventory, int64 instanceUID) const; // 정제 가능한 아이템인지 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "Refine")
	int32 GetRefineValue(UInventoryComponent* inventory, int64 instanceUID, int32 quantity = -1) const; // 정제 시 얻는 탈피석을 계산하는 함수

	UFUNCTION(BlueprintCallable, Category = "Refine")
	bool Refine(UInventoryComponent* inventory, int64 instanceUID, int32 quantity = -1); // 정제를 실행하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Refine|Event")
	FOnItemRefined onItemRefined; // 정제 성공 시 실행되는 델리게이트

private:
	UAttributeComponent* GetOwnerAttributes() const; // 소유자 속성 컴포넌트를 반환하는 함수
};