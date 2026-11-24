#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInputModeComponent.generated.h"

class UWindlessInputConfig;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;

UENUM()
enum class EPlayerInputMode : uint8
{
	EPIM_Gameplay  UMETA(DisplayName = "Gameplay"),  // 이동 및 전투
	EPIM_Dialogue  UMETA(DisplayName = "Dialogue"),  // 대화 및 연출
	EPIM_Inventory UMETA(DisplayName = "Inventory")  // 인벤토리 조작
};

/*
 * 플레이어 입력 관리 컴포넌트
 */

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UPlayerInputModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerInputModeComponent();

	/* 초기화 */
	void Initialize(const UWindlessInputConfig* inConfig); // 기본 컨텍스트로 초기화하는 함수


	/* 입력 모드 설정 */
	void SetMode(EPlayerInputMode newMode); // 입력 모드를 바꾸고 해당 컨텍스트로 교체하는 함수

	
	/* 상호작용 입력 모드 */
	void SetInteractionContextActive(bool bActive); // 상호작용 컨텍스트를 겹쳐 켜거나 끄는 함수


private:
	/* 입력 모드 설정 */
	UPROPERTY()
	TObjectPtr<const UWindlessInputConfig> config = nullptr; // 입력 설정 에셋

	EPlayerInputMode currentMode = EPlayerInputMode::EPIM_Gameplay; // 현재 입력 모드
	bool bInteractionContextActive = false; // 상호작용 컨텍스트 활성 여부


	/* 캐시 변수 */
	static constexpr int32 GameplayPriority = 0; // 기본 컨텍스트 우선순위
	static constexpr int32 OverlayPriority = 1;  // 기타 컨텍스트 우선순위

	UEnhancedInputLocalPlayerSubsystem* GetInputSubsystem() const; // 입력 서브시스템을 반환하는 함수
	const UInputMappingContext* GetModeContext(EPlayerInputMode mode) const; // 모드의 입력 컨텍스트를 반환하는 함수

public:
	/* 변수 Getter 함수 */
	FORCEINLINE EPlayerInputMode GetMode() const { return currentMode; }
	FORCEINLINE bool IsInteractionContextActive() const { return bInteractionContextActive; }
};