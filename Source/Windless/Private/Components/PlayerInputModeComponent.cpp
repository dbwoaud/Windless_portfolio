#include "Components/PlayerInputModeComponent.h"
#include "Input/WindlessInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Windless/DebugMacro.h"

UPlayerInputModeComponent::UPlayerInputModeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerInputModeComponent::Initialize(const UWindlessInputConfig* inConfig)
{
	config = inConfig;
	currentMode = EPlayerInputMode::EPIM_Gameplay;
	bInteractionContextActive = false;

	if (!config)
		return;

	UEnhancedInputLocalPlayerSubsystem* subsystem = GetInputSubsystem();
	if (subsystem && config->gameplayContext)
		subsystem->AddMappingContext(config->gameplayContext, GameplayPriority);
}

void UPlayerInputModeComponent::SetMode(EPlayerInputMode newMode)
{
	if (newMode == currentMode)
		return;

	UEnhancedInputLocalPlayerSubsystem* subsystem = GetInputSubsystem();
	if (!subsystem || !config)
		return;

	if (const UInputMappingContext* oldContext = GetModeContext(currentMode))
		subsystem->RemoveMappingContext(oldContext);

	currentMode = newMode;

	if (const UInputMappingContext* newContext = GetModeContext(currentMode))
		subsystem->AddMappingContext(newContext, currentMode == EPlayerInputMode::EPIM_Gameplay ? GameplayPriority : OverlayPriority);
}

void UPlayerInputModeComponent::SetInteractionContextActive(bool bActive)
{
	if (bActive == bInteractionContextActive)
		return;

	UEnhancedInputLocalPlayerSubsystem* subsystem = GetInputSubsystem();
	if (!subsystem || !config || !config->interactionContext)
		return;

	if (bActive)
		subsystem->AddMappingContext(config->interactionContext, OverlayPriority);
	else
		subsystem->RemoveMappingContext(config->interactionContext);

	bInteractionContextActive = bActive;
}

UEnhancedInputLocalPlayerSubsystem* UPlayerInputModeComponent::GetInputSubsystem() const
{
	const APawn* pawn = Cast<APawn>(GetOwner());
	const APlayerController* playerController = pawn ? Cast<APlayerController>(pawn->GetController()) : nullptr;
	return playerController ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()) : nullptr;
}

const UInputMappingContext* UPlayerInputModeComponent::GetModeContext(EPlayerInputMode mode) const
{
	if (!config)
		return nullptr;

	switch (mode)
	{
	case EPlayerInputMode::EPIM_Dialogue:  return config->dialogueContext;
	case EPlayerInputMode::EPIM_Inventory: return config->inventoryContext;
	default:                               return config->gameplayContext;
	}
}