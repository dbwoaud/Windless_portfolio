#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "WindlessItemSettings.generated.h"

class UDataTable;

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Item"))
class WINDLESS_API UWindlessItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FName("Windless"); }

	UPROPERTY(config, EditAnywhere, Category = "Database")
	TArray<TSoftObjectPtr<UDataTable>> itemDataTables; // 아이템 원본 데이터 테이블 목록

	static const UWindlessItemSettings* Get() { return GetDefault<UWindlessItemSettings>(); }
};