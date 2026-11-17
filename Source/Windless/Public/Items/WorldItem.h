#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"
#include "Items/ItemTypes.h"
#include "WorldItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UPointLightComponent;
class UInteractionSphereComponent;
class UDataTable;
class USoundBase;

UCLASS()
class WINDLESS_API AWorldItem : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AWorldItem();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Interact_Implementation(AActor* interactor) override; // 아이템을 획득하는 함수
	virtual FText GetInteractionText_Implementation() override; // 상호작용 텍스트를 반환하는 함수
	virtual bool CanInteract_Implementation(AActor* interactor) override; // 상호작용 가능 상태인지 확인하는 함수


	/* 초기화 */
	UFUNCTION(BlueprintCallable, Category = "WorldItem")
	void InitializeItem(FName inItemId, int32 inQuantity, float inDurability = -1.f); // 아이템 정보를 초기화하는 함수

protected:
	virtual void BeginPlay() override;


	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "WorldItem")
	TObjectPtr<USphereComponent> itemCollision; // 아이템 콜리전 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "WorldItem")
	TObjectPtr<UStaticMeshComponent> itemMesh; // 아이템 메시 컴포넌트 

	UPROPERTY(VisibleAnywhere, Category = "WorldItem")
	TObjectPtr<UNiagaraComponent> beamEffect; // 아이템 빛줄기 이펙트 컴포넌트 

	UPROPERTY(VisibleAnywhere, Category = "WorldItem")
	TObjectPtr<UPointLightComponent> beamLight; // 아이템 빛줄기 주변 조명 컴포넌트

	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	TObjectPtr<UInteractionSphereComponent> interactionSphere; // 아이템 상호작용 감지 컴포넌트


	/* 아이템 정보 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WorldItem|Item")
	FName itemId = NAME_None; // 원본 아이템 ID

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WorldItem|Item", meta = (ClampMin = "1"))
	int32 quantity = 1; // 수량

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WorldItem|Item")
	float durability = -1.f; // 내구도


	/* 원본 데이터 */
	bool LookupItemData(FItemData& outData) const; // 원본 데이터를 조회하는 함수


	/* 연출 */
	void ApplyPresentation(); // 등급 색상/스케일/메시를 적용하는 함수
	void PlayPickupEffect(); // 획득 연출을 재생하는 함수

	UPROPERTY(EditDefaultsOnly, Category = "WorldItem|Effect")
	TObjectPtr<UNiagaraSystem> fieldEffectAsset; // 아이템 빛줄기 이펙트 컴포넌트 

	UPROPERTY(EditAnywhere, Category = "WorldItem|Effect", meta = (ClampMin = "0.05"))
	float presentationScale = 1.f; // 빛줄기/메시 표시 배율

	UPROPERTY(EditDefaultsOnly, Category = "WorldItem|Effect")
	FName beamColorParamName = TEXT("BeamColor"); // 나이아가라 색상 이름

	UPROPERTY(EditDefaultsOnly, Category = "WorldItem|Effect")
	FName beamScaleParamName = TEXT("BeamScale"); // 나이아가라 크기 이름

	UPROPERTY(EditAnywhere, Category = "WorldItem|Effect")
	bool bShowItemMesh = true; // 필드 메시 표시 여부

	UPROPERTY(EditAnywhere, Category = "WorldItem|Effect", meta = (ClampMin = "0.0"))
	float baseLightIntensity = 1200.f; // 주변 조명 세기

	UPROPERTY(EditAnywhere, Category = "WorldItem|Effect", meta = (ClampMin = "0.0"))
	float baseLightRadius = 260.f; // 주변 조명 반경

	UPROPERTY(EditDefaultsOnly, Category = "WorldItem|Effect")
	TObjectPtr<USoundBase> pickupSound; // 획득 사운드

	UPROPERTY(EditDefaultsOnly, Category = "WorldItem|Effect")
	TObjectPtr<UNiagaraSystem> pickupEffect; // 획득 VFX

	UFUNCTION(BlueprintImplementableEvent, Category = "WorldItem")
	void OnPickedUpBP(); // 아이템 획득 시 블루프린트에서 실행되는 함수


	/* 상호작용 설정 */
	UPROPERTY(EditAnywhere, Category = "WorldItem|Interaction")
	FText interactionText = FText::FromString(TEXT("줍기")); // 상호작용 텍스트

	UPROPERTY(EditAnywhere, Category = "WorldItem|Interaction", meta = (ClampMin = "10.0"))
	float interactionRadius = 180.f; // 상호작용 감지 반경

	UPROPERTY(EditAnywhere, Category = "WorldItem|Interaction", meta = (ClampMin = "1.0"))
	float collisionRadius = 22.f; // 물리/충돌 반경

public:
	/* 변수 getter 함수 */
	FORCEINLINE FName GetItemId() const { return itemId; }
	FORCEINLINE int32 GetQuantity() const { return quantity; }
};