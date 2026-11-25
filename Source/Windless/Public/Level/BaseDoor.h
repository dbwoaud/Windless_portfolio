#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"
#include "BaseDoor.generated.h"

class UInteractionSphereComponent;
class ABaseStagingActor;

UENUM(BlueprintType)
enum class EDoorType : uint8
{
	EDT_LeftSingle  UMETA(DisplayName = "왼쪽 여닫이"),
	EDT_RightSingle UMETA(DisplayName = "오른쪽 여닫이"),
	EDT_Double      UMETA(DisplayName = "양문")
};

/* 문을 열 수 있는 쪽. 문의 오른쪽 벡터(+Y)가 기준이다 */
UENUM(BlueprintType)
enum class EDoorOpenSide : uint8
{
	EDOS_Both  UMETA(DisplayName = "양쪽"),
	EDOS_Right UMETA(DisplayName = "오른쪽(+Y)에서만"),
	EDOS_Left  UMETA(DisplayName = "왼쪽(-Y)에서만")
};

UCLASS()
class WINDLESS_API ABaseDoor : public AActor, public IInteractableInterface
{
	GENERATED_BODY()
	
public:	
	ABaseDoor();
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Interact_Implementation(AActor* interactor) override; // 문과 상호작용을 수행하는 함수
	virtual FText GetInteractionText_Implementation() override; // 상호작용 텍스트를 반환하는 함수
	virtual bool CanInteract_Implementation(AActor* interactor) override; // 현재 상호작용 가능한 상태인지 확인하는 함수


	/* 문 동작 */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void OpenDoor(); // 문 열기 시 실행되는 C++용 함수

	UFUNCTION(BlueprintCallable, Category = "Door")
	void CloseDoor(); // 문 닫기 시 실행되는 C++용 함수

protected:
	virtual void BeginPlay() override;
	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	UInteractionSphereComponent* interactionSphere; // 상호작용 감지 컴포넌트


	/* 문 구조 */
	UPROPERTY(VisibleAnywhere, Category = "Door")
	USceneComponent* doorRoot; // 문 중심

	UPROPERTY(VisibleAnywhere, Category = "Door")
	USceneComponent* leftHinge;   // 왼쪽 문 경첩

	UPROPERTY(VisibleAnywhere, Category = "Door")
	UStaticMeshComponent* leftLeaf; // 왼쪽 문

	UPROPERTY(VisibleAnywhere, Category = "Door")
	USceneComponent* rightHinge;  // 오른쪽 문 경첩

	UPROPERTY(VisibleAnywhere, Category = "Door")
	UStaticMeshComponent* rightLeaf; // 오른쪽 문


	/* 문 설정 */
	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	EDoorType doorType = EDoorType::EDT_Double; // 문 종류

	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	float doorWidth = 907.f; // 문 너비

	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	float openAngle = 90.f; // 문 열림 각도

	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	float openSpeed = 4.f;  // 문 열림 속도

	UPROPERTY(EditAnywhere, Category = "Door|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float openRatio = 1.f; // 문 열림 정도

	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	bool bOpenAwayFromInteractor = true; // 문 열림 방향(반대 방향으로 열지 여부)

	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	bool bCanInteractableOnce = false;   // 한 번만 상호작용이 가능한 지 여부

	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	bool bCanInteractable = true;   // 현재 상호작용이 가능한 지 여부

	UPROPERTY(EditAnywhere, Category = "Door|Settings")
	FText interactionText = FText::FromString(TEXT("열기")); // 상호작용 텍스트


	/* 지름길 문 */
	UPROPERTY(EditInstanceOnly, Category = "Door|Shortcut")
	FName doorId; // 채우면 한 번 연 뒤 열린 채로 남고 세이브에 적힌다. 문마다 다른 이름을 쓴다

	UPROPERTY(EditAnywhere, Category = "Door|Shortcut")
	EDoorOpenSide openFrom = EDoorOpenSide::EDOS_Both; // 닫힌 문을 열 수 있는 쪽

	UPROPERTY(EditAnywhere, Category = "Door|Shortcut")
	FText lockedText = FText::FromString(TEXT("이쪽에서는 열리지 않는다")); // 못 여는 쪽에서 보이는 글


	/* 문 연출 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorStartOpen();  // 문 열기 시작 시 실행되는 블루프린트용 함수

	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnDoorStartClose(); // 문 닫기 시작 시 실행되는 블루프린트용 함수


private:	
	/* 상태 확인 */
	bool bOpen = false; // 목표 문 상태
	bool bMoving = false; // 문이 움직이는 상태인지 여부
	float currentAlpha = 0.f; // 현재 문 상태(0=닫힘, 1=열림)
	float openDirection = 1.f; // 현재 문 열림 방향


	/* 문 설정 */
	void SetupDoor(); // 문 종류에 따라 미러링과 힌지 위치를 자동 배치하는 함수


	/* 문 연출 */
	void ApplyDoorRotation(float alpha); // 문에 회전을 적용하는 함수
	void UpdateOpenDirection(AActor* interactor); // 상호작용 위치로 문 열림 방향을 결정하는 함수
	UPROPERTY(EditAnywhere, Category = "Door|Staging")
	bool bPlayEffectOnOpen = false; // 열림 시작 시 연출 재생 여부

	UPROPERTY(EditInstanceOnly, Category = "Door|Staging")
	TObjectPtr<ABaseStagingActor> openEffect; // 열림 시 재생할 연출

	bool bPlayingOpenEffect = false; // 현재 연출 재생 중인지 여부


	/* 지름길 문 */
	bool bLockedSide = false; // 마지막으로 확인한 상호작용자가 못 여는 쪽에 있었는지 여부
	bool IsOnOpenSide(const AActor* interactor) const; // 상호작용자가 열 수 있는 쪽에 있는지 확인하는 함수

	UFUNCTION()
	void RestoreOpened(); // 세이브에 열린 문으로 적혀 있으면 열린 상태로 맞추는 함수
};