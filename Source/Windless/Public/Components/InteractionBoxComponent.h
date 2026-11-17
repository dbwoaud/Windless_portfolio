#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "InteractionBoxComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UInteractionBoxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UInteractionBoxComponent();

protected:
	virtual void BeginPlay() override;

private:
	// 박스 콜리전 오버랩 시작 시 실행되는 함수
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 박스 콜리전 오버랩 이탈 시 실행되는 함수
	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
