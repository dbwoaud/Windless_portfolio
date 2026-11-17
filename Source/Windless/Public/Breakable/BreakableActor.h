#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/HitInterface.h"
#include "BreakableActor.generated.h"

class UGeometryCollectionComponent;
class UCapsuleComponent;

UCLASS()
class WINDLESS_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	ABreakableActor();
	virtual void Tick(float DeltaTime) override; 
	virtual void GetHit_Implementation(const FVector& impactPoint, AActor* hitter) override; // 피격 시 실행되는 함수(IHitInterface 오버라이드)
	void SpawnTreasureItem(UWorld* world); // 파괴 시 보물 아이템을 생성하는 함수

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UGeometryCollectionComponent* geometryCollection; // 파괴 가능한 객체의 지오메트리 컬렉션 컴포넌트

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UCapsuleComponent* capsule; // 캡슐 콜라이더 컴포넌트 

private:
	UPROPERTY(EditAnywhere, Category = "Breakable Properties")
	TArray<TSubclassOf<class ATreasure>> treasureClasses; // 액터 파괴 시 스폰되는 보물 아이템 목록

	bool bBroken = false; // 현재 액터가 파괴된 상태인지 확인하는 변수
};
