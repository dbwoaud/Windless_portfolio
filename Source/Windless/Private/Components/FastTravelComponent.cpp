#include "Components/FastTravelComponent.h"
#include "Components/MaintenanceComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Level/SavePointSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Windless/DebugMacro.h"

UFastTravelComponent::UFastTravelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFastTravelComponent::TravelTo(FName destinationId)
{
	if (GetBlockReason(destinationId) != EFastTravelBlockReason::EFTB_None)
		return false;

	UWorld* world = GetWorld();
	USavePointSubsystem* savePoints = world ? world->GetSubsystem<USavePointSubsystem>() : nullptr;
	if (!savePoints || !savePoints->FindActivated(destinationId, pendingDestination))
		return false;

	bTraveling = true;
	onFastTravelStateChanged.Broadcast(true);

	// 페이드 연출
	if (APlayerController* controller = GetOwnerController())
	{
		if (controller->PlayerCameraManager)
			controller->PlayerCameraManager->StartCameraFade(0.f, 1.f, fadeOutDuration, fadeColor, false, true);

		if (AWindlessCharacter* character = GetOwnerCharacter())
			character->DisableInput(controller);
	}

	world->GetTimerManager().SetTimer(travelTimer, this, &UFastTravelComponent::PerformTeleport, FMath::Max(0.01f, fadeOutDuration), false);
	return true;
}

EFastTravelBlockReason UFastTravelComponent::GetBlockReason(FName destinationId) const
{
	if (bTraveling)
		return EFastTravelBlockReason::EFTB_Busy;

	AWindlessCharacter* character = GetOwnerCharacter();
	if (!character)
		return EFastTravelBlockReason::EFTB_NotAtPoint;

	if (character->IsInCombatState())
		return EFastTravelBlockReason::EFTB_InCombat;

	UMaintenanceComponent* maintenance = character->FindComponentByClass<UMaintenanceComponent>();
	if (!maintenance || !maintenance->IsInMaintenance())
		return EFastTravelBlockReason::EFTB_NotAtPoint;

	if (destinationId.IsNone())
		return EFastTravelBlockReason::EFTB_NotActive;

	if (destinationId == maintenance->GetCurrentSavePointId())
		return EFastTravelBlockReason::EFTB_SamePoint;

	const UWorld* world = GetWorld();
	const USavePointSubsystem* savePoints = world ? world->GetSubsystem<USavePointSubsystem>() : nullptr;
	if (!savePoints || !savePoints->IsActivated(destinationId))
		return EFastTravelBlockReason::EFTB_NotActive;

	return EFastTravelBlockReason::EFTB_None;
}

AWindlessCharacter* UFastTravelComponent::GetOwnerCharacter() const
{
	return Cast<AWindlessCharacter>(GetOwner());
}

APlayerController* UFastTravelComponent::GetOwnerController() const
{
	const AWindlessCharacter* character = GetOwnerCharacter();
	return character ? Cast<APlayerController>(character->GetController()) : nullptr;
}

void UFastTravelComponent::PerformTeleport()
{
	AWindlessCharacter* character = GetOwnerCharacter();
	UWorld* world = GetWorld();
	if (!character || !world)
		return;

	// 좌표 이동
	if (UCharacterMovementComponent* movement = character->GetCharacterMovement())
		movement->StopMovementImmediately();

	character->SetActorLocationAndRotation(
		pendingDestination.respawnLocation,
		pendingDestination.respawnRotation,
		false, nullptr, ETeleportType::TeleportPhysics);

	if (APlayerController* controller = GetOwnerController())
		controller->SetControlRotation(pendingDestination.respawnRotation);

	//페이드 인 연출
	world->GetTimerManager().SetTimer(travelTimer, this, &UFastTravelComponent::FinishTravel, FMath::Max(0.01f, loadingDuration), false);
}

void UFastTravelComponent::FinishTravel()
{
	if (APlayerController* controller = GetOwnerController())
	{
		if (controller->PlayerCameraManager)
			controller->PlayerCameraManager->StartCameraFade(1.f, 0.f, fadeInDuration, fadeColor, false, false);

		if (AWindlessCharacter* character = GetOwnerCharacter())
			character->EnableInput(controller);
	}

	bTraveling = false;
	pendingDestination = FSavePointRecord();

	UE_LOG(LogWindless, Log, TEXT("[빠른 이동] 완료"));
	onFastTravelStateChanged.Broadcast(false);
}
