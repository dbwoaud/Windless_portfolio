#include "Components/PlayerInteractionComponent.h"
#include "Components/PlayerInputModeComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/InteractableInterface.h"
#include "UI/InteractionWidget.h"
#include "Windless/DebugMacro.h"

UPlayerInteractionComponent::UPlayerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerInteractionComponent::Initialize(APlayerController* playerController)
{
	if (AActor* owner = GetOwner())
		inputMode = owner->FindComponentByClass<UPlayerInputModeComponent>();

	if (!playerController || !promptWidgetClass)
		return;

	promptWidget = CreateWidget<UInteractionWidget>(playerController, promptWidgetClass);
	if (promptWidget)
	{
		promptWidget->AddToViewport();
		promptWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerInteractionComponent::RegisterCandidate(AActor* interactable)
{
	if (!interactable)
		return;

	candidates.AddUnique(interactable);
	Refresh();
}

void UPlayerInteractionComponent::UnregisterCandidate(AActor* interactable)
{
	candidates.Remove(interactable);
	Refresh();
}

void UPlayerInteractionComponent::Refresh()
{
	currentTarget = IsBlocked() ? nullptr : FindNearestCandidate();
	ApplyPrompt();
}

bool UPlayerInteractionComponent::TryInteract()
{
	AActor* owner = GetOwner();
	AActor* target = currentTarget;
	if (IsBlocked() || !owner || !IsValid(target) || !target->Implements<UInteractableInterface>())
		return false;

	if (!IInteractableInterface::Execute_CanInteract(target, owner))
		return false;

	UE_LOG(LogWindless, Log, TEXT("상호작용 실행 대상 = %s"), *target->GetName());
	IInteractableInterface::Execute_Interact(target, owner);
	Refresh();
	return true;
}

bool UPlayerInteractionComponent::IsBlocked() const
{
	return blockQuery.IsBound() && blockQuery.Execute();
}

AActor* UPlayerInteractionComponent::FindNearestCandidate()
{
	AActor* owner = GetOwner();
	if (!owner)
		return nullptr;

	AActor* nearest = nullptr;
	float nearestDistSq = TNumericLimits<float>::Max();
	const FVector ownerLocation = owner->GetActorLocation();

	for (int32 i = candidates.Num() - 1; i >= 0; --i)
	{
		AActor* candidate = candidates[i];
		if (!IsValid(candidate) || !candidate->Implements<UInteractableInterface>())
		{
			candidates.RemoveAt(i);
			continue;
		}

		if (!IInteractableInterface::Execute_CanInteract(candidate, owner))
			continue;

		const float distSq = FVector::DistSquared(ownerLocation, candidate->GetActorLocation());
		if (distSq < nearestDistSq)
		{
			nearestDistSq = distSq;
			nearest = candidate;
		}
	}

	return nearest;
}

void UPlayerInteractionComponent::ApplyPrompt()
{
	const bool bHasTarget = currentTarget != nullptr;

	if (promptWidget)
	{
		if (bHasTarget)
			promptWidget->SetActionText(IInteractableInterface::Execute_GetInteractionText(currentTarget));

		promptWidget->SetVisibility(bHasTarget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (inputMode)
		inputMode->SetInteractionContextActive(bHasTarget);
}