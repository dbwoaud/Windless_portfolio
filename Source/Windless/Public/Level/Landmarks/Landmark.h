#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Landmark.generated.h"

class USphereComponent;

UCLASS()
class WINDLESS_API ALandmark : public AActor
{
	GENERATED_BODY()

public:
	ALandmark();
	virtual void OnConstruction(const FTransform& transform) override;

protected:
	virtual void BeginPlay() override;


	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Landmark")
	USceneComponent* sceneRoot; // 루트

	UPROPERTY(VisibleAnywhere, Category = "Landmark")
	USphereComponent* discoverySphere; // 발견 판정 구체 


	/* 랜드마크 설정 */
	UPROPERTY(EditAnywhere, Category = "Landmark", meta = (ClampMin = "100.0"))
	float discoveryRadius = 1500.f; // 발견 반경

	UPROPERTY(EditInstanceOnly, Category = "Landmark")
	FName landmarkId = NAME_None; // 고유 ID

	UPROPERTY(EditInstanceOnly, Category = "Landmark")
	FText displayName; // 지도 표시 이름


	/* 블루프린트용 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Landmark")
	void OnDiscoveredBP(); // 랜드마크 최초 발견 시 실행되는 블루프린트용 함수

private:
	/* 랜드마크 설정 */
	// 발견 판정 구체에 오버랩 시작 시 실행되는 함수
	UFUNCTION()
	void OnDiscoveryBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	/* 변수 Getter 함수 */
	FORCEINLINE FName GetLandmarkId() const { return landmarkId; }
	FORCEINLINE FText GetDisplayName() const { return displayName; }
};