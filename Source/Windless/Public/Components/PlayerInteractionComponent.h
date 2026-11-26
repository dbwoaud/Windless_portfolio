#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractionComponent.generated.h"

class APlayerController;
class UInteractionWidget;
class UPlayerInputModeComponent;

DECLARE_DELEGATE_RetVal(bool, FInteractionBlockQuery);

/*
 * 플레이어 상호작용 컴포넌트
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UPlayerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerInteractionComponent();


	/* 초기화 */
	void Initialize(APlayerController* playerController); // 상호작용 위젯을 초기화하는 함수


	/* 상호작용 */
	void RegisterCandidate(AActor* interactable); // 감지 범위에 들어온 상호작용 대상을 등록하는 함수
	void UnregisterCandidate(AActor* interactable); // 감지 범위를 벗어난 상호작용 대상을 해제하는 함수
	void Refresh(); // 현재 대상, 상호작용 위젯, 상호작용 입력 컨텍스트를 갱신하는 함수
	bool TryInteract(); // 현재 대상과 상호작용을 시도하는 함수


	/* 델리게이트 */
	FInteractionBlockQuery blockQuery; // 상호작용을 막아야 하는 상태인지 소유자에게 묻는 델리게이트

private:
	/* 캐시 변수 */
	UPROPERTY()
	TObjectPtr<UPlayerInputModeComponent> inputMode = nullptr; // 플레이어 입력 모드 컴포넌트


	/* 상태 확인*/
	bool IsBlocked() const; // 상호작용 차단 여부를 확인하는 함수


	/* 상호작용 */
	AActor* FindNearestCandidate(); // 상호작용 가능한 후보 중 가장 가까운 대상을 찾는 함수
	void ApplyPrompt(); // 현재 대상에 맞는 상호작용 위젯과 입력 컨텍스트를 적용하는 함수

	UPROPERTY()
	TArray<TObjectPtr<AActor>> candidates; // 상호작용 대상 후보

	UPROPERTY()
	TObjectPtr<AActor> currentTarget = nullptr; // 현재 상호작용 대상

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TSubclassOf<UInteractionWidget> promptWidgetClass; // 상호작용 안내 위젯 클래스

	UPROPERTY()
	TObjectPtr<UInteractionWidget> promptWidget = nullptr; // 상호작용 안내 위젯 인스턴스

public:
	/* 변수 Getter 함수 */
	FORCEINLINE AActor* GetCurrentTarget() const { return currentTarget; }
};