#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/Crafts/CraftTypes.h"
#include "CraftingComponent.generated.h"

class UInventoryComponent;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrafted, FName, recipeId, int32, craftedCount);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingComponent();

	/* 조회 */
	UFUNCTION(BlueprintPure, Category = "Craft")
	TArray<FCraftRecipeEntry> GetRecipeEntries(ECraftCategory category, bool bFieldOnly = false) const; // 분류별 레시피 목록을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Craft")
	int32 GetMaxCraftableCount(FName recipeId) const; // 최대 제작 횟수를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Craft")
	ECraftBlockReason GetBlockReason(FName recipeId, int32 count) const; // 제작 불가 사유를 반환하는 함수


	/* 제작 */
	UFUNCTION(BlueprintCallable, Category = "Craft")
	bool Craft(FName recipeId, int32 count); // 제작을 실행하는 함수


	/* 디버그 */
	void PrintDebugInfo() const; // 레시피 상태를 로그로 출력하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Craft|Event")
	FOnCrafted onCrafted; // 제작 성공 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Craft")
	TObjectPtr<UDataTable> recipeTable = nullptr; // 레시피 데이터 테이블

private:
	/* 캐시 변수 */
	UPROPERTY()
	TArray<FCraftRecipe> recipes; // 캐시한 레시피 목록


	/* 조회 */
	const FCraftRecipe* FindRecipe(FName recipeId) const; // 레시피를 찾는 함수
	UInventoryComponent* GetOwnerInventory() const; // 소유 인벤토리를 반환하는 함수


	/* 제작 */
	int32 GetMaterialLimit(const FCraftRecipe& recipe) const; // 재료 기준 최대 제작 횟수를 반환하는 함수
	int32 GetSpaceLimit(const FCraftRecipe& recipe) const;    // 인벤토리 공간 기준 최대 제작 횟수를 반환하는 함수
	bool ConsumeIngredients(const FCraftRecipe& recipe, int32 count); // 재료를 소모하는 함수

};