#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StorageHolder.generated.h"

class UInventoryComponent;

UCLASS(NotBlueprintable)
class WINDLESS_API AStorageHolder : public AActor
{
	GENERATED_BODY()

public:
	AStorageHolder();

private:
	UPROPERTY(VisibleAnywhere, Category = "Storage")
	TObjectPtr<UInventoryComponent> storageInventory = nullptr; // 통합 창고 인벤토리

public:
	/* 변수 Getter 함수 */
	UFUNCTION(BlueprintPure, Category = "Storage")
	FORCEINLINE UInventoryComponent* GetInventory() const { return storageInventory; }
};