#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoulMarker.generated.h"

class USphereComponent;

UCLASS()
class WINDLESS_API ASoulMarker : public AActor
{
	GENERATED_BODY()

public:
	ASoulMarker();


	/* 초기화 함수 */
	void Initialize(int32 inMoltStones); // 남은 탈피석 수량을 설정하는 함수

protected:
	virtual void BeginPlay() override;


	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "SoulMarker")
	USceneComponent* sceneRoot; // 루트 컴포넌트 

	UPROPERTY(VisibleAnywhere, Category = "SoulMarker")
	USphereComponent* pickupSphere; // 회수 판정 구체


	/* 회수 설정 */
	UPROPERTY(EditAnywhere, Category = "SoulMarker", meta = (ClampMin = "50.0"))
	float pickupRadius = 180.f; // 회수 반경


	/* 블루프린트용 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SoulMarker")
	void OnRecoveredBP(); // 탈피석 회수 시 실행되는 블루프린트용 함수

private:
	/* 회수 설정 */
	// 회수 반경 오버랩 시작 시 실행되는 함수
	UFUNCTION()
	void OnPickupBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	int32 moltStones = 0; // 남은 탈피석

public:
	/* 변수 Getter 함수 */
	UFUNCTION(BlueprintPure, Category = "SoulMarker")
	FORCEINLINE int32 GetMoltStones() const { return moltStones; }
};