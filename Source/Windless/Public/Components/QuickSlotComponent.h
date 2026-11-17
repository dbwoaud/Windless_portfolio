#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Characters/BodyPartTypes.h"
#include "Items/ItemTypes.h"
#include "Items/QuickSlotTypes.h"
#include "QuickSlotComponent.generated.h"

class AWindlessCharacter;
class UInventoryComponent;
class UBodyPartComponent;
class UBodyPartRadialWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickSlotHighlightChanged, EQuickSlot, quickSlot);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UQuickSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuickSlotComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	/* 입력 */
	void HandleSlotPressed(EQuickSlot quickSlot); // 퀵슬롯 단축키를 눌렀을 때 실행되는 함수
	void HandleSlotReleased(EQuickSlot quickSlot); // 퀵슬롯 단축키를 뗐을 때 실행되는 함수
	bool HandleCancelInput(); // 우클릭/ESC 취소 입력을 처리하는 함수


	/* 제어 */
	void ForceCancel(); // 사망/피격 경직 시 퀵슬롯을 강제 종료하는 함수
	void NotifyItemUsed(FName itemId); // 아이템 사용이 실제로 완료되었을 때 쿨다운을 적용하는 함수


	/* 조회 */
	UFUNCTION(BlueprintPure, Category = "QuickSlot")
	bool IsQuickSlotActive() const { return bRadialOpen; } // 퀵슬롯 UI가 출력 중인지 확인하는 함수

	UFUNCTION(BlueprintPure, Category = "QuickSlot|Cooldown")
	float GetCooldownRemaining(FName itemId) const; // 남은 재사용 대기시간을 반환하는 함수

	UFUNCTION(BlueprintPure, Category = "QuickSlot|Cooldown")
	float GetSlotCooldownRatio(EQuickSlot quickSlot) const; // 슬롯의 쿨다운 진행 비율을 반환하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "QuickSlot|Event")
	FOnQuickSlotHighlightChanged OnHighlightChanged; // 하이라이트 대상 슬롯이 바뀔 때 호출되는 델리게이트

protected:
	virtual void BeginPlay() override;

private:
	/* 검증 */
	bool ValidateSlotCall(EQuickSlot quickSlot, int64& outUID, FItemData& outData) const; // 퀵슬롯 호출 검증을 수행하는 함수
	bool PassesUseCondition(const FItemData& data) const; // 사용 조건 검증값을 판정하는 함수


	/* 라디얼 제어 */
	void OpenRadial(EQuickSlot quickSlot); // 방사형 부위 선택 UI를 출력하는 함수
	void CloseRadial(); // 방사형 UI를 제거하고 입력을 게임플레이로 반환하는 함수
	void SwapPendingItem(EQuickSlot quickSlot); // 라디얼 유지 상태에서 사용 대상 아이템만 교체하는 함수
	void RefreshSectorStatus(); // 모든 섹터의 체력/부상 표시를 갱신하는 함수


	/* 부위 선택 연산 */
	void UpdateSelection(float deltaTime); // 마우스 이동량을 누적해 대상 부위를 판정하는 함수
	EBodyPart ResolvePartFromVector() const; // 선택 벡터로부터 대상 부위를 산출하는 함수


	/* 사용 연산 */
	void ConfirmUse(); // 단축키 해제 시점에 부위를 확정하고 사용하는 함수
	void UseWithPart(EQuickSlot quickSlot, EBodyPart part); // 확정된 부위로 아이템 사용을 개시하는 함수
	void StartCooldown(FName itemId, float cooldown); // 재사용 대기시간을 시작하는 함수


	/* 컴포넌트 */
	UPROPERTY()
	TObjectPtr<AWindlessCharacter> character = nullptr; // 소유 캐릭터

	UPROPERTY()
	TObjectPtr<UInventoryComponent> inventory = nullptr; // 소유 인벤토리 컴포넌트

	UPROPERTY()
	TObjectPtr<UBodyPartComponent> bodyParts = nullptr; // 소유 부위 컴포넌트


	/* 설정값 */
	UPROPERTY(EditAnywhere, Category = "QuickSlot", meta = (ClampMin = "0.05"))
	float holdThreshold = 0.3f; // 라디얼이 출력되기까지의 단축키 유지 시간

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Selection", meta = (ClampMin = "0.01"))
	float quickSlotSensitivity = 1.f; // 퀵슬롯 전용 마우스 감도

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Selection", meta = (ClampMin = "0.0"))
	float inputDetectThreshold = 3.f; // 선택 활성화를 판정하는 최소 입력 감지값

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Selection", meta = (ClampMin = "1.0"))
	float deadZoneRadius = 40.f; // 몸통 판정 중앙 데드존 반경

	UPROPERTY(EditAnywhere, Category = "QuickSlot|Selection", meta = (ClampMin = "1.0"))
	float maxRadius = 120.f; // 선택 벡터 최대 반경

	UPROPERTY(EditAnywhere, Category = "QuickSlot")
	TSubclassOf<UBodyPartRadialWidget> radialWidgetClass; // 방사형 부위 선택 위젯 클래스


	/* 상태 */
	UPROPERTY()
	TObjectPtr<UBodyPartRadialWidget> radialWidget = nullptr; // 현재 열린 라디얼 위젯

	EQuickSlot activeQuickSlot = EQuickSlot::EQS_None; // 현재 눌려 있는 퀵슬롯
	EBodyPart currentPart = EBodyPart::EBP_None; // 현재 확정된 대상 부위
	FVector2D selectionVector = FVector2D::ZeroVector; // 누적 마우스 이동량
	float holdTime = 0.f; // 단축키 유지 시간
	bool bRadialOpen = false; // 라디얼 출력 여부
	bool bSelectionStarted = false; // 부위 선택 활성화 플래그

	TSet<EQuickSlot> cancelledSlots; // 취소 상태로 기록된 단축키 집합
	TMap<FName, double> cooldownEndTimes; // 아이템별 재사용 대기 종료 시각
	TMap<FName, float> cooldownDurations; // 아이템별 재사용 대기 총 시간
};