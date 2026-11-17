#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeakPointMarkerWidget.generated.h"

class UTextBlock;

UCLASS()
class WINDLESS_API UWeakPointMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/* 초기화 함수 */
	void SetupWarning(int32 weakPointCount); // 각성 1단계 경고를 표시하는 함수
	void SetupDetail(const FText& regionLabel); // 각성 2단계 이상 약점 부위 이름을 표시하는 함수

protected:
	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* labelText; // 눈금 또는 부위 이름 텍스트


	/* 블루프린트용 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category = "WeakPoint")
	void OnMarkerSetup(bool bDetail); // 마커 설정 시 실행되는 블루프린트용 함수
};