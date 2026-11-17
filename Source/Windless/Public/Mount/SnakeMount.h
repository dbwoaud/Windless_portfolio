#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SnakeMount.generated.h"

class USkeletalMeshComponent;
class UBoxComponent;
class UAnimMontage;
class UNiagaraSystem;

DECLARE_MULTICAST_DELEGATE(FOnEmergeFinished);
DECLARE_MULTICAST_DELEGATE(FOnSubmergeFinished);

UCLASS()
class WINDLESS_API ASnakeMount : public AActor
{
	GENERATED_BODY()
	
public:	
	ASnakeMount();

	/* 소환 */
	void Emerge();   // 탈 것 소환 연출을 수행하는 함수


	/* 퇴장 */
	void Submerge(); // 탈 것 퇴장 연출을 수행하는 함수


	/* 델리게이트 */
	FOnEmergeFinished OnEmergeFinished; // 탈 것 등장 연출 완료 시 실행되는 델리게이트
	FOnSubmergeFinished OnSubmergeFinished; // 탈 것 퇴장 연출 완료 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;

	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = Components)
	UBoxComponent* collisionBox; // 박스 콜라이더 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = Components)
	USkeletalMeshComponent* mountMesh; // 스켈레탈 메시 컴포넌트


	/* 소환 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* emergeMontage; // 등장 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = VFX)
	UNiagaraSystem* groundBurstEffect; // 흙먼지 시각 효과
	

	/* 탑승 */
	UPROPERTY(EditAnywhere, Category = Mount)
	FName riderSocketName = FName("RiderSocket"); // 탑승 소켓 이름


	/* 퇴장 */
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* submergeMontage; // 퇴장 애니메이션 몽타주

private:
	/* 소환 */
	void HandleEmergeEnded(UAnimMontage* montage, bool bInterrupted); // 탈 것 소환 애니메이션 몽타주 종료 시 실행되는 함수
	void SpawnGroundBurst(); // 흙먼지 시각 효과를 생성하는 함수

	/* 퇴장 */
	void HandleSubmergeEnded(UAnimMontage* montage, bool bInterrupted); // 탈 것 퇴장 애니메이션 몽타주 종료 시 실행되는 함수

public:
	/* 변수 getter 함수 */
	FORCEINLINE USkeletalMeshComponent* GetMountMesh() const { return mountMesh; }
	FORCEINLINE FName GetRiderSocketName() const { return riderSocketName; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return collisionBox; }
};