#include "Components/PlayerProgressionComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/InventoryComponent.h"
#include "Characters/LevelUpTypes.h"
#include "Windless/DebugMacro.h"

UPlayerProgressionComponent::UPlayerProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UPlayerProgressionComponent::TryMolt()
{
	if (!attributes || !attributes->CanMolt())
		return false;

	if (!attributes->Molt())
		return false;

	ApplyLevelToAllStats();
	return true;
}

void UPlayerProgressionComponent::ApplyLevelToAllStats()
{
	if (!attributes)
		return;

	const int32 level = attributes->GetCurrentLevel();
	attributes->ApplyStatsForLevel(level);

	if (bodyParts)
		bodyParts->ApplyLevelToParts(level);

	RefreshInventorySizeForLevel();
}

void UPlayerProgressionComponent::RefreshInventorySizeForLevel()
{
	if (!attributes || !inventory)
		return;

	int32 newWidth = 0, newHeight = 0;
	WindlessLevelUp::GetInventorySizeForLevel(attributes->GetCurrentLevel(), newWidth, newHeight);

	if (newWidth != inventory->GetGridWidth() || newHeight != inventory->GetGridHeight())
		inventory->ResizeGrid(newWidth, newHeight);
}

void UPlayerProgressionComponent::RestAtSavePoint()
{
	if (attributes)
		attributes->ResetChaos();

	RefillRecoveryItem();
	UE_LOG(LogWindless, Log, TEXT("[세이브] 휴식 완료"));
}

void UPlayerProgressionComponent::RefillRecoveryItem()
{
	if (inventory && attributes)
		inventory->RefillRecoveryItem(GetRecoveryChargesForLevel());
}

void UPlayerProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* owner = GetOwner())
	{
		attributes = owner->FindComponentByClass<UAttributeComponent>();
		bodyParts = owner->FindComponentByClass<UBodyPartComponent>();
		inventory = owner->FindComponentByClass<UInventoryComponent>();
	}
}

int32 UPlayerProgressionComponent::GetRecoveryChargesForLevel() const
{
	const int32 level = attributes ? attributes->GetCurrentLevel() : 0;
	return FMath::Min(maxRecoveryCharges, baseRecoveryCharges + level / FMath::Max(1, levelsPerExtraCharge));
}
