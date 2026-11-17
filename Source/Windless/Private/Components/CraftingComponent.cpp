#include "Components/CraftingComponent.h"
#include "Components/InventoryComponent.h"
#include "Items/Storage/SharedStorageSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Windless/DebugMacro.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<FCraftRecipeEntry> UCraftingComponent::GetRecipeEntries(ECraftCategory category, bool bFieldOnly) const
{
	TArray<FCraftRecipeEntry> result;

	UInventoryComponent* inventory = GetOwnerInventory();
	if (!inventory)
		return result;

	for (const FCraftRecipe& recipe : recipes)
	{
		if (recipe.category != category)
			continue;

		if (bFieldOnly && !recipe.bFieldCraftable)
			continue;

		FCraftRecipeEntry entry;
		entry.recipeId = recipe.recipeId;
		entry.displayName = recipe.displayName;
		entry.maxCraftableCount = GetMaxCraftableCount(recipe.recipeId);
		entry.blockReason = GetBlockReason(recipe.recipeId, 1);
		entry.bCraftable = (entry.blockReason == ECraftBlockReason::ECBR_None);

		// 결과 요약
		if (recipe.bStorageUpgrade)
		{
			entry.resultSummary = FText::FromString(FString::Printf(TEXT("%s 창고 (%dx%d)"),
				*GetStorageGradeName(recipe.targetStorageGrade).ToString(),
				StorageGridWidth, GetStorageGradeHeight(recipe.targetStorageGrade)));
		}
		else
		{
			FItemData resultData;
			if (inventory->GetItemData(recipe.resultItemId, resultData))
				entry.resultSummary = FText::FromString(FString::Printf(TEXT("%s x%d"),
					*resultData.itemName.ToString(), recipe.resultCount));
		}

		// 재료 요약
		TArray<FString> parts;
		for (const FCraftIngredient& ing : recipe.ingredients)
		{
			FItemData ingData;
			const FString name = inventory->GetItemData(ing.itemId, ingData)
				? ingData.itemName.ToString()
				: ing.itemId.ToString();

			parts.Add(FString::Printf(TEXT("%s %d/%d"), *name, inventory->GetItemTotalCount(ing.itemId), ing.quantity));
		}
		entry.ingredientSummary = FText::FromString(FString::Join(parts, TEXT("   ")));

		result.Add(entry);
	}

	// 제작 가능한 레시피 상단 배치
	result.Sort([](const FCraftRecipeEntry& a, const FCraftRecipeEntry& b)
		{
			if (a.bCraftable != b.bCraftable)
				return a.bCraftable;
			return a.displayName.ToString() < b.displayName.ToString();
		});

	return result;
}

int32 UCraftingComponent::GetMaxCraftableCount(FName recipeId) const
{
	const FCraftRecipe* recipe = FindRecipe(recipeId);
	if (!recipe)
		return 0;

	// 창고 업그레이드
	if (recipe->bStorageUpgrade)
	{
		const UWorld* world = GetWorld();
		const USharedStorageSubsystem* storage = world ? world->GetSubsystem<USharedStorageSubsystem>() : nullptr;
		if (!storage)
			return 0;

		const bool bNextGrade = (static_cast<int32>(recipe->targetStorageGrade) == static_cast<int32>(storage->GetGrade()) + 1);
		return bNextGrade ? FMath::Min(1, GetMaterialLimit(*recipe)) : 0;
	}

	return FMath::Min(GetMaterialLimit(*recipe), GetSpaceLimit(*recipe));
}

ECraftBlockReason UCraftingComponent::GetBlockReason(FName recipeId, int32 count) const
{
	const FCraftRecipe* recipe = FindRecipe(recipeId);
	if (!recipe || count <= 0)
		return ECraftBlockReason::ECBR_Invalid;

	if (recipe->bStorageUpgrade)
	{
		const UWorld* world = GetWorld();
		const USharedStorageSubsystem* storage = world ? world->GetSubsystem<USharedStorageSubsystem>() : nullptr;
		if (!storage)
			return ECraftBlockReason::ECBR_Invalid;

		if (static_cast<int32>(recipe->targetStorageGrade) != static_cast<int32>(storage->GetGrade()) + 1)
			return ECraftBlockReason::ECBR_GradeMismatch;
	}

	if (GetMaterialLimit(*recipe) < count)
		return ECraftBlockReason::ECBR_NoMaterial;

	if (GetSpaceLimit(*recipe) < count)
		return ECraftBlockReason::ECBR_NoSpace;

	return ECraftBlockReason::ECBR_None;
}

bool UCraftingComponent::Craft(FName recipeId, int32 count)
{
	const FCraftRecipe* recipe = FindRecipe(recipeId);
	if (!recipe || count <= 0)
		return false;

	const ECraftBlockReason reason = GetBlockReason(recipeId, count);
	if (reason != ECraftBlockReason::ECBR_None)
	{
		UE_LOG(LogWindless, Warning, TEXT("[제작] 차단: %s (사유 %d)"), *recipeId.ToString(), (int32)reason);
		return false;
	}

	if (!ConsumeIngredients(*recipe, count))
		return false;

	// 창고 등급 업그레이드
	if (recipe->bStorageUpgrade)
	{
		UWorld* world = GetWorld();
		USharedStorageSubsystem* storage = world ? world->GetSubsystem<USharedStorageSubsystem>() : nullptr;
		if (!storage || !storage->UpgradeTo(recipe->targetStorageGrade))
			return false;

		UE_LOG(LogWindless, Log, TEXT("[제작] 창고 업그레이드 완료 -> %s"), *GetStorageGradeName(recipe->targetStorageGrade).ToString());
		onCrafted.Broadcast(recipeId, count);
		return true;
	}

	// 일반 아이템 제작
	UInventoryComponent* inventory = GetOwnerInventory();
	const int32 totalResult = recipe->resultCount * count;

	if (!inventory->AddItemById(recipe->resultItemId, totalResult))
		return false;

	UE_LOG(LogWindless, Log, TEXT("[제작] %s x%d 완료"), *recipe->resultItemId.ToString(), totalResult);
	onCrafted.Broadcast(recipeId, count);
	return true;
}

void UCraftingComponent::PrintDebugInfo() const
{
	UE_LOG(LogWindless, Warning, TEXT("===== 제작 레시피 %d종 ====="), recipes.Num());

	for (const FCraftRecipe& recipe : recipes)
	{
		UE_LOG(LogWindless, Warning, TEXT("[%s] %-18s 최대 %d회 (사유 %d)"),
			*recipe.recipeId.ToString(), *recipe.displayName.ToString(),
			GetMaxCraftableCount(recipe.recipeId), (int32)GetBlockReason(recipe.recipeId, 1));
	}
}


void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!recipeTable)
		return;

	recipes.Reset();
	for (const TPair<FName, uint8*>& pair : recipeTable->GetRowMap())
	{
		if (const FCraftRecipe* row = reinterpret_cast<const FCraftRecipe*>(pair.Value))
		{
			FCraftRecipe copy = *row;
			if (copy.recipeId.IsNone())
				copy.recipeId = pair.Key;
			recipes.Add(copy);
		}
	}

	UE_LOG(LogWindless, Log, TEXT("[제작] 레시피 %d종 로드"), recipes.Num());
}

const FCraftRecipe* UCraftingComponent::FindRecipe(FName recipeId) const
{
	return recipes.FindByPredicate([recipeId](const FCraftRecipe& r) { return r.recipeId == recipeId; });
}

UInventoryComponent* UCraftingComponent::GetOwnerInventory() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UInventoryComponent>() : nullptr;
}

int32 UCraftingComponent::GetMaterialLimit(const FCraftRecipe& recipe) const
{
	UInventoryComponent* inventory = GetOwnerInventory();
	if (!inventory || recipe.ingredients.IsEmpty())
		return 0;

	int32 limit = MAX_int32;
	for (const FCraftIngredient& ing : recipe.ingredients)
	{
		if (ing.itemId.IsNone() || ing.quantity <= 0)
			return 0;

		limit = FMath::Min(limit, inventory->GetItemTotalCount(ing.itemId) / ing.quantity);
	}
	return FMath::Max(0, limit);
}

int32 UCraftingComponent::GetSpaceLimit(const FCraftRecipe& recipe) const
{
	if (recipe.bStorageUpgrade || recipe.resultItemId.IsNone())
		return MAX_int32;

	UInventoryComponent* inventory = GetOwnerInventory();
	if (!inventory)
		return 0;

	FItemData resultData;
	if (!inventory->GetItemData(recipe.resultItemId, resultData))
		return 0;

	int32 stackRoom = 0;
	if (resultData.bStackable && resultData.maxStack > 1)
	{
		for (const FItemInstance& inst : inventory->GetItems())
		{
			if (inst.itemId == recipe.resultItemId && !inst.bIsEquipped)
				stackRoom += FMath::Max(0, resultData.maxStack - inst.currentStackCount);
		}
	}

	const int32 footprint = FMath::Max(1, resultData.width * resultData.height);
	const int32 newStacks = inventory->GetFreeCellCount() / footprint;
	const int32 perStack = resultData.bStackable ? FMath::Max(1, resultData.maxStack) : 1;

	const int32 totalRoom = stackRoom + newStacks * perStack;
	return FMath::Max(0, totalRoom / FMath::Max(1, recipe.resultCount));
}

bool UCraftingComponent::ConsumeIngredients(const FCraftRecipe& recipe, int32 count)
{
	UInventoryComponent* inventory = GetOwnerInventory();
	if (!inventory)
		return false;

	for (const FCraftIngredient& ing : recipe.ingredients)
	{
		const int32 need = ing.quantity * count;
		const int32 removed = inventory->RemoveItemById(ing.itemId, need);

		if (removed < need)
			return false;
	}
	return true;
}

