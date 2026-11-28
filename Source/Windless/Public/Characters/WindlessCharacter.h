#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "Characters/BodyPartTypes.h"
#include "Components/MountComponent.h"
#include "Components/CombatComponent.h"
#include "InputActionValue.h"
#include "Items/ItemTypes.h"
#include "Interfaces/PickupInterface.h"
#include "Items/Storage/SharedStorageSubsystem.h"
#include "WindlessCharacter.generated.h"

class UWindlessInputConfig;
class UPlayerInputModeComponent;
class UCameraComponent;
class USpringArmComponent;
class UGroomComponent;
class AItems;
class ASouls;
class ATreasure;
class UAnimMontage;
class UWindlessOverlay;
class UTargetingComponent;
class UInventoryComponent;
class UCharacterDebugComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPlayerInteractionComponent;
class UDialogueComponent;
class UItemDropComponent;
class UInventoryWidget;
class UQuickSlotComponent;
class UMaintenanceComponent;
class URefineComponent;
class URepairComponent;
class UCraftingComponent;
class UFastTravelComponent;
class UWeaponLoadoutComponent;
class UBowComponent;
class UPlayerStaggerComponent;
class UPlayerAISignal;
class UPlayerProgressionComponent;
class UPlayerSaveComponent;
class UCameraShakeBase;
class USoundBase;
enum class EChaosState : uint8;
enum class EChaosZone : uint8;
enum class EQuickSlot : uint8;
struct FStaggerStatus;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeathSignature);

UCLASS(Blueprintable)
class WINDLESS_API AWindlessCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	AWindlessCharacter();
	virtual void Tick(float DeltaTime) override;
	
	/* 입력 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override; // 향상된 입력 기능을 사용하기 위한 함수
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SetDialogueInputMode(bool bEnable); // 대화 입력 모드 컨텍스트를 활성/비활성화하는 함수
	void SetInventoryInputMode(bool bEnable); // 인벤토리 입력 컨텍스트를 활성/비활성화하는 함수


	/* 상태 확인 */
	bool IsOccupied(); // 캐릭터가 특정 상태인지 확인하는 함수
	bool CanStartMovementAction(EMovementAction action, bool bIgnoreReleasableState = false); // 이동 액션을 시작할 수 있는 상태인지 확인하는 함수
	bool IsInMovement() const; // 점프/낙하/앉기/구르기 중인지 확인하는 함수
	bool IsCrouchRequested() const; // 앉기 상태이거나 앉기 요청이 접수된 상태인지 확인하는 함수
	bool HasStandingHeadroom() const; // 일어설 수 있는 공간이 있는지 확인하는 함수
	bool IsBowEquipped() const; // 활을 들고 있는지 확인하는 함수


	/* 이동 시스템 */
	virtual void Jump() override; // 플레이어 캐릭터 점프 시 호출되는 함수
	virtual void Landed(const FHitResult& Hit) override; // 착지 시 실행되는 함수


	/* 전투 시스템(공격) */
	virtual float GetAttackPowerWith(const AWeapon* weapon) const override; // 공격하는 손 무기의 보정치를 더한 공격력을 반환하는 함수
	virtual void OnDealtWeaponHit() override; // 근접 공격에 적 히트 시 실행되는 함수
	virtual void OnDealtRangedHit() override; // 원거리 공격에 적 히트 시 실행되는 함수
	bool CanAttack() override; // 공격 가능한 상태인지 확인하는 함수
	virtual void HandleDamage(float damageAmount) override; // 공격 대미지를 처리하는 함수(부위별 피격 시스템으로 미사용)
	void BeginAttack(); // 공격을 시작하는 함수
	bool PerformAttack(AWeapon* weapon, EAttackType attackType, UAnimMontage* montage); // 공격을 수행하는 함수
	bool JumpToAttackSection(UAnimMontage* montage, FName sectionName); // 공격 애니메이션의 섹션을 이동하는 함수
	void EnterGuardState(); // 가드 상태에 진입하는 함수
	void ExitGuardState(); // 가드 상태에 탈출하는 함수
	void MarkCombatEngaged(); // 전투 상태 타이머를 갱신하는 함수

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInCombatState() const; // 현재 전투 상태인지 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetTimeSinceLastDamage() const; // 마지막 전투 이후 경과 시간을 반환하는 함수


	/* 전투 시스템(콤보) */
	bool ContinueAttackCombo(AWeapon* weapon, EAttackType attackType, UAnimMontage* montage, FName sectionName); // 콤보를 실행하는 함수
	void FaceAttackDirection(); // 공격 시작 시 시야 방향으로 회전하는 함수


	/* 전투 시스템(피격) */
	// 무기 피격 시 실행되는 함수
	virtual bool ReceiveWeaponHit(const FCombatHitInfo& hitInfo) override;
	virtual void GetHit_Implementation(const FVector& impactPoint, AActor* hitter) override; // HitInterface를 구현하는 함수
	// 플레이어 캐릭터 피격 시 호출되는 함수
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override; 


	/* 전투 시스템(사망) */
	virtual bool IsAlive() override; // 부위 기반으로 사망 판정을 확인하는 함수
	virtual void DieByFall() override; // 낙사 영역에서 즉사하는 함수

	UFUNCTION(BlueprintCallable, Category = "Death")
	void Revive(); // 부활 시 캐릭터 설정을 되돌리는 함수
	
	UFUNCTION(BlueprintCallable, Category = "Death")
	void ForceDeath(); // 플레이어 즉사용 함수


	/* 전투 시스템(무기) */
	virtual void SetOverlappingItem(AItems* item) override; // PickupInterface를 구현하기 위한 함수
	void SetHandWeapon(EEquipSlot slot, AWeapon* weapon); // 손 무기를 교체하는 함수


	/* 전투 시스템(활) */
	void EnterAimState(); // 활 조준 상태로 진입하는 함수
	void ExitAimState(); // 활 조준 상태를 해제하는 함수
	void BeginBowShotRecovery(); // 발사 후딜 상태로 전환하는 함수


	/* 전투 시스템(락온) */
	float GetLockOnRangeMultiplier() const; // 락온 거리 배율을 반환하는 함수
	bool IsStrafing() const; // 락온 중 대상을 바라보며 이동하는지 확인하는 함수
	UTargetingComponent* GetTargeting() const; // 락온 컴포넌트를 반환하는 함수


	/* 캐릭터 속성(스태미나) */
	float GetCurrentStamina() const; // 현재 스태미나를 반환하는 함수
	void ConsumeStamina(float cost); // 스태미나를 소모하는 함수
	bool HasStamina(float cost) const; // 스태미나가 특정 코스트 이상인지 확인하는 함수
	bool HasEnoughAttackStamina(const AWeapon* weapon, EAttackType attackType) const; // 강공격 스태미나가 충분한지 확인하는 함수


	/* 캐릭터 속성(기타) */
	virtual void AddGold(ATreasure* treasure) override; // PickupInterface를 구현하기 위한 함수
	virtual void AddSouls(ASoul* soul) override; // PickupInterface를 구현하기 위한 함수


	/* 혼돈 시스템 */
	UFUNCTION(BlueprintCallable, Category = "Chaos")
	void EnterLightZone(); // 빛 구역 진입 시 실행되는 함수
	UFUNCTION(BlueprintCallable, Category = "Chaos")
	void ExitLightZone(); // 빛 구역 이탈 시 실행되는 함수
	void AddChaosZone(const class AChaosZoneVolume* zone); // 빛/어둠 볼륨 진입 시 실행되는 함수
	void RemoveChaosZone(const class AChaosZoneVolume* zone); // 빛/어둠 볼륨 이탈 시 실행되는 함수
	EChaosZone GetChaosZone(float& outLimit) const; // 지금 서 있는 곳(빛/그늘/어둠)과 그 하한 또는 상한을 반환하는 함수
	bool IsInLightZone() const; // 빛 구역 안인지 확인하는 함수


	/* 부위 시스템 */
	bool CheckActionDelay() const; // 액션 딜레이 상태인지 확인하는 함수
	void ApplyActionDelay(); // 다음 액션 시작 딜레이를 적용하는 함수


	/* 인벤토리 시스템 */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UInventoryComponent* GetInventory() const { return inventory; } // 인벤토리 컴포넌트를 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsInventoryOpen() const; // 인벤토리 UI가 열려 있는지 확인하는 함수
	void CloseInventoryFromUI(); // 가방 위젯이 Tab을 받았을 때 가방을 닫는 함수


	/* 아이템 시스템 */
	UFUNCTION(BlueprintCallable, Category = "Item")
	bool BeginItemCast(int64 instanceUID, EBodyPart targetPart, float castTime); // 아이템 시전을 시작하는 함수

	UFUNCTION(BlueprintCallable, Category = "Item")
	void CancelItemCast(bool bConsumeItem); // 시전을 취소하는 함수

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsCastingItem() const { return actionState == EActionState::EAS_UsingItem; } // 현재 아이템 시전 중인지 확인하는 함수

	
	/* 대화 시스템 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UDialogueComponent* GetDialogueComponent() const { return dialogueComponent; } // 대화 컴포넌트를 반환하는 함수


	/* 창고 시스템 */
	void OpenStorage(class AStorageContainer* container); // 창고와 함께 인벤토리를 여는 함수


	/* 퀵슬롯 시스템 */
	UFUNCTION(BlueprintPure, Category = "QuickSlot")
	bool IsQuickSlotActive() const { return bQuickSlotActive; } // 퀵슬롯 조작 중인지 확인하는 함수

	void SetQuickSlotActive(bool bActive) { bQuickSlotActive = bActive; } // 퀵슬롯 조작 상태를 설정하는 함수


	/* 강한 경직 */
	virtual FStaggerStatus GetStaggerStatus() const override; // 경직 상태 정보를 반환하는 함수
	FStaggerStatus PlayerStagger() const; // 플레이어 경직 상태를 반환하는 함수
	float PlayerToughness() const; // 플레이어 강인도를 반환하는 함수
	bool ForceStrongStagger(const FVector& fromLocation); // 강한 경직을 강제하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Death|Event")
	FOnPlayerDeathSignature onPlayerDeath; // 플레이어 사망 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;

	/* 입력 설정 */
	UPROPERTY(EditDefaultsOnly, Category = Input)
	TObjectPtr<UWindlessInputConfig> inputConfig; // 입력 액션 설정 에셋


	/* 이동 시스템 */
	void Move(const FInputActionValue& value); // 플레이어 캐릭터 이동 입력 시 실행되는 함수
	void StopMoving(); // 플레이어 캐릭터 이동 정지 시 실행되는 함수
	void Look(const FInputActionValue& value); // 플레이어 캐릭터 회전 입력 시 실행되는 함수
	void Sprint(); // 플레이어 캐릭터 달리기 입력 시 실행되는 함수
	void HandleSprintStamina(float DeltaTime); // 플레이어 캐릭터 달리기 상태 일 때, 스태미나를 처리하는 함수 
	void StopSprinting(); // 플레이어 캐릭터 달리기 정지 시 실행되는 함수
	void ToggleCrouch(); // 플레이어 캐릭터 숙이기 입력 시 실행되는 함수
	void LockAirRotation(); // 공중 회전을 잠그는 함수
	void UnlockAirRotation(); // 공중 회전 잠금을 해제하는 함수
	void HandleAirRotationLock(); // 공중 상태 변화에 따라 회전 잠금을 갱신하는 함수
	void RefreshRotationMode(); // 상태에 맞게 이동 회전 방식을 갱신하는 함수


	/* 전투 시스템(공격) */
	virtual void AttackEnd() override; // 공격 완료 시 실행되는 함수
	void RightAttack(); // 플레이어 캐릭터 우 공격 입력 시 실행되는 함수
	void PlayAttackMontageSingle(UAnimMontage* montage, bool bStrong); // 강공격 여부에 따른 애니메이션을 재생하는 함수
	void StopGuard(); // 방어를 해제하는 함수

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0.0"))
	float combatStateDuration = 8.f; // 피격 후 전투 상태 시간
	

	/* 전투 시스템(콤보) */
	void UpdateAttackFacing(float DeltaTime); // 공격 방향으로 회전을 갱신하는 함수

	UPROPERTY(EditAnywhere, Category = "Combat|Attack", meta = (ClampMin = "0.0"))
	float attackTurnSpeed = 20.f; // 공격 시야 회전 속도

	UPROPERTY(EditAnywhere, Category = "Combat|Attack", meta = (ClampMin = "0.0"))
	float attackTurnDuration = 0.15f; // 회전 시간

	float attackFacingYaw = 0.f; // 목표 공격 방향
	float attackTurnRemaining = 0.f; // 남은 회전 시간


	/* 전투 시스템(피격) */
	UFUNCTION(BlueprintCallable)
	void HitReactEnd(); // 피격 완료 시 실행되는 함수 
	virtual void OnStaggered(const FVector& impactPoint, AActor* hitter) override; 
	virtual void OnEndured() override;
	virtual void CancelCurrentAttack() override;
	void ApplyGuardBreakStagger(); // 가드 브레이크 경직을 적용하는 함수
	void BeginStrongStagger(const FVector& fromLocation); // 강한 경직을 시작하는 함수
	void HandleStrongStaggerEnded(); // 강한 경직이 끝났을 때 실행되는 함수


	/* 전투 시스템(사망) */
	virtual void Die_Implementation() override; // 캐릭터 사망 시 호출되는 함수


	/* 전투 시스템(구르기) */
	virtual void DodgeEnd() override; // 구르기 완료 시 실행되는 함수
	void OnSpacePressed(); // 스페이스 바 입력 시 실행되는 함수
	void Dodge(); // 플레이어 캐릭터 구르기 시 실행되는 함수
	void RotateToMovementInputDirection(); // 이동 입력 방향으로 회전하는 함수
	bool HasEnoughDodgeStamina(); // 구르기 할 충분한 스태미나가 있는지 확인하는 함수
	bool CanUseLockOnStep() const; // 락온 스텝 회피를 사용할 수 있는지 확인하는 함수
	void FaceLockOnTarget(); // 락온 대상을 향해 즉시 회전하는 함수
	FName GetStepDodgeSection() const; // 이동 입력에 맞는 스텝 방향 섹션을 반환하는 함수
	void PlayStepDodgeMontage(FName sectionName); // 락온 스텝 애니메이션을 재생하는 함수
	void ClearDodgeState(); // 구르기 상태를 초기화하는 함수

	UFUNCTION(BlueprintCallable)
	void GetUpStart(); // 구르기 완료 시 실행되는 함수

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	UAnimMontage* stepDodgeMontage = nullptr; // 락온 스텝 회피 몽타주

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	FName stepForwardSection = FName("Forward"); // 전방 스텝 섹션 이름

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	FName stepBackwardSection = FName("Backward"); // 후방 스텝 섹션 이름

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	FName stepLeftSection = FName("Left"); // 좌측 스텝 섹션 이름

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge")
	FName stepRightSection = FName("Right"); // 우측 스텝 섹션 이름

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Dodge", meta = (ClampMin = "0.1"))
	float stepDodgeDistanceScale = 1.3f; // 스텝 구르기 거리 배율

	bool bPerfectDodgeConsumed = false; // 퍼펙트 회피 여부


	/* 전투 시스템(무기) */
	virtual void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled) override; // 현재 공격 무기의 콜리전을 활성화하는 함수
	void EKeyPressed(); // 플레이어 캐릭터 무기 장착 입력 시 실행되는 함수
	void PlayEquipMontage(const FName& sectionName); // 무기 장착 애니메이션 몽타주 섹션 재생 함수
	bool CanDisarm(); // 무장 해제할 수 있는 상태인지 확인하는 함수
	bool CanArm(); // 무장할 수 있는 상태인지 확인하는 함수
	void Disarm(); // 무장 해제하는 함수
	void Arm(); // 무장하는 함수

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack(); // 무기를 등에 장착하는 함수

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand(); // 무기를 손에 장착하는 함수

	UFUNCTION(BlueprintCallable)
	void FinishEquipping(); // 무기 장착 완료 시 실행되는 함수


	/* 전투 시스템(락온) */
	void OnLockOnPressed(); // 플레이어 캐릭터 락온 입력 시 실행되는 함수
	void OnSwitchTargetTriggered(const FInputActionValue& Value); // 플레이어 캐릭터 락온 타겟 변경 입력 시 실행되는 함수


	/* 탈 것 시스템 */
	void ToggleMount(); // 탈 것 토글키 입력 시 실행되는 함수
	UFUNCTION()
	void HandleMountPhaseChanged(EMountPhase phase); // 탈 것 단계 변화 시 실행되는 함수


	/* 혼돈 시스템 */
	void HandleChaos(float DeltaTime); // 정신력을 처리하는 함수
	UFUNCTION(BlueprintCallable, Category = "Chaos")
	void ResetChaosGauge();   // 혼돈을 0으로 초기화하는 함수
	UFUNCTION(BlueprintCallable, Category = "Chaos")
	void NotifyPerfectDodge(); // 완벽한 구르기 회피 성공 시 호출되는 함수
	UFUNCTION()
	void HandleChaosStateChanged(EChaosState newState); // 혼돈 상태 변경 시 실행되는 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Chaos")
	void OnChaosStateChangedBP(EChaosState newState); // 혼돈 상태 변경 시 블루프린트에서 실행되는 함수


	/* 부위 시스템 */
	UFUNCTION(BlueprintImplementableEvent, Category = "BodyPart")
	void OnEyeDamageChanged(bool bActive); // 안구 손상 부상 발생 시 호출되는 함수


	/* 인벤토리 시스템 */
	void ToggleInventory(); // 인벤토리 창을 토글하는 함수

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInventoryWidget> inventoryWidgetClass; // 인벤토리 위젯 클래스

	UPROPERTY()
	TObjectPtr<UInventoryWidget> inventoryWidget = nullptr; // 현재 인벤토리 위젯 인스턴스

	
	/* 아이템 시스템 */
	void FinishItemCast(); // 아이템 시전 완료 시 효과를 적용하는 함수
	void ApplyCastMovementRestriction(float speedRate); // 시전 중 이동 제약을 적용하는 함수
	void EndItemCastRecovery(); // 시전 후딜을 끝내고 행동 가능 상태로 되돌리는 함수
	void ClearCastMovementRestriction(); // 시전 종료 시 이동 제약을 해제하는 함수

	UPROPERTY(EditDefaultsOnly, Category = "Item|Cast", meta = (ClampMin = "0.0"))
	float itemCastRecoveryTime = 0.4f; // // 시전 완료 후 후딜 시간

	FTimerHandle itemCastTimer; // 시전 타이머
	FTimerHandle itemCastRecoveryTimer; // 시전 후딜 타이머
	int64 castingItemUID = 0; // 시전 중인 아이템 UID
	EBodyPart castingTargetPart = EBodyPart::EBP_None; // 시전 대상 부위
	float castCachedWalkSpeed = 0.f;

	/* 상호작용 시스템 */
	void Interact(); // 현재 상호작용 가능한 대상과 상호작용하는 함수


	/* 퀵슬롯 시스템 */
	UFUNCTION()
	void HandleQuickSlotHighlightChanged(EQuickSlot quickSlot); // 하이라이트 대상 변경 시 오버레이를 갱신하는 함수

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UQuickSlotComponent> quickSlots = nullptr; // 퀵슬롯 컴포넌트


	/* 정비 모드 시스템 */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UMaintenanceComponent> maintenance = nullptr; // 정비 모드 컴포넌트

	UFUNCTION()
	void HandleMaintenanceStateChanged(bool bInMaintenance); // 정비 모드 진입/종료 시 상태를 갱신하는 함수

	UFUNCTION()
	void HandleMoltingChanged(int32 newLevel, float expPercent); // 탈피석 변경 시 HUD를 갱신하는 함수

private:
	/* 캐릭터 상태 */
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ECharacterState characterState = ECharacterState::ECS_Unequipped; // 캐릭터의 무기 장착 상태 

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState actionState = EActionState::EAS_Unoccupied; // 캐릭터의 액션 상태


	/* 캐릭터 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* springArm; // 스프링 암 컴포넌트

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* viewCamera; // 카메라 컴포넌트 

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* hair; // 그룸 컴포넌트(머리카락)

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* eyebrows; // 그룸 컴포넌트(눈썹)

	UPROPERTY(VisibleAnywhere, Category = Combat)
	UCombatComponent* combat; // 전투 시스템 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = LockOn)
	TObjectPtr<UTargetingComponent> targetingComponent; // 락온 시스템 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Mount)
	UMountComponent* mountComponent; // 탈 것 시스템 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Inventory)
	UInventoryComponent* inventory; // 인벤토리 시스템 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Debug)
	UCharacterDebugComponent* debugComponent; // 디버그 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Refine)
	URefineComponent* refine; // 정제 컴포넌트
	
	UPROPERTY(VisibleAnywhere, Category = Repair)
	URepairComponent* repair; // 수리 컴포넌트
	
	UPROPERTY(VisibleAnywhere, Category = Craft)
	UCraftingComponent* crafting; // 제작 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "Fast Travel")
	UFastTravelComponent* fastTravel; // 빠른 이동 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	UWeaponLoadoutComponent* weaponLoadout; // 손 무기 액터 동기화 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Combat)
	UBowComponent* bow; // 활 시스템 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Combat)
	UPlayerStaggerComponent* staggerComponent; // 강한 경직 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Combat)
	UPlayerAISignal* aiSignals; // AI 신호 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Input)
	TObjectPtr<UPlayerInputModeComponent> inputMode; // 입력 모드 관리 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TObjectPtr<UPlayerInteractionComponent> interaction; // 상호작용 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "Item|Drop")
	TObjectPtr<UItemDropComponent> itemDrop; // 아이템 버리기 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "Molting")
	TObjectPtr<UPlayerProgressionComponent> progression; // 성장 및 휴식 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "Save")
	TObjectPtr<UPlayerSaveComponent> saveComponent; // 세이브 컴포넌트


	/* 이동 시스템 */
	UPROPERTY()
	FVector2D currentMovementInput = FVector2D::ZeroVector; // 현재 이동 입력 방향

	UPROPERTY(EditAnywhere, Category = Movement)
	float crawlSpeed = 100.f; // 양다리 부상으로 인한 기어 다니기 속도

	UPROPERTY(EditAnywhere, Category = "Combat|Bow", meta = (ClampMin = "0.0"))
	float aimWalkSpeed = 150.f; // 활 조준 이동 속도

	UPROPERTY(EditAnywhere, Category = Movement)
	float guardWalkSpeed = 200.f; // 가드 중 걷기 속도

	UPROPERTY(EditAnywhere, Category = Movement)
	float walkSpeed = 240.f; // 걷기 속도 (기준치수표 7.1 D2)

	UPROPERTY(EditAnywhere, Category = Movement)
	float sprintSpeed = 500.f; // 달리기 속도 (기준치수표 7.1 D1)

	bool bIsSprinting = false; // 달리기 중인지 확인하는 변수

	UPROPERTY(EditAnywhere, Category = "Movement|Air")
	bool bLockRotationInAir = true; // 공중에서 캐릭터 회전을 잠글지 여부

	UPROPERTY(EditAnywhere, Category = "Movement|Air")
	bool bLockRotationOnFall = true; // 점프가 아닌 낙하 시에도 회전을 잠글지 여부

	UPROPERTY(EditAnywhere, Category = "Movement|Air", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float airControlWhileLocked = 0.15f; // 회전 잠금 중 공중 이동 제어량

	bool bAirRotationLocked = false; // 현재 공중 회전이 잠겨 있는지 여부
	float cachedAirControl = 0.05f;  // 잠금 이전 공중 제어량 복원값


	/* 전투 시스템(공격) */
	float lastDamagedTime = -1000.f; // 마지막으로 피해를 입은 시각


	/* 전투 시스템(구르기) */
	UPROPERTY(EditAnywhere, Category = "Movement|Dodge")
	float dodgePlayRate = 1.0f; // 구르기 애니메이션 재생 속도

	UPROPERTY(EditAnywhere, Category = "Movement|Dodge")
	float getUpPlayRate = 1.5f; // 구르기 후 일어나기 애니메이션 재생 속도


	/* 전투 시스템(사망) */
	bool bIsDead = false; // 사망 상태 여부


	/* 전투 시스템(무기) */
	bool CanChangeEquipSlot(EEquipSlot slot) const; // 인벤토리 장착 슬롯 변경 가능 여부를 확인하는 함수
	void RefreshCharacterState(); // 무기에 맞춰 장착 상태를 변경하는 함수

	UPROPERTY(VisibleInstanceOnly)
	AItems* overlappingItem; // 현재 overlapping 된 아이템 

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* equipMontage; // 무기 장착 애니메이션 몽타주

	UPROPERTY(VisibleInstanceOnly, Category = Weapon)
	AWeapon* leftHandWeapon; // 캐릭터의 왼손 무기

	UPROPERTY()
	AWeapon* currentAttackWeapon; // 현재 공격 무기


	/* 캐릭터 속성(스태미나) */
	void HandleStamina(float DeltaTime); // 스태미나를 처리하는 함수


	/* 탈 것 시스템 */
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bIsMounted = false; // 캐릭터가 탈 것 탑승 중인지 확인하는 변수


	/* 혼돈 시스템 */
	void SetPostProcessingEffect(EChaosState newState); // 포스트 프로세싱 효과로 붕괴 단계별 시야 연출을 설정하는 함수 
	void ForceUnlockOnCollapseState(float DeltaTime); // 붕괴 2단계 혼돈 상태에서 일정확률로 락온 기능을 해제하는 함수
	void DisableLockOn(EChaosState newState); // 붕괴 3단계 혼돈 상태에서 락온 기능을 비활성화하는 함수
	void HandleLockOnCameraNoise(float DeltaTime); // 붕괴 단계에서 락온 중 주기적으로 카메라를 흔드는 함수
	void UpdateChaosWobbleShake(EChaosState newState); // 붕괴 3단계 진입/이탈 시 화면 일렁임을 켜고 끄는 함수


	UPROPERTY(EditAnywhere, Category = "Chaos")
	UMaterialInterface* vignetteMaterial = nullptr; // 혼돈 시야 머터리얼 인스턴스

	UPROPERTY()
	UMaterialInstanceDynamic* vignetteMID = nullptr; // 게임 시작 시 할당되는 혼돈 시야 머터리얼 인스턴스

	int32 lightZoneCount = 0; // 현재 겹친 빛 볼륨 수 (EnterLightZone으로 들어온 것)

	TArray<TWeakObjectPtr<const class AChaosZoneVolume>> chaosZones; // 현재 겹친 빛/어둠 볼륨

	float collapse2UnlockTimer = 0.f; // 붕괴2 강제 락온 해제 누적 타이머

	UPROPERTY(EditAnywhere, Category = "Chaos|LockOn")
	TSubclassOf<UCameraShakeBase> lockOnNoiseShakeClass; // 락온 카메라 노이즈 셰이크

	UPROPERTY(EditAnywhere, Category = "Chaos|LockOn")
	TObjectPtr<USoundBase> lockOnForcedBreakSound = nullptr; // 붕괴2 강제 해제 시 이명 사운드

	float lockOnShakeTimer = 0.f; // 락온 카메라 노이즈 누적 타이머

	UPROPERTY(EditAnywhere, Category = "Chaos|Shake")
	TSubclassOf<UCameraShakeBase> chaosWobbleShakeClass; // 붕괴3 화면 일렁임 셰이크

	UPROPERTY(EditAnywhere, Category = "Chaos|Shake", meta = (ClampMin = "0.0"))
	float chaosWobbleShakeScale = 1.f; // 붕괴3 화면 일렁임 강도

	bool bChaosWobbleActive = false; // 붕괴3 화면 일렁임 재생 여부


	/* 부위별 대미지 시스템 */
	void HandleBodyRegen(float DeltaTime); // 부위 자연 회복을 처리하는 함수
	void HandleLegFractureInjury(float DeltaTime); // 다리 골절로 인한 2차 효과를 처리하는 함수
	void ToggleBodyStatus(); // 부위 상태창을 토글하는 함수
	UFUNCTION()
	void HandleBodyPartChanged(EBodyPart part, float percent, bool bInjured, bool bDisabled); // 현재 선택 부위의 변경을 처리하는 함수
	void UpdateBodyPartOverlay(EBodyPart part, float percent, bool bInjured, bool bDisabled); // 부위 상태와 전체 체력 바를 업데이트 하는 함수
	void HandleArmFractureInjury(bool bDisabled, EBodyPart part); // 팔 골절로 인한 2차 효과를 처리하는 함수
	void HandleEyeDamageInjury(); // 안구 손상으로 인한 2차 효과를 처리하는 함수

	bool bEyeDamageActive = false; // 안구 손상 부상이 발생했는지 확인하는 변수

	float actionUnlockTime = 0.f; // 액션 입력 차단 시간


	/* 인벤토리 시스템 */
	void OpenInventory(APlayerController* controller); // 인벤토리 창을 여는 함수
	void CloseInventory(APlayerController* controller); // 인벤토리 창을 닫는 함수


	/* 상호작용 시스템 */
	bool IsInteractionBlocked() const; // 상호작용을 막아야 하는지 확인하는 함수


	/* 대화 시스템 */
	UPROPERTY(VisibleAnywhere, Category = Dialogue)
	UDialogueComponent* dialogueComponent; // 대화 컴포넌트

	
	/* 창고 시스템 */
	UPROPERTY()
	TObjectPtr<AStorageContainer> currentStorage = nullptr; // 현재 열려 있는 창고


	/* 퀵슬롯 시스템 */
	void RefreshQuickSlotHUD(); // 퀵슬롯 HUD 전체를 갱신하는 함수
	void RefreshQuickSlotCooldownUI(); // 퀵슬롯 쿨타임 UI를 갱신하는 함수

	UFUNCTION()
	void HandleQuickSlotChanged(EQuickSlot quickSlot); // 퀵슬롯 등록 변경 시 실행되는 함수

	bool bQuickSlotActive = false; // 퀵슬롯 조작 중인지 여부


	/* 오버레이 */
	void InitializeWindlessOverlay(); // 오버레이를 초기화하는 함수
	void SetHUDStamina(); // HUD의 스태미나 바를 설정하는 함수
	void SetHUDChaos(); // HUD의 정신력 바를 설정하는 함수
	UPROPERTY()
	UWindlessOverlay* windlessOverlay; // 오버레이를 저장하는 변수


	/* 기타 헬퍼 함수*/
	void BindCallbacks();
	void InitializePostProcessingEffect();

public:
	/* 변수 Getter 함수 */
	FORCEINLINE ECharacterState GetCharacterState() const { return characterState; }
	FORCEINLINE EActionState GetActionState() const { return actionState; }
	FORCEINLINE bool IsMounted() const { return bIsMounted; }
	FORCEINLINE AWeapon* GetLeftHandWeapon() const { return leftHandWeapon; }
	FORCEINLINE AWeapon* GetRightHandWeapon() const { return rightHandWeapon; }
	FORCEINLINE UBowComponent* GetBow() const { return bow; }
	FORCEINLINE UPlayerStaggerComponent* GetStaggerComponent() const { return staggerComponent; }
	FORCEINLINE bool IsGuarding() const { return combat && combat->IsGuarding(); }
	FORCEINLINE bool IsGuardOnLeftHand() const { return combat && combat->IsGuardOnLeftHand(); }
	UFUNCTION(BlueprintPure, Category = "Death")
	FORCEINLINE bool IsDead() const { return bIsDead; }
};