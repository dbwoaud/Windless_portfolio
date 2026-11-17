#include "Components/BowComponent.h"
#include "Components/CombatComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/TargetingComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Characters/BodyPartTypes.h"
#include "Items/Weapons/Bow.h"
#include "Items/Weapons/Projectiles/ArrowProjectile.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Windless/DebugMacro.h"

UBowComponent::UBowComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBowComponent::HandleBowPressed(ABow* bow, EAttackHand inputHand)
{
	if (!bow || !character)
		return;

	if (bDrawing)
	{
		if (inputHand != drawButton)
			CancelDraw(TEXT("취소 입력"));
		return;
	}

	if (!CanStartDraw())
		return;

	if (GetAmmoCount(bow) <= 0)
	{
		UE_LOG(LogWindless, Log, TEXT("[활] 화살 없음 - 발동 불가"));
		return;
	}

	UAnimInstance* anim = GetAnimInstance();
	if (!anim || !bowShotMontage)
		return;

	anim->Montage_Play(bowShotMontage);
	anim->Montage_JumpToSection(drawSectionName, bowShotMontage);

	bDrawing = true;
	drawButton = inputHand;
	drawingBow = bow;
	drawStartTime = GetWorld()->GetTimeSeconds();

	character->EnterAimState();
	SetCrosshairVisible(true);

	UE_LOG(LogWindless, Log, TEXT("[활] 당기기 시작 (화살 %d개)"), GetAmmoCount(bow));
}

void UBowComponent::HandleBowReleased(EAttackHand inputHand)
{
	if (!bDrawing || inputHand != drawButton || !drawingBow || !character)
		return;

	const float heldTime = GetWorld()->GetTimeSeconds() - drawStartTime;
	if (heldTime < drawingBow->GetMinDrawTime())
	{
		CancelDraw(TEXT("당김 부족"));
		return;
	}

	if (!drawingBow->GetArrowClass())
	{
		UE_LOG(LogWindless, Warning, TEXT("[활] 화살 클래스 미지정 - BP_Bow 설정 확인"));
		CancelDraw(TEXT("화살 클래스 미지정"));
		return;
	}

	const float staminaCost = drawingBow->GetShotStaminaCost();
	if (character->GetCurrentStamina() < staminaCost)
	{
		CancelDraw(TEXT("스태미나 부족"));
		return;
	}

	if (!ConsumeAmmo(drawingBow))
	{
		CancelDraw(TEXT("화살 없음"));
		return;
	}

	character->ConsumeStamina(staminaCost);
	FireArrow(GetChargeRatio());

	EndDraw();
	character->BeginBowShotRecovery();

	if (UAnimInstance* anim = GetAnimInstance())
		anim->Montage_JumpToSection(fireSectionName, bowShotMontage);
}

void UBowComponent::CancelDraw(const TCHAR* reason)
{
	if (!bDrawing)
		return;

	EndDraw();

	UAnimInstance* anim = GetAnimInstance();
	if (anim && bowShotMontage && anim->Montage_IsActive(bowShotMontage))
		anim->Montage_Stop(cancelBlendOutTime, bowShotMontage);

	UE_LOG(LogWindless, Log, TEXT("[활] 당기기 취소 (%s) - 화살·스태미나 미소모"), reason);
}

void UBowComponent::OnWeaponRemoved(const AWeapon* removedWeapon)
{
	if (bDrawing && removedWeapon && removedWeapon == drawingBow)
		CancelDraw(TEXT("무기 교체"));
}

float UBowComponent::GetChargeRatio() const
{
	if (!bDrawing || !drawingBow)
		return 0.f;

	return drawingBow->GetChargeRatio(GetWorld()->GetTimeSeconds() - drawStartTime);
}

void UBowComponent::BeginPlay()
{
	Super::BeginPlay();
	character = Cast<AWindlessCharacter>(GetOwner());
}

bool UBowComponent::CanStartDraw() const
{
	if (character->GetActionState() != EActionState::EAS_Unoccupied)
		return false;

	if (character->GetCharacterState() == ECharacterState::ECS_Unequipped)
		return false;

	if (character->IsMounted() || character->IsQuickSlotActive() || character->CheckActionDelay())
		return false;

	if (const UCharacterMovementComponent* movement = character->GetCharacterMovement(); movement && movement->IsFalling())
		return false;

	if (const UBodyPartComponent* bodyParts = character->GetBodyParts())
	{
		if (bodyParts->IsPartDisabled(EBodyPart::EBP_LeftArm) || bodyParts->IsPartDisabled(EBodyPart::EBP_RightArm))
		{
			UE_LOG(LogWindless, Log, TEXT("[활] 팔 파괴 - 당기기 불가"));
			return false;
		}
	}

	return true;
}

int32 UBowComponent::GetAmmoCount(const ABow* bow) const
{
	const UInventoryComponent* inventory = character ? character->GetInventory() : nullptr;
	return (inventory && bow) ? inventory->GetItemTotalCount(bow->GetAmmoItemId()) : 0;
}

void UBowComponent::EndDraw()
{
	bDrawing = false;
	drawingBow = nullptr;
	SetCrosshairVisible(false);

	if (character)
		character->ExitAimState();
}

FVector UBowComponent::ComputeAimPoint() const
{
	APlayerController* controller = Cast<APlayerController>(character->GetController());
	if (!controller)
		return character->GetActorLocation() + character->GetActorForwardVector() * aimTraceDistance;

	FVector viewLocation;
	FRotator viewRotation;
	controller->GetPlayerViewPoint(viewLocation, viewRotation);
	const FVector traceEnd = viewLocation + viewRotation.Vector() * aimTraceDistance;

	FCollisionQueryParams params(SCENE_QUERY_STAT(BowAim), false, character);
	if (drawingBow)
		params.AddIgnoredActor(drawingBow);

	FHitResult hit;
	if (GetWorld()->LineTraceSingleByChannel(hit, viewLocation, traceEnd, aimTraceChannel, params))
		return hit.ImpactPoint;

	return traceEnd;
}

void UBowComponent::FireArrow(float chargeRatio)
{
	UWorld* world = GetWorld();
	if (!world || !drawingBow)
		return;

	const FVector spawnLocation = drawingBow->GetArrowSpawnLocation();
	const FVector direction = (ComputeAimPoint() - spawnLocation).GetSafeNormal();

	FActorSpawnParameters spawnParams;
	spawnParams.Owner = character;
	spawnParams.Instigator = character;
	spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AArrowProjectile* arrow = world->SpawnActor<AArrowProjectile>(drawingBow->GetArrowClass(), spawnLocation, direction.Rotation(), spawnParams);
	if (!arrow)
		return;

	const FArrowShotParams shot = drawingBow->BuildShotParams(chargeRatio);
	arrow->Launch(direction, shot, drawingBow);

	UE_LOG(LogWindless, Log, TEXT("[활] 발사 차징=%.0f%% 대미지=%.1f 속도=%.0f (남은 화살 %d개)"),
		shot.chargeRatio * 100.f, shot.damage, shot.speed, GetAmmoCount(drawingBow));
}

bool UBowComponent::ConsumeAmmo(const ABow* bow)
{
	UInventoryComponent* inventory = character ? character->GetInventory() : nullptr;
	return inventory && bow && inventory->RemoveItemById(bow->GetAmmoItemId(), 1) == 1;
}

UAnimInstance* UBowComponent::GetAnimInstance() const
{
	return (character && character->GetMesh()) ? character->GetMesh()->GetAnimInstance() : nullptr;
}

void UBowComponent::SetCrosshairVisible(bool bVisible)
{
	if (!crosshairWidget && bVisible && crosshairWidgetClass && character)
	{
		if (APlayerController* controller = Cast<APlayerController>(character->GetController()))
		{
			crosshairWidget = CreateWidget<UUserWidget>(controller, crosshairWidgetClass);
			if (crosshairWidget)
				crosshairWidget->AddToViewport();
		}
	}

	if (crosshairWidget)
		crosshairWidget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
