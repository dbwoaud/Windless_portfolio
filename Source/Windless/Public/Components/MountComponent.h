#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MountComponent.generated.h"

class ACharacter;
class ASnakeMount;

UENUM(BlueprintType)
enum class EMountPhase : uint8
{
	EMP_Grounded UMETA(DisplayName = "Grounded"), // 미탑승 상태
	EMP_Mounting UMETA(DisplayName = "Mounting"), // 탑승 연출 중
	EMP_Mounted UMETA(DisplayName = "Mounted"), // 탑승 완료
	EMP_Dismounting UMETA(DisplayName = "Dismounting") // 탑승 해제 연출 중
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMountPhaseChanged, EMountPhase, phase);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WINDLESS_API UMountComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMountComponent();
	
	/* 탑승 */
	void Mount(); // 탈 것 소환 및 등장 연출을 시작하는 함수


	/* 탑승 해제 */
	void Dismount(); // 탈 것 탑승 해제 및 퇴장 연출을 시작하는 함수


	/* 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Mount")
	FOnMountPhaseChanged OnMountPhaseChanged; // 탑승 상태 변경 시 실행되는 델리게이트

protected:
	virtual void BeginPlay() override;

private:	
	/* 캐싱 변수 */
	UPROPERTY()
	ACharacter* ownerCharacter = nullptr; // 소유 캐릭터

	UPROPERTY()
	ASnakeMount* currentMount = nullptr;  // 현재 소환된 탈것


	/* 탑승 */
	void OnMountEmerged(); // 실제 탑승을 수행하는 함수
	bool CanMount() const; // 캐릭터가 탑승할 수 있는 상태인지 확인하는 함수
	bool TrySpawnMount(); // 탈 것 소환을 시도하는 함수
	void SetMountCollision(); // 탑승 시 캐릭터와 탈 것의 콜라이더를 설정하는 함수
	void MoveToMountSocket(); // 탑승 시 캐릭터를 탈 것 소켓으로 이동시키는 함수
	void ApplyMountEffect(); // 탑승 시 효과를 적용하는 함수


	/* 탑승 해제*/
	void OnMountSubmerged(); // 탈 것을 제거하는 함수
	void ResetMountCollision(); // 탑승 해제 시 캐릭터와 탈 것의 콜라이더를 리셋하는 함수
	void MoveToGround(); // 탑승 해제 시 캐릭터를 땅으로 이동시키는 함수
	void DisapplyMountEffect();// 탑승 해제 시 효과를 복구시키는 함수

	/* 탈 것 설정 */
	UPROPERTY(EditAnywhere, Category = "Mount")
	TSubclassOf<ASnakeMount> snakeMountClass; // 생성할 탈 것 클래스

	UPROPERTY(EditAnywhere, Category = "Mount")
	float mountSpeed = 1200.f; // 탑승 중 이동 속도

	UPROPERTY(EditAnywhere, Category = "Mount")
	float mountStepHeightBoost = 10.f; // 탑승 중 단차 높이

	UPROPERTY(EditAnywhere, Category = "Mount")
	FRotator mountMeshRotationOffset = FRotator::ZeroRotator; // 탈 것 회전 오프셋

public:
	/* 변수 getter 함수 */
	float GetCapsuleHalfHeight() const;
};