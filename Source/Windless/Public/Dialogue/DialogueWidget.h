#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidget.generated.h"

class UTextBlock;

UCLASS()
class WINDLESS_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetLine(const FText& speaker, const FText& line); // 대화창의 화자와 대사를 설정하는 함수

private:
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* speakerText; // 화자 이름

	UPROPERTY(meta = (BindWidget))
	UTextBlock* dialogueText; // 대사 본문
};