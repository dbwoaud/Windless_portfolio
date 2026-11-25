#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HealthBarComponent.generated.h"

class UHealthBar;

UCLASS()
class WINDLESS_API UHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	void SetHealthPercent(float percent); // 퍼센트 단위 현재 생명력을 설정하는 함수

private:
	UPROPERTY()
	UHealthBar* healthBarWidget; // 체력바 위젯
};
