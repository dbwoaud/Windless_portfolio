#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

enum class EItemState : uint8 // 아이템 상태를 설정하는 Enum
{
	EIS_Hovering,
	EIS_Equipped
};

UCLASS()
class WINDLESS_API AItems : public AActor
{
	GENERATED_BODY()
	
public:	
	AItems();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	// 다른 액터와 overlap이 시작되었을 때 실행되는 함수 
	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 다른 액터와 overlap이 끝났을 때 실행되는 함수 
	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ItemMesh; // 정적 메시

	UPROPERTY(VisibleAnywhere)
	USphereComponent* sphere; // 콜라이더


	/* 상태 */
	EItemState itemState = EItemState::EIS_Hovering; // 아이템 상태


	/* 연출 */
	virtual void SpawnPickupEffect(); // 픽업 VFX 효과를 생성하는 함수
	virtual void SpawnPickupSound(); // 픽업 사운드를 생성하는 함수
	UFUNCTION(BlueprintPure)
	float TransformedSin(); // 사인 함수

	UPROPERTY(EditAnywhere)
	UNiagaraComponent* itemEffect; // 아이템 VFX

	UPROPERTY(EditAnywhere)
	USoundBase* pickupSound; // 아이템 픽업 사운드

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float amplitude = 0.25f; // 사인 함수 진폭

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float timeConstant = 5.f; // 사인 함수 시간 상수

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float runningTime; // 현재 실행시간

	UPROPERTY(EditAnywhere)
	UNiagaraSystem* pickupEffect; // 아이템 픽업 VFX 
};