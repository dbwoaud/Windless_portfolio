#pragma once

#include "CoreMinimal.h"
#include "Staging/BaseStagingActor.h"
#include "BridgeCollapseEffectActor.generated.h"

class UStaticMeshComponent;
class UPhysicsConstraintComponent;

UCLASS()
class WINDLESS_API ABridgeCollapseEffectActor : public ABaseStagingActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;


	/* 연출 제어*/
	virtual void HandlePlay() override;


	/* 다리 구성 */
	UPROPERTY(EditInstanceOnly, Category = "Bridge")
	TArray<TObjectPtr<AActor>> collapsingSegments; // 다리 세그먼트 액터 목록

	UPROPERTY(EditAnywhere, Category = "Bridge")
	bool bAnchorStart = true; // 다리 시작 지점 고정 여부

	UPROPERTY(EditAnywhere, Category = "Bridge")
	bool bAnchorEnd = true;   // 다리 끝 지점 고정 여부


	/* 붕괴 설정 */
	UPROPERTY(EditAnywhere, Category = "Bridge|Settings", meta = (ClampMin = "0.0"))
	float linearBreakThreshold = 50000.f;  // 선형 파괴 임계값

	UPROPERTY(EditAnywhere, Category = "Bridge|Settings", meta = (ClampMin = "0.0"))
	float angularBreakThreshold = 50000.f; // 각 파괴 임계값

	UPROPERTY(EditAnywhere, Category = "Bridge|Settings")
	int32 breakStartIndex = -1; // ★수정: 붕괴 구간 시작 세그먼트 인덱스

	UPROPERTY(EditAnywhere, Category = "Bridge|Settings")
	int32 breakEndIndex = -1;   // ★수정: 붕괴 구간 끝 세그먼트 인덱스

private:
	/* 붕괴 설정 */
	void SetConstraints(); // 세그먼트를 위한 물리 컨스트레인트를 설정하는 함수
	UStaticMeshComponent* GetSegmentMesh(AActor* seg) const; // 현재 액터의 정적 메쉬를 반환하는 함수 



	UPROPERTY()
	TArray<TObjectPtr<UPhysicsConstraintComponent>> constraints; // 생성된 컨스트레인트
};