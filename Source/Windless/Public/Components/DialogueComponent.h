#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.generated.h"

class UEnhancedInputComponent;
class UInputAction;
class UDataTable;
class UDialogueWidget;
class AWindlessCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueFinished);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WINDLESS_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDialogueComponent();

	/* 입력 */
	void BindDialogueInputs(UEnhancedInputComponent* enhancedInputComponent); // 대화용 입력 컨텍스트에 입력 키를 할당하는 함수

	/* 상태 확인 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialogueActive() const { return bDialogueActive; }

	/* 대화 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(UDataTable* dialogueTable); // 대화 시작 시 실행되는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueFinished onDialogueFinished; // 대화 종료 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;

private:
	/* 대화 설정 */
	UPROPERTY()
	AWindlessCharacter* ownerCharacter = nullptr; // 컴포넌트를 소유한 캐릭터

	UPROPERTY()
	UDataTable* currentTable = nullptr; // 대사 정보가 저장된 테이블

	UPROPERTY()
	TArray<FName> lineRowNames; // 대화행 이름

	int32 currentLineIndex = 0; // 현재 대화 인덱스

	bool bDialogueActive = false; // 대화 활성 여부


	/* 입력 */
	UPROPERTY(EditAnywhere, Category = "Dialogue|Input")
	UInputAction* advanceDialogueAction; // 대화 진행 입력 키


	/* 대화 진행 */
	void AdvanceDialogue(); // 대화를 진행하는 함수
	void ShowCurrentLine(); // 현재 대화행을 출력하는 함수
	void EndDialogue(); // 대화 완료 시 실행되는 함수


	/* UI */
	UPROPERTY(EditAnywhere, Category = "Dialogue|UI")
	TSubclassOf<UDialogueWidget> dialogueWidgetClass; // 대화 위젯 클래스

	UPROPERTY()
	UDialogueWidget* dialogueWidget = nullptr; // 현재 대화 위젯

public:
	/* 변수 Getter 함수 */
};
