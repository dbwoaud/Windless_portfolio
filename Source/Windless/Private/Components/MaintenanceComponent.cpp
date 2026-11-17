#include "Components/MaintenanceComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/FastTravelComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Level/SavePoint.h"
#include "UI/SavePoint/SavePointMenuWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Windless/DebugMacro.h"

UMaintenanceComponent::UMaintenanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UMaintenanceComponent::EnterMaintenance(ASavePoint* savePoint)
{
	AWindlessCharacter* character = GetOwnerCharacter();
	if (state != EMaintenanceState::Idle || !savePoint || !character || !GetOwnerController())
		return false;

	if (!menuWidgetClass)
		return false;

	currentSavePoint = savePoint;
	state = EMaintenanceState::Active;

	// 1. 정지 및 세이브포인트 바라보기
	if (UCharacterMovementComponent* movement = character->GetCharacterMovement())
		movement->StopMovementImmediately();

	FRotator lookRotation = (savePoint->GetActorLocation() - character->GetActorLocation()).Rotation();
	lookRotation.Pitch = 0.f;
	lookRotation.Roll = 0.f;
	character->SetActorRotation(lookRotation);

	// 2. 앉기
	PlayRestMontage();

	// 3. 필드 메뉴 열기
	OpenMenu();

	UE_LOG(LogWindless, Log, TEXT("[정비] 진입: %s"), *savePoint->GetSavePointId().ToString());
	onMaintenanceStateChanged.Broadcast(true);
	return true;
}

void UMaintenanceComponent::ExitMaintenance()
{
	if (state != EMaintenanceState::Active)
		return;

	state = EMaintenanceState::Exiting;
	CloseMenu();
	SetMenuInputMode(false);

	AWindlessCharacter* character = GetOwnerCharacter();
	APlayerController* controller = GetOwnerController();
	if (character && controller)
		character->DisableInput(controller);

	if (!PlayStandUpSection())
		FinishExit();
}

FName UMaintenanceComponent::GetCurrentSavePointId() const
{
	return currentSavePoint ? currentSavePoint->GetSavePointId() : NAME_None;
}

void UMaintenanceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AWindlessCharacter* character = GetOwnerCharacter())
	{
		character->onPlayerDeath.AddDynamic(this, &UMaintenanceComponent::HandleOwnerDeath);

		if (UFastTravelComponent* travel = character->FindComponentByClass<UFastTravelComponent>())
			travel->onFastTravelStateChanged.AddDynamic(this, &UMaintenanceComponent::HandleFastTravelStateChanged);
	}

}

void UMaintenanceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CloseMenu();
	Super::EndPlay(EndPlayReason);
}

void UMaintenanceComponent::OpenMenu()
{
	APlayerController* controller = GetOwnerController();
	if (!controller)
		return;

	if (!menuWidget)
	{
		menuWidget = CreateWidget<USavePointMenuWidget>(controller, menuWidgetClass);
		if (menuWidget)
			menuWidget->onCloseRequested.AddDynamic(this, &UMaintenanceComponent::HandleMenuCloseRequested);
	}

	if (!menuWidget)
		return;

	menuWidget->AddToViewport(menuZOrder);
	menuWidget->InitializeMenu(currentSavePoint ? currentSavePoint->GetDisplayName() : FText::GetEmpty(), GetCurrencyAmount());
	SetMenuInputMode(true);
}

void UMaintenanceComponent::CloseMenu()
{
	if (menuWidget && menuWidget->IsInViewport())
		menuWidget->RemoveFromParent();
}

void UMaintenanceComponent::SetMenuInputMode(bool bMenu)
{
	APlayerController* controller = GetOwnerController();
	if (!controller)
		return;

	controller->FlushPressedKeys();

	if (bMenu && menuWidget)
	{
		FInputModeUIOnly inputMode;
		inputMode.SetWidgetToFocus(menuWidget->TakeWidget());
		inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		controller->SetInputMode(inputMode);
	}
	else
	{
		controller->SetInputMode(FInputModeGameOnly());
	}
	controller->SetShowMouseCursor(bMenu);
}

void UMaintenanceComponent::HandleMenuCloseRequested()
{
	ExitMaintenance();
}

void UMaintenanceComponent::HandleFastTravelStateChanged(bool bTraveling)
{
	if (!bTraveling || state == EMaintenanceState::Idle)
		return;

	state = EMaintenanceState::Idle;
	CloseMenu();
	SetMenuInputMode(false);

	if (UAnimInstance* animInstance = GetOwnerAnimInstance())
		animInstance->Montage_Stop(0.2f, restMontage);

	currentSavePoint = nullptr;
	onMaintenanceStateChanged.Broadcast(false);
}

void UMaintenanceComponent::PlayRestMontage()
{
	UAnimInstance* animInstance = GetOwnerAnimInstance();
	if (!animInstance || !restMontage)
		return;

	animInstance->Montage_Play(restMontage);

	FOnMontageEnded endDelegate;
	endDelegate.BindUObject(this, &UMaintenanceComponent::HandleRestMontageEnded);
	animInstance->Montage_SetEndDelegate(endDelegate, restMontage);
}

bool UMaintenanceComponent::PlayStandUpSection()
{
	UAnimInstance* animInstance = GetOwnerAnimInstance();
	if (!animInstance || !restMontage || !animInstance->Montage_IsPlaying(restMontage))
		return false;

	if (restMontage->GetSectionIndex(standUpSection) == INDEX_NONE)
	{
		animInstance->Montage_Stop(0.25f, restMontage);
		return true;
	}

	animInstance->Montage_JumpToSection(standUpSection, restMontage);
	return true;
}

void UMaintenanceComponent::HandleRestMontageEnded(UAnimMontage* montage, bool bInterrupted)
{
	if (state == EMaintenanceState::Exiting)
		FinishExit();
}

void UMaintenanceComponent::FinishExit()
{
	AWindlessCharacter* character = GetOwnerCharacter();
	APlayerController* controller = GetOwnerController();
	if (character && controller)
		character->EnableInput(controller);

	state = EMaintenanceState::Idle;
	currentSavePoint = nullptr;

	UE_LOG(LogWindless, Log, TEXT("[정비] 종료"));
	onMaintenanceStateChanged.Broadcast(false);
}

void UMaintenanceComponent::HandleOwnerDeath()
{
	if (state == EMaintenanceState::Idle)
		return;

	state = EMaintenanceState::Idle;
	CloseMenu();
	SetMenuInputMode(false);

	if (UAnimInstance* animInstance = GetOwnerAnimInstance())
		animInstance->Montage_Stop(0.f, restMontage);

	currentSavePoint = nullptr;
	onMaintenanceStateChanged.Broadcast(false);
}

AWindlessCharacter* UMaintenanceComponent::GetOwnerCharacter() const
{
	return Cast<AWindlessCharacter>(GetOwner());
}

APlayerController* UMaintenanceComponent::GetOwnerController() const
{
	const AWindlessCharacter* character = GetOwnerCharacter();
	return character ? Cast<APlayerController>(character->GetController()) : nullptr;
}

UAnimInstance* UMaintenanceComponent::GetOwnerAnimInstance() const
{
	const AWindlessCharacter* character = GetOwnerCharacter();
	return (character && character->GetMesh()) ? character->GetMesh()->GetAnimInstance() : nullptr;
}

int32 UMaintenanceComponent::GetCurrencyAmount() const
{
	const UAttributeComponent* attributes = GetOwner() ? GetOwner()->FindComponentByClass<UAttributeComponent>() : nullptr;
	return attributes ? attributes->GetMoltStones() : 0;
}
