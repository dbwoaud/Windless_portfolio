#pragma once

#include "CoreMinimal.h"
#include "Items.h"
#include "Soul.generated.h"

class UNiagaraSystem;

UCLASS()
class WINDLESS_API ASoul : public AItems
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	
	// 다른 액터와 overlap이 시작되었을 때 실행되는 함수 
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	int32 souls; // 소울의 개수
	

	/* 연출 */
	void MoveSoulToDesireZ(float DeltaTime); // 목표 Z값까지 소울을 이동시키는 함수

	UPROPERTY(EditAnywhere)
	float moveSpeed = -15.f; // 소울 하강 속도
	double desiredZ; // 목표 Z값

public:
	/* 변수의 Getter, Setter 함수 */
	FORCEINLINE int32 GetSouls() const { return souls; }
	FORCEINLINE void SetSouls(int32 numberOfSouls) { souls = numberOfSouls; }
};