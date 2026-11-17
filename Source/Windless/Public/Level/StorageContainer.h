#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractableInterface.h"
#include "StorageContainer.generated.h"

class UStaticMeshComponent;
class UInventoryComponent;
class UInteractionSphereComponent;

UCLASS()
class WINDLESS_API AStorageContainer : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AStorageContainer();


	/* 상호작용 인터페이스 */
	virtual void Interact_Implementation(AActor* interactor) override; // 창고와 상호작용을 수행하는 함수
	virtual FText GetInteractionText_Implementation() override; // 상호작용 텍스트를 반환하는 함수
	virtual bool CanInteract_Implementation(AActor* interactor) override; // 현재 상호작용 가능한 상태인지 확인하는 함수

protected:
	virtual void BeginPlay() override;


	/* 컴포넌트 */
	UPROPERTY(VisibleAnywhere, Category = "Storage")
	TObjectPtr<UStaticMeshComponent> mesh; // 상자 정적 메시

	UPROPERTY(VisibleAnywhere, Category = "Storage")
	TObjectPtr<UInteractionSphereComponent> interactionSphere; // 상호작용 구형 콜라이더

	UPROPERTY(VisibleAnywhere, Category = "Storage")
	TObjectPtr<UInventoryComponent> storageInventory; // 창고 인벤토리


	/* 설정값 */
	UPROPERTY(EditAnywhere, Category = "Storage")
	FText interactionText = FText::FromString(TEXT("열기")); // 상호작용 텍스트

	UPROPERTY(EditAnywhere, Category = "Storage")
	FText containerName = FText::FromString(TEXT("창고")); // 창고 이름

public:
	/* 변수 getter 함수 */
	UFUNCTION(BlueprintPure, Category = "Storage")
	FORCEINLINE UInventoryComponent* GetStorageInventory() const { return storageInventory; }

	UFUNCTION(BlueprintPure, Category = "Storage")
	FORCEINLINE FText GetContainerName() const { return containerName; }
};