#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionWidget.generated.h"

class UTextBlock;

UCLASS()
class WINDLESS_API UInteractionWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetActionText(const FText& inActionText); // 동작 텍스트를 설정하는 함수

private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* actionText; // 동작 테스트

};