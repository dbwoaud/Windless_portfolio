#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Items/ItemTypes.h"
#include "ItemDragDrop.generated.h"

class UInventoryComponent;

UENUM(BlueprintType)
enum class EDragSource : uint8
{
    EDS_None      UMETA(DisplayName = "None"),
    EDS_Grid      UMETA(DisplayName = "Grid"),
    EDS_EquipSlot UMETA(DisplayName = "EquipSlot"),
    EDS_QuickSlot UMETA(DisplayName = "QuickSlot")
};

UCLASS()
class WINDLESS_API UItemDragDrop : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
    /* 컴포넌트 */
    UPROPERTY()
    TObjectPtr<UInventoryComponent> sourceInventory = nullptr; // 인벤토리


    /* 아이템 */
    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    int64 instanceUID = 0; // 끌고 있는 아이템 인스턴스 ID

    
    /* 드래그 시작 설정 */
    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    EDragSource sourceType = EDragSource::EDS_None; // 출발지 종류

    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    EEquipSlot sourceSlot = EEquipSlot::EES_None; // 출발지가 장비 슬롯일 때의 슬롯

    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    EQuickSlot sourceQuickSlot = EQuickSlot::EQS_None; // 출발지가 퀵슬롯일 때의 슬롯

    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    int32 grabOffsetX = 0; // 드래그 시작 X 셀 위치

    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    int32 grabOffsetY = 0; // 드래그 시작 Y 셀 위치

    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    bool bRotated = false; // 드래그 중 회전 상태

    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    bool bSplitDrag = false; // 나누기 드래그 여부

    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    bool bUseOnly = false; // 이동/해제 없이 부위 사용만 허용하는 드래그인지
};