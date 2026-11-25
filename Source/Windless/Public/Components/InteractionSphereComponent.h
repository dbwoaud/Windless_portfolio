#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "InteractionSphereComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WINDLESS_API UInteractionSphereComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UInteractionSphereComponent();

protected:
	virtual void BeginPlay() override;

private:
	// 구형 콜라이더 오버랩 시작 시 실행되는 함수
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 구형 콜라이더 오버랩 이탈 시 실행되는 함수
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
