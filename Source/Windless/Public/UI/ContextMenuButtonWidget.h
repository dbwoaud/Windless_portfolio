#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/BodyPartTypes.h"
#include "ContextMenuButtonWidget.generated.h"

class UButton;
class UTextBlock;

/* 우클릭 팝업에서 실행 가능한 동작 종류 */
UENUM(BlueprintType)
enum class EItemContextAction : uint8
{
	EICA_None        UMETA(DisplayName = "None"),
	EICA_Equip       UMETA(DisplayName = "Equip"),       // 장착
	EICA_Unequip     UMETA(DisplayName = "Unequip"),     // 해제
	EICA_Use         UMETA(DisplayName = "Use"),         // 사용
	EICA_UsePartMenu UMETA(DisplayName = "UsePartMenu"), // 부위 선택 서브메뉴 열기
	EICA_UsePart     UMETA(DisplayName = "UsePart"),     // 부위 사용
	EICA_Split       UMETA(DisplayName = "Split"),       // 나누기
	EICA_Drop        UMETA(DisplayName = "Drop"),        // 버리기
	EICA_Info        UMETA(DisplayName = "Info"),        // 정보
	EICA_Back        UMETA(DisplayName = "Back")         // 서브메뉴에서 되돌아가기
};

DECLARE_DELEGATE_TwoParams(FOnContextActionClicked, EItemContextAction, EBodyPart);

UCLASS()
class WINDLESS_API UContextMenuButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void SetupButton(EItemContextAction inAction, const FText& label, bool bEnabled, EBodyPart inTargetPart = EBodyPart::EBP_None); // 버튼의 동작과 텍스트를 설정하는 함수


	/* 델리게이트 */
	FOnContextActionClicked OnContextActionClicked; // 버튼 클릭 시 호출되는 델리게이트

protected:
	virtual void NativeConstruct() override;
	

	/* 버튼 동작 */
	UFUNCTION()
	void HandleClicked(); // 버튼 클릭 시 실행되는 함수

	UFUNCTION()
	void HandleHovered(); // 마우스 호버 시 실행되는 함수

	UFUNCTION()
	void HandleUnhovered(); // 마우스 호버 해제 시 실행되는 함수


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> actionButton; // 클릭 대상 버튼

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> actionText; // 버튼 텍스트


	/* 색상 설정 */
	UPROPERTY(EditAnywhere, Category = "ContextMenu|Color")
	FLinearColor normalTextColor = FLinearColor(FColor(190, 182, 164, 255)); // 기본 텍스트 색

	UPROPERTY(EditAnywhere, Category = "ContextMenu|Color")
	FLinearColor hoveredTextColor = FLinearColor(FColor(244, 236, 214, 255)); // 호버 텍스트 색

	UPROPERTY(EditAnywhere, Category = "ContextMenu|Color")
	FLinearColor disabledTextColor = FLinearColor(0.40f, 0.38f, 0.35f, 1.f); // 비활성 텍스트 색

private:
	/* 버튼 동작 */
	EItemContextAction action = EItemContextAction::EICA_None; // 버튼 실행 동작
	EBodyPart targetPart = EBodyPart::EBP_None; // 부위 선택 버튼일 때의 대상 부위
	bool bActionEnabled = true; // 실행 가능 여부

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "ContextMenu")
	FORCEINLINE EItemContextAction GetAction() const { return action; }
};