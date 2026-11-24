#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WindlessInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

/*
 * 플레이어 입력 설정 데이터 에셋
 */

UCLASS()
class WINDLESS_API UWindlessInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/* 입력 매핑 컨텍스트 */
	UPROPERTY(EditDefaultsOnly, Category = "Context")
	TObjectPtr<UInputMappingContext> gameplayContext; // 기본 입력 매핑 컨텍스트

	UPROPERTY(EditDefaultsOnly, Category = "Context")
	TObjectPtr<UInputMappingContext> dialogueContext; // 대화 및 연출 입력 매핑 컨텍스트

	UPROPERTY(EditDefaultsOnly, Category = "Context")
	TObjectPtr<UInputMappingContext> interactionContext; // 상호작용 입력 매핑 컨텍스트

	UPROPERTY(EditDefaultsOnly, Category = "Context")
	TObjectPtr<UInputMappingContext> inventoryContext; // 인벤토리 조작 입력 매핑 컨텍스트


	/* 이동 */
	UPROPERTY(EditDefaultsOnly, Category = "Action|Movement")
	TObjectPtr<UInputAction> movementAction; // 이동 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Movement")
	TObjectPtr<UInputAction> lookAction; // 회전 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Movement")
	TObjectPtr<UInputAction> sprintAction; // 달리기 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Movement")
	TObjectPtr<UInputAction> jumpAction; // 점프 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Movement")
	TObjectPtr<UInputAction> crouchAction; // 앉기 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Movement")
	TObjectPtr<UInputAction> dodgeAction; // 구르기 입력 액션


	/* 전투 */
	UPROPERTY(EditDefaultsOnly, Category = "Action|Combat")
	TObjectPtr<UInputAction> leftAttackAction; // 좌 공격 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Combat")
	TObjectPtr<UInputAction> rightAttackAction; // 우 공격 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Combat")
	TObjectPtr<UInputAction> leftStrongAttackAction; // 좌 강공격 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Combat")
	TObjectPtr<UInputAction> rightStrongAttackAction; // 우 강공격 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Combat")
	TObjectPtr<UInputAction> equipAction; // 무기 장착 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Combat")
	TObjectPtr<UInputAction> lockOnAction; // 락온 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Combat")
	TObjectPtr<UInputAction> switchTargetAction; // 락온 타겟 변경 입력 액션


	/* 상호작용 및 UI */
	UPROPERTY(EditDefaultsOnly, Category = "Action|Interaction")
	TObjectPtr<UInputAction> interactAction; // 상호작용 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Interaction")
	TObjectPtr<UInputAction> mountAction; // 탈 것 토글 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Interaction")
	TObjectPtr<UInputAction> bodyStatusToggleAction; // 부위 상태창 토글 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|Interaction")
	TObjectPtr<UInputAction> inventoryToggleAction; // 인벤토리 창 토글 입력 액션


	/* 퀵슬롯 */
	UPROPERTY(EditDefaultsOnly, Category = "Action|QuickSlot")
	TObjectPtr<UInputAction> quickSlotRecoveryAction; // 회복 퀵슬롯 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|QuickSlot")
	TObjectPtr<UInputAction> quickSlotConsume1Action; // 소비1 슬롯 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|QuickSlot")
	TObjectPtr<UInputAction> quickSlotConsume2Action; // 소비2 슬롯 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|QuickSlot")
	TObjectPtr<UInputAction> quickSlotConsume3Action; // 소비3 슬롯 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|QuickSlot")
	TObjectPtr<UInputAction> quickSlotConsume4Action; // 소비4 슬롯 입력 액션

	UPROPERTY(EditDefaultsOnly, Category = "Action|QuickSlot")
	TObjectPtr<UInputAction> quickSlotCancelAction; // 퀵슬롯 취소 입력 액션
};