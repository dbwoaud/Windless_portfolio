#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemTypes.h"
#include "Characters/BodyPartTypes.h"
#include "Save/WindlessSaveTypes.h"
#include "InventoryComponent.generated.h"

class UAttributeComponent;
class UBodyPartComponent;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDestroyed, FName, itemId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDropped, FItemInstance, droppedInstance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickSlotChanged, EQuickSlot, quickSlot);
DECLARE_DELEGATE_RetVal_OneParam(bool, FCanChangeEquipSlot, EEquipSlot);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WINDLESS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();


	/* 아이템 획득 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Add")
	bool AddItemById(FName itemId, int32 quantity = 1); // 원본 ID로 아이템을 획득하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
	void RefillRecoveryItem(int32 charges); // 회복 슬롯 아이템을 정한 횟수로 채우는 함수 (없으면 새로 줌, D-62)

	UPROPERTY(EditAnywhere, Category = "Inventory|QuickSlot")
	FName recoveryItemId = TEXT("Cons_HPPortion01"); // 회복 슬롯에 주는 아이템 행 (이름과 수치는 [미정])

	TWeakObjectPtr<AActor> shiningStone; // 놓여 있는 빛나는 돌 (한 번에 하나, D-52)

	UFUNCTION(BlueprintCallable, Category = "Inventory|Add")
	bool AddItem(const FItemData& data, int32 quantity = 1); // 원본 데이터로 아이템을 추가하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Add")
	bool AddItemInstance(const FItemInstance& srcInstance, int32& outAddedCount); // 내구도와 수량을 유지하여 아이템을 획득하는 함수

	bool AddStackToInstance(int64 instanceUID, int32 amount); // 특정 인스턴스의 스택을 증가시키는 함수


	/* 아이템 제거/사용/버리기 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Remove")
	bool RemoveInstance(int64 instanceUID, int32 quantity = 1); // 인스턴스 UID로 아이템을 제거하는 함수

	void RemoveInstanceAt(int32 index); // 인덱스로 아이템을 제거하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Remove")
	int32 RemoveItemById(FName itemId, int32 quantity); // 아이템 ID로 지정 수량만큼 제거하고 제거한 수를 반환하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Use")
	bool UseItem(int64 instanceUID); // 소비 아이템을 사용하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Use")
	float GetItemCastTime(int64 instanceUID) const; // 혼돈 배율이 적용된 시전 시간을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Use")
	float GetItemMoveSpeedRate(int64 instanceUID) const; // 시전 중 이동 속도 배율을 반환하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Use")
	bool UseItemOnPart(int64 instanceUID, EBodyPart targetPart); // 특정 부위에 아이템을 사용하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Use")
	bool CanUseItemOnPart(int64 instanceUID, EBodyPart targetPart) const; // 특정 부위에 사용 가능한 아이템인지 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Use")
	void GetUsableBodyParts(int64 instanceUID, TArray<EBodyPart>& outParts) const; // 해당 아이템을 사용할 수 있는 부위 목록을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Use")
	bool RequiresBodyPartSelection(int64 instanceUID) const; // 사용 시 부위 지정이 필요한 아이템인지 확인하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Drop")
	bool DropItem(int64 instanceUID, int32 quantity = -1); // 아이템을 버리는 함수(-1이면 전량)

	UFUNCTION(BlueprintPure, Category = "Inventory|Drop")
	bool CanDropItem(int64 instanceUID) const; // 버리기 가능 여부를 검사하는 함수


	/* 아이템 장착/해제 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Equip")
	bool EquipItem(int64 instanceUID, EEquipSlot targetSlot = EEquipSlot::EES_None); // 아이템 인스턴스를 목표 슬롯에 장착하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Equip")
	bool CanEquipToSlot(FName itemId, EEquipSlot targetSlot) const; // 아이템을 목표 슬롯에 장착 가능한지 여부를 검사하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Equip")
	bool UnequipSlot(EEquipSlot slot); // 특정 슬롯의 장비 해제 후 인벤토리로 되돌리는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Equip")
	int64 GetEquippedInstanceUID(EEquipSlot slot) const; // 특정 슬롯에 장착된 인스턴스 UID를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Equip")
	EBodyPart GetBodyPartForSlot(EEquipSlot slot) const; // 장착 슬롯에 대응하는 신체 부위를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Equip")
	EEquipSlot GetOccupyingSlot(EEquipSlot slot) const; // 해당 슬롯을 실제로 점유한 장착 슬롯을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Equip")
	float GetHandAttackBonus(EEquipSlot handSlot) const; // 손 슬롯 무기의 공격력 보정치를 반환하는 함수


	/* 아이템 소지 개수 제한 */
	UFUNCTION(BlueprintPure, Category = "Inventory|CarryLimit")
	int32 GetCarryLimit(FName itemId) const; // 아이템 종류별 최대 소지 개수를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|CarryLimit")
	int32 GetItemTotalCount(FName itemId) const; // 인벤토리 전체에서 해당 아이템의 총 수량을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Query")
	int32 GetFreeCellCount() const; // 비어 있는 그리드 칸 수를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|CarryLimit")
	int32 GetRemainingCarrySpace(FName itemId) const; // 소지 한도까지 남은 여유 수량을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|CarryLimit")
	bool IsCarryLimited(FName itemId) const; // 소지 개수 제한 대상 아이템인지 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|CarryLimit")
	bool IsCarryLimitReached(FName itemId) const; // 소지 한도에 도달했는지 확인하는 함수


	/* 장비 아이템 내구도 */
	void DamageEquippedDurability(EEquipSlot slot, float amount); // 장착 장비의 내구도를 감소시키는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Durability")
	bool IsInstanceBroken(int64 instanceUID) const; // 장비의 내구도 소진 시 파괴된 상태인지 확인하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	bool RepairItem(int64 instanceUID, float amount = -1.f); // 아이템 내구도를 회복하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Durability")
	bool SetInstanceDurability(int64 instanceUID, float newValue); // 인스턴스의 내구도와 충전량을 직접 설정하는 함수


	/* 그리드 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool ResizeGrid(int32 newWidth, int32 newHeight); // 그리드 크기를 변경하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool MoveItem(int64 instanceUID, int32 newX, int32 newY, bool bRotated); // 아이템을 그리드 좌표로 이동시키는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool SwapItems(int64 uidA, int64 uidB); // 두 아이템의 위치를 맞바꾸는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool MergeStacks(int64 sourceUID, int64 targetUID); // 두 인스턴스의 수량을 합치는 함수

	bool TransferMergeTo(UInventoryComponent* target, int64 sourceUID, int64 targetUID); // 다른 인벤토리의 동일 아이템에 스택을 합치는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool SplitStack(int64 instanceUID, int32 splitAmount, int32 newX, int32 newY, bool bRotated); // 중첩 아이템을 지정 좌표로 분리하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool SplitStackAuto(int64 instanceUID, int32 splitAmount); // 중첩 아이템을 첫 빈 공간으로 분리하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	bool CanSplitStack(int64 instanceUID, int32 splitAmount) const; // 분리 가능 여부를 검사하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	bool CanSplitStackAt(int64 instanceUID, int32 splitAmount, int32 x, int32 y, bool bRotated) const; // 지정 좌표로 분리 가능한지 검사하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	bool CanPlaceNewInstanceAt(FName itemId, int32 x, int32 y, bool bRotated) const; // 신규 인스턴스를 지정 좌표에 놓을 수 있는지 검사하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	bool IsSplittable(int64 instanceUID) const; // 나누기가 가능한 인스턴스인지 확인하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|Grid")
	bool RotateItem(int64 instanceUID); // 아이템을 제자리에서 회전시키는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	bool CanPlaceInstanceAt(int64 instanceUID, int32 x, int32 y, bool bRotated) const; // 자기 자신을 제외하고 배치 가능 여부를 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	void GetInstanceFootprint(int64 instanceUID, bool bRotated, int32& outW, int32& outH) const; // 회전 반영 점유 크기를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	bool IsCellOccupied(int32 x, int32 y) const; // 특정 셀이 점유 중인지 반환하는 함수


	/* 조회 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	FItemInstance FindInstance(int64 instanceUID) const; // UID로 아이템 인스턴스를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int64 FindInstanceUIDAtCell(int32 x, int32 y) const; // 특정 셀의 아이템 UID를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FColor GetGradeColor(EItemGrade grade) const; // 아이템 등급별 색을 반환하는 함수
	
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsItemRegistered(FName itemId) const; // 아이템 원본이 데이터베이스에 등록되어 있는지 확인하는 함수

	bool GetItemData(FName itemId, FItemData& outData) const; // 아이템 ID로 아이템 원본 데이터를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|Query")
	bool IsAmmoItem(FName itemId) const; // 탄약 아이템인지 확인하는 함수


	/* 저장/불러오기 */
	void SaveToRecord(FInventorySaveRecord& outRecord) const; // 현재 인벤토리 상태를 저장 레코드로 변환하는 함수
	void LoadFromRecord(const FInventorySaveRecord& record); // 저장 레코드로 인벤토리 상태를 복원하는 함수


	/* 컨테이너 간 이동 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TransferInstanceTo(UInventoryComponent* target, int64 instanceUID, int32 targetX, int32 targetY, bool bRotated); // 다른 인벤토리로 아이템을 옮기는 함수
	bool CanAcceptInstanceAt(const FItemInstance& inst, int32 x, int32 y, bool bRotated) const; // 해당 인스턴스를 특정 칸에 받을 수 있는지 확인하는 함수
	bool PlaceInstanceAt(const FItemInstance& inst, int32 x, int32 y, bool bRotated); // 인스턴스를 특정 칸에 배치하는 함수
	void GetNewInstanceFootprint(FName itemId, bool bRotated, int32& outW, int32& outH) const; // 아이템 ID 기준 점유 크기를 반환하는 함수


	/* 빠른 보관 시스템 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickTransfer")
	bool QuickTransferTo(UInventoryComponent* target, int64 instanceUID); // 대상 컨테이너의 빈 슬롯으로 아이템을 즉시 이동시키는 함수


	/* 퀵슬롯 시스템 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
	bool EquipToQuickSlot(int64 instanceUID, EQuickSlot quickSlot); // 아이템을 지정 퀵슬롯에 장착하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
	bool QuickEquipToQuickSlot(int64 instanceUID); // 빈 슬롯을 우선 탐색해 자동 장착/해제하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
	bool UnequipQuickSlot(EQuickSlot quickSlot); // 퀵슬롯 장착을 해제하고 인벤토리로 되돌리는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
	bool UseQuickSlot(EQuickSlot quickSlot, EBodyPart targetPart); // 퀵슬롯에 등록된 아이템을 사용하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|QuickSlot")
	int64 GetQuickSlotInstanceUID(EQuickSlot quickSlot) const; // 퀵슬롯에 등록된 인스턴스 UID를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|QuickSlot")
	bool CanAssignToQuickSlot(FName itemId, EQuickSlot quickSlot) const; // 해당 슬롯에 장착 가능한 아이템인지 검사하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|QuickSlot")
	int32 GetMaxEquipCount(FName itemId) const; // 소모품 슬롯 최대 장착 수량을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|QuickSlot")
	EBodyPart GetAutoTargetPart(int64 instanceUID) const; // 즉시 사용 시 자동으로 선택할 부위를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|QuickSlot")
	float GetItemCooldown(int64 instanceUID) const; // 아이템의 재사용 대기시간을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|QuickSlot")
	int32 GetItemConditionCode(int64 instanceUID) const; // 아이템의 사용 조건 검증값을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory|QuickSlot")
	bool IsRecoveryInstance(int64 instanceUID) const; // 체력 회복 아이템 인스턴스인지 확인하는 함수

	UFUNCTION(BlueprintCallable, Category = "Inventory|QuickSlot")
	void BroadcastAllQuickSlots(); // 모든 퀵슬롯의 변경을 브로드캐스트하는 함수


	/* 디버그 출력용 */
	int64 FindFirstUnequippedUID(FName itemId) const; // 장착되지 않는 첫 장비 아이템의 UID를 찾는 함수
	void PrintInventoryDebug() const; // 인벤토리 전체를 화면에 출력하는 함수
	void PrintEquippedDebug() const;  // 장착 현황을 화면에 출력하는 함수
	void PrintGridDebug() const; // 그리드 점유 맵을 출력하는 함수
	void PrintCarryLimitDebug() const; // 아이템 소지 개수 제한 현황을 출력하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Event")
	FOnInventoryChanged OnInventoryChanged; // 인벤토리 상태 변경 시 호출되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Event")
	FOnEquipmentChanged OnEquipmentChanged; // 장착 현황 변경 시 호출되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Event")
	FOnItemDestroyed OnItemDestroyed; // 장비 내구도 소진으로 파괴될 때 호출되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Event")
	FOnItemDropped OnItemDropped; // 아이템을 버릴 때 호출되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Event")
	FOnQuickSlotChanged OnQuickSlotChanged; // 퀵슬롯 등록 내용 변경 시 호출되는 델리게이트

	FCanChangeEquipSlot canChangeEquipSlot; // 장착 슬롯 변경을 확인하는 델리게이트

protected:
	virtual void BeginPlay() override;

private:
	/* 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UAttributeComponent> ownerAttributes = nullptr; // 같은 액터의 속성 컴포넌트
	UPROPERTY()
	TObjectPtr<UBodyPartComponent> ownerBodyParts = nullptr; // 같은 액터의 부위 컴포넌트


	/* 인벤토리 스테이터스 */
	UPROPERTY(EditAnywhere, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 gridWidth = 6; // 인벤토리 가로 크기

	UPROPERTY(EditAnywhere, Category = "Inventory|Grid", meta = (ClampMin = "1"))
	int32 gridHeight = 6; // 인벤토리 세로 크기

	UPROPERTY(EditAnywhere, Category = "Inventory|Grid")
	bool bAllowAutoRotate = true; // 자동 배치 시 회전 배치 시도 여부

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TArray<FItemInstance> items; // 보유 아이템 인스턴스 목록

	UPROPERTY(VisibleAnywhere, Transient, Category = "Inventory")
	TArray<int32> gridMap; // 점유 상태 테이블(0=빈칸,1=점유)


	/* 인스턴스 UID 발급 */
	int64 nextInstanceUID = 1; // 다음 발급할 인스턴스 UID
	int64 GenerateInstanceUID(); // 새 인스턴스 UID를 발급하는 함수
	

	/* 아이템 원본 */
	UPROPERTY(Transient)
	TObjectPtr<class UItemDatabaseSubsystem> itemDatabase = nullptr; // 아이템 원본 서브시스템 캐시


	/* 초기화 */
	void ResetGridMap(); // 그리드 점유 맵을 초기화하는 함수


	/* 그리드 연산 */
	int32 CellIndex(int32 x, int32 y) const { return y * gridWidth + x; } // 2차원 배열 인덱스를 1차원 배열 인덱스로 반환하는 함수
	void GetFootprint(const FItemData& data, bool bRotated, int32& outW, int32& outH) const; // 회전을 반영한 아이템의 점유 크기를 반환하는 함수
	bool CanPlaceAt(int32 startX, int32 startY, int32 w, int32 h) const; // 해당 영역에 배치 가능한지 확인하는 함수
	void SetCells(int32 startX, int32 startY, int32 w, int32 h, int32 value); // 영역 셀 값을 설정하는 함수
	void OccupyForInstance(const FItemInstance& inst); // 인스턴스가 차지하는 셀을 점유 처리하는 함수
	void FreeForInstance(const FItemInstance& inst); // 인스턴스가 차지하던 셀을 빈칸 처리하는 함수
	bool TryAutoPlace(FItemInstance& inst); // 빈 공간을 탐색해 인스턴스를 배치하는 함수


	/* 인스턴스 탐색 */
	int32 FindInstanceIndex(int64 instanceUID) const; // UID로 items 배열 인덱스를 찾는 함수
	int32 FindEquippedIndex(EEquipSlot slot) const; // 특정 슬롯에 장착된 인스턴스 인덱스를 찾는 함수
	int32 FindStackableIndex(FName itemId, int32 maxStack) const; // 중첩 여유가 있는 동일 아이템 인덱스를 찾는 함수
	int32 FindInstanceAtCell(int32 x, int32 y) const; // 특정 셀을 점유한 인스턴스 인덱스를 찾는 함수


	/* 슬롯 규칙 */
	static EEquipSlot GetSlotGroup(EEquipSlot slot); // 좌우 짝 그룹의 대표값을 반환하는 함수
	static EEquipSlot GetPairedSlot(EEquipSlot slot); // 좌우 짝 슬롯을 반환하는 함수
	static EEquipSlot GetLeftSlotOfPair(EEquipSlot slot); // 좌우 쌍의 왼쪽 슬롯을 반환하는 함수
	EEquipSlot ResolveAutoEquipSlot(EEquipSlot desiredSlot) const; // 자동 장착 시 비어 있는 짝 슬롯을 우선 선택하는 함수
	bool IsEquipSlotChangeAllowed(EEquipSlot slot) const; // 캐릭터의 장비 슬롯 변경을 확인하는 함수
	bool ReleaseEquippedToGrid(EEquipSlot slot); // 장착 아이템을 그리드로 되돌리는 함수
	EEquipSlot FindTwoHandedConflictSlot(EEquipSlot desiredSlot, bool bTwoHanded) const; // 양손무기 장착 시 반대 손 슬롯을 반환하는 함수
	static bool IsHandSlot(EEquipSlot slot); // 손 슬롯인지 확인하는 함수
	static bool IsTwoHandedWeapon(const FItemData& data); // 양손무기 아이템인지 확인하는 함수



	/* 효과/스탯 */
	bool ApplyConsumableEffect(const FItemData& data, EBodyPart targetPart = EBodyPart::EBP_None, int64 instanceUID = 0); // 소비 아이템 효과를 적용하는 함수
	void RecalculateEquipmentStats(); // 장착 장비의 보정치 합계를 재계산하는 함수

	float totalAttackBonus = 0.f;   // 장비 공격력 보정 합계
	float totalLimbHPBonus = 0.f;   // 장비 부위 생명력 보정 합계

	
	/* 부위 */
	static EBodyPart EquipSlotToBodyPart(EEquipSlot slot); // 슬롯과 신체 부위를 연결하는 함수


	/* 소지 개수 제한 */ 
	static bool IsInjuryCureData(const FItemData& data); // 부상 치유 아이템인지 판별하는 함수
	int32 GetCarryLimitFromData(const FItemData& data) const; // 원본 데이터로 소지 한도를 계산하는 함수
	void NotifyCarryLimitBlocked(const FItemData& data, int32 addedCount) const; // 소지 한도 초과 안내를 출력하는 함수

	UPROPERTY(EditAnywhere, Category = "Inventory|CarryLimit", meta = (ClampMin = "0"))
	int32 defaultInjuryCureCarryLimit = 0; // 부상 치유 아이템 기본 소지 한도 (0 = 제한 없음, D-63)


	/* 퀵슬롯 시스템 */
	int32 FindQuickSlotIndex(EQuickSlot quickSlot) const; // 특정 퀵슬롯에 등록된 인스턴스 인덱스를 찾는 함수
	static bool IsRecoveryData(const FItemData& data); // 체력 회복 아이템인지 판별하는 함수
	bool ReturnInstanceToGrid(FItemInstance inst); // 인스턴스를 인벤토리 그리드로 되돌리는 함수
	bool ReturnInstanceToGridAt(FItemInstance inst, int32 x, int32 y, bool bRotated); // 지정 좌표 우선으로 되돌리는 함수
	void TryAutoEquipRecoveryItem(FName itemId); // 회복 아이템을 회복 슬롯에 자동 등록하는 함수

public:
	/* 변수 Getter 함수 */
	FORCEINLINE const TArray<FItemInstance>& GetItems() const { return items; }
	FORCEINLINE float GetTotalAttackBonus() const { return totalAttackBonus; }
	FORCEINLINE float GetTotalLimbHPBonus() const { return totalLimbHPBonus; }
	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	FORCEINLINE int32 GetGridWidth() const { return gridWidth; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Grid")
	FORCEINLINE int32 GetGridHeight() const { return gridHeight; }
};
