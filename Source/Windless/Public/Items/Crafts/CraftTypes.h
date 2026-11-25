#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Level/StorageTypes.h"
#include "CraftTypes.generated.h"

/* 제작 레시피 분류 */
UENUM(BlueprintType)
enum class ECraftCategory : uint8
{
	ECC_Consumable UMETA(DisplayName = "소비"),   // 소비 아이템
	ECC_Equipment  UMETA(DisplayName = "장비"),   // 장비
	ECC_Tool       UMETA(DisplayName = "도구"),   // 수리도구 등
	ECC_Facility   UMETA(DisplayName = "시설"),   // 창고 업그레이드 등
	ECC_MAX        UMETA(Hidden)
};

/* 레시피 재료 1종 */
USTRUCT(BlueprintType)
struct FCraftIngredient
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft")
	FName itemId = NAME_None; // 재료 아이템 ID

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft", meta = (ClampMin = "1"))
	int32 quantity = 1; // 필요 개수
};

/* 제작 레시피 */
USTRUCT(BlueprintType)
struct FCraftRecipe : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft")
	FName recipeId = NAME_None; // 레시피 고유 ID

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft")
	FText displayName; // 표시 이름

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft")
	ECraftCategory category = ECraftCategory::ECC_Consumable; // 분류 탭

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft")
	FName resultItemId = NAME_None; // 결과 아이템 ID

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft", meta = (ClampMin = "1"))
	int32 resultCount = 1; // 1회 제작으로 얻는 개수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft")
	TArray<FCraftIngredient> ingredients; // 필요 재료 목록

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft")
	bool bFieldCraftable = false; // 휴대용 제작도구로 필드에서 제작 가능한지


	/* 창고 업그레이드 전용  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft|Facility")
	bool bStorageUpgrade = false; // 창고 등급 업그레이드 레시피인지

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Craft|Facility")
	EStorageGrade targetStorageGrade = EStorageGrade::ESG_Medium; // 올라갈 창고 등급
};

/* 제작 불가 사유 */
UENUM(BlueprintType)
enum class ECraftBlockReason : uint8
{
	ECBR_None          UMETA(DisplayName = "가능"),
	ECBR_NoMaterial    UMETA(DisplayName = "재료 부족"),
	ECBR_NoSpace       UMETA(DisplayName = "공간 부족"),
	ECBR_GradeMismatch UMETA(DisplayName = "이전 등급 필요"),
	ECBR_Invalid       UMETA(DisplayName = "레시피 오류")
};

/* 제작 화면 목록 항목 */
USTRUCT(BlueprintType)
struct FCraftRecipeEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Craft")
	FName recipeId = NAME_None; // 레시피 ID

	UPROPERTY(BlueprintReadOnly, Category = "Craft")
	FText displayName; // 표시 이름

	UPROPERTY(BlueprintReadOnly, Category = "Craft")
	FText resultSummary; // 결과 요약

	UPROPERTY(BlueprintReadOnly, Category = "Craft")
	FText ingredientSummary; // 재료 요약

	UPROPERTY(BlueprintReadOnly, Category = "Craft")
	int32 maxCraftableCount = 0; // 현재 재료/공간으로 만들 수 있는 최대 횟수

	UPROPERTY(BlueprintReadOnly, Category = "Craft")
	ECraftBlockReason blockReason = ECraftBlockReason::ECBR_None; // 불가 사유

	UPROPERTY(BlueprintReadOnly, Category = "Craft")
	bool bCraftable = false; // 1회 이상 제작 가능한지
};