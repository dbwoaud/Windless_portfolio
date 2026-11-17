#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class WINDLESS_API IInteractableInterface
{
	GENERATED_BODY()

public:
	/* 상호작용 */
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	void Interact(AActor* interactor); // 실제 오브젝트와 상호작용을 수행하는 함수

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	FText GetInteractionText(); // 상호작용 텍스트를 반환하는 함수

	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool CanInteract(AActor* interactor); // 현재 상호작용 가능한 상태인지 확인하는 함수
};