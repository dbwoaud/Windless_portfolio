#pragma once

#include "CoreMinimal.h"
#include "Cheats/WindlessCheatExtension.h"
#include "InventoryCheats.generated.h"

/*
 * 인벤토리, 장비, 정비 콘솔 치트
 */

UCLASS()
class WINDLESS_API UInventoryCheats : public UWindlessCheatExtension
{
	GENERATED_BODY()

public:
	/* 아이템 */
	UFUNCTION(Exec)
	void InvAdd(const FString& itemId, int32 count = 1); // 아이템을 획득하는 함수

	UFUNCTION(Exec)
	void InvUse(const FString& itemId); // 소비 아이템을 사용하는 함수

	UFUNCTION(Exec)
	void InvSplit(const FString& itemId, int32 amount); // 아이템을 나누는 함수

	UFUNCTION(Exec)
	void InvDrop(const FString& itemId, int32 quantity); // 아이템을 버리는 함수

	UFUNCTION(Exec)
	void InvMove(int32 uid, int32 x, int32 y, bool bRot); // 인벤토리 내 아이템을 이동/회전시키는 함수

	UFUNCTION(Exec)
	void InvSpawnWorld(const FString& itemId, int32 count = 1); // 플레이어 앞에 필드 아이템을 생성하는 함수


	/* 장비 */
	UFUNCTION(Exec)
	void InvEquip(const FString& itemId, int32 slot = 0); // 장비 슬롯을 지정하여 장비를 장착하는 함수

	UFUNCTION(Exec)
	void InvUnequip(int32 slot); // 장비 슬롯을 해제하는 함수

	UFUNCTION(Exec)
	void InvDur(int32 slot, float amount); // 장비의 내구도를 감소시키는 함수

	UFUNCTION(Exec)
	void InvRepair(int32 slot, float amount); // 장비의 내구도를 수리하는 함수


	/* 출력 */
	UFUNCTION(Exec)
	void InvPrint(); // 인벤토리를 출력하는 함수

	UFUNCTION(Exec)
	void InvLimit(); // 소지 개수 제한 현황을 출력하는 함수

	UFUNCTION(Exec)
	void WLRepairInfo(); // 수리도구와 수리 대상을 출력하는 함수

	UFUNCTION(Exec)
	void WLCraftInfo(); // 제작 레시피 상태를 출력하는 함수
};