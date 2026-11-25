#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WindlessHUD.generated.h"

class UWindlessOverlay;

UCLASS()
class WINDLESS_API AWindlessHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = Windless)
	TSubclassOf<UWindlessOverlay> windlessOverlayClass; // 오버레이 위젯 클래스 

	UPROPERTY()
	UWindlessOverlay* windlessOverlay; // 오버레이 위젯 인스턴스

public:
	/* 변수의 Getter 함수 */
	FORCEINLINE UWindlessOverlay* GetWindlessOverlay() const { return windlessOverlay; }
};
