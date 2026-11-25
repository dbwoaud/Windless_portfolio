#include "Items/Database/ItemDatabaseSubsystem.h"
#include "Items/Settings/WindlessItemSettings.h"
#include "Windless/DebugMacro.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"

void UItemDatabaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BuildDatabase();
}

void UItemDatabaseSubsystem::Deinitialize()
{
	itemDatabase.Empty();
	Super::Deinitialize();
}

bool UItemDatabaseSubsystem::GetItemData(FName itemId, FItemData& outData) const
{
	if (itemId.IsNone())
		return false;

	if (const FItemData* found = itemDatabase.Find(itemId))
	{
		outData = *found;
		return true;
	}

	return false;
}

bool UItemDatabaseSubsystem::HasItem(FName itemId) const
{
	return !itemId.IsNone() && itemDatabase.Contains(itemId);
}

void UItemDatabaseSubsystem::RegisterItem(const FItemData& data)
{
	if (data.itemId.IsNone())
		return;

	itemDatabase.FindOrAdd(data.itemId) = data;
}

void UItemDatabaseSubsystem::Reload()
{
	BuildDatabase();
}

UItemDatabaseSubsystem* UItemDatabaseSubsystem::Get(const UObject* worldContext)
{
	if (!worldContext)
		return nullptr;

	const UWorld* world = worldContext->GetWorld();
	if (!world)
		return nullptr;

	UGameInstance* gameInstance = world->GetGameInstance();
	return gameInstance ? gameInstance->GetSubsystem<UItemDatabaseSubsystem>() : nullptr;
}

void UItemDatabaseSubsystem::BuildDatabase()
{
	itemDatabase.Empty();

	static const FString ctx(TEXT("ItemDatabaseSubsystem"));
	int32 loadedTables = 0;

	const UWindlessItemSettings* settings = UWindlessItemSettings::Get();
	if (settings)
	{
		for (const TSoftObjectPtr<UDataTable>& softTable : settings->itemDataTables)
		{
			UDataTable* table = softTable.LoadSynchronous();
			if (!table || table->GetRowStruct() != FItemData::StaticStruct())
				continue;

			for (const FName& rowName : table->GetRowNames())
			{
				const FItemData* row = table->FindRow<FItemData>(rowName, ctx);
				if (!row)
					continue;

				FItemData data = *row;
				if (data.itemId == NAME_None)
					data.itemId = rowName;

				if (itemDatabase.Contains(data.itemId))
					continue;

				itemDatabase.Add(data.itemId, data);
			}

			++loadedTables;
		}
	}

	if (loadedTables == 0)
		return;

	UE_LOG(LogWindless, Log, TEXT("[아이템 DB] 테이블 %d개에서 아이템 %d종 로드"), loadedTables, itemDatabase.Num());
}