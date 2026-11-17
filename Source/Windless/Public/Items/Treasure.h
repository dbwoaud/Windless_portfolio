#pragma once

#include "CoreMinimal.h"
#include "Items.h"
#include "Treasure.generated.h"

UCLASS()
class WINDLESS_API ATreasure : public AItems
{
	GENERATED_BODY()
protected:
	// 다른 액터와 overlap이 시작되었을 때 실행되는 함수 
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Treasure Properties")
	int32 gold; // 골드의 개수

public:
	/* 변수의 Getter 함수 */
	FORCEINLINE int32 GetGold() const { return gold; }
};
