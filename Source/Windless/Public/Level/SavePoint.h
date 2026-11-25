#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"
#include "SavePoint.generated.h"

class UInteractionSphereComponent;
class UStaticMeshComponent;
class USphereComponent;
class AWindlessCharacter;
class USavePointSubsystem;

UCLASS()
class WINDLESS_API ASavePoint : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	ASavePoint();
	virtual void OnConstruction(const FTransform& transform) override; // 에디터에서도 구역 반경 반영


	/* 상호작용 인터페이스 */
	virtual void Interact_Implementation(AActor* interactor) override; // 세이브 포인트와 상호작용을 수행하는 함수
	virtual FText GetInteractionText_Implementation() override; // 상호작용 텍스트를 반환하는 함수
	virtual bool CanInteract_Implementation(AActor* interactor) override; // 현재 상호작용 가능한 상태인지 확인하는 함수


	/* 리스폰 정보 */
	FVector  GetRespawnLocation() const;
	FRotator GetRespawnRotation() const;


	/* 활성화 */
	UFUNCTION(BlueprintPure, Category = "SavePoint")
	bool IsActivated() const; // 이 세이브포인트가 활성화 상태인지 확인하는 함수

	void RefreshActivationState(); // 서브시스템의 활성화 상태를 표시, 구역 효과에 반영하는 함수


	/* 안전지대 */
	bool IsLocationInSafeZone(const FVector& location) const; // 위치가 이 세이브포인트의 안전지대 안인지 확인하는 함수

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "SavePoint")
	USceneComponent* sceneRoot; // 루트 씬 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "SavePoint")
	UStaticMeshComponent* savePointMesh; // 균열 정적 메쉬

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	UInteractionSphereComponent* interactionSphere; // 상호작용 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "SavePoint")
	USphereComponent* saveZoneSphere; // 빛 구역, 안전지대, 회복 구역 판정 구체


	/* 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "SavePoint")
	void OnActivatedBP(); // 세이브포인트 최초 활성화 시 1회만 실행되는 블루프린트용 함수

	UFUNCTION(BlueprintImplementableEvent, Category = "SavePoint")
	void OnRestedBP(); // 활성화된 세이브포인트에서 휴식할 때마다 실행되는 블루프린트용 함수

	UFUNCTION(BlueprintImplementableEvent, Category = "SavePoint")
	void OnActivationStateChangedBP(bool bActivated); // 활성화 표시를 갱신하는 블루프린트용 함수


	/* 세이브 포인트 설정 */
	UPROPERTY(VisibleAnywhere, Category = "SavePoint")
	USceneComponent* respawnPoint; // 부활 위치

	UPROPERTY(EditAnywhere, Category = "SavePoint")
	FText interactionText = FText::FromString(TEXT("휴식")); // 활성화 상태 상호작용 텍스트

	UPROPERTY(EditAnywhere, Category = "SavePoint")
	FText activateText = FText::FromString(TEXT("활성화")); // 비활성화 상태 상호작용 텍스트

	UPROPERTY(EditAnywhere, Category = "SavePoint", meta = (ClampMin = "100.0"))
	float saveZoneRadius = 600.f; // 빛 구역, 안전지대 반경

	UPROPERTY(EditInstanceOnly, Category = "SavePoint")
	FName savePointId = NAME_None; // 세이브 포인트 고유 ID

	UPROPERTY(EditInstanceOnly, Category = "SavePoint")
	FText displayName; // 지도, 메뉴 표시 이름

private:
	/* 구역 */
	UFUNCTION()
	void OnSaveZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSaveZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void HandlePlayerEnter(AWindlessCharacter* player); // 세이브 구역 진입을 처리하는 함수
	void HandlePlayerExit(AWindlessCharacter* player);  // 세이브 구역 이탈을 처리하는 함수
	void SetBodyPartRegen(AWindlessCharacter* player, bool bActive) const; // 부위 회복을 토글하는 함수


	/* 휴식 */
	void ApplyRestEffects(AWindlessCharacter* player); // 세이브 포인트 상호작용 시 자동 효과를 적용하는 함수


	/* 유틸 */
	USavePointSubsystem* GetSavePointSubsystem() const; // 세이브 포인트 서비시스템을 반환하는 함수

	UPROPERTY()
	TObjectPtr<AWindlessCharacter> overlappingPlayer = nullptr; // 구역 안의 플레이어

	bool bActivationStateInitialized = false; // 활성화 표시 최초 반영 여부
	bool bCachedActivated = false;            // 마지막으로 반영한 활성화 상태

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "SavePoint")
	FORCEINLINE FName GetSavePointId() const { return savePointId; }

	UFUNCTION(BlueprintPure, Category = "SavePoint")
	FORCEINLINE FText GetDisplayName() const { return displayName; } 
};