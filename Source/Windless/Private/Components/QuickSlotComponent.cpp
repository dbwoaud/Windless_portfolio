#include "Components/QuickSlotComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Components/InventoryComponent.h"
#include "Components/BodyPartComponent.h"
#include "UI/BodyPartRadialWidget.h"
#include "GameFramework/PlayerController.h"
#include "Windless/DebugMacro.h"

UQuickSlotComponent::UQuickSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UQuickSlotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (activeQuickSlot == EQuickSlot::EQS_None)
		return;

	holdTime += DeltaTime;

	if (!bRadialOpen && holdTime >= holdThreshold)
		OpenRadial(activeQuickSlot);

	if (bRadialOpen)
		UpdateSelection(DeltaTime);
}

void UQuickSlotComponent::HandleSlotPressed(EQuickSlot quickSlot)
{
	if (!character || !inventory || character->IsDead())
		return;

	if (character->IsInventoryOpen())
		return;

	if (cancelledSlots.Contains(quickSlot))
		return;

	if (quickSlot == activeQuickSlot)
		return;

	int64 uid = 0;
	FItemData data;
	if (!ValidateSlotCall(quickSlot, uid, data))
		return;

	if (!inventory->RequiresBodyPartSelection(uid))
	{
		UseWithPart(quickSlot, EBodyPart::EBP_None);
		cancelledSlots.Add(quickSlot);
		return;
	}

	if (bRadialOpen)
	{
		SwapPendingItem(quickSlot);
		return;
	}

	activeQuickSlot = quickSlot;
	holdTime = 0.f;
}

void UQuickSlotComponent::HandleSlotReleased(EQuickSlot quickSlot)
{
	if (cancelledSlots.Contains(quickSlot))
	{
		cancelledSlots.Remove(quickSlot);
		return;
	}

	if (quickSlot != activeQuickSlot)
		return;

	if (character && character->GetActionState() == EActionState::EAS_HitReaction) // 경직 중 키를 떼면 쓰지 않고 닫음(D-67)
	{
		CloseRadial();
		activeQuickSlot = EQuickSlot::EQS_None;
		holdTime = 0.f;
		return;
	}

	if (!bRadialOpen)
	{
		/* 짧은 입력 처리: 자동 대상 부위로 즉시 사용 */
		const int64 uid = inventory ? inventory->GetQuickSlotInstanceUID(quickSlot) : 0;
		const EBodyPart autoPart = (uid != 0) ? inventory->GetAutoTargetPart(uid) : EBodyPart::EBP_None;

		activeQuickSlot = EQuickSlot::EQS_None;
		holdTime = 0.f;

		UseWithPart(quickSlot, autoPart);
		return;
	}

	ConfirmUse();
}

bool UQuickSlotComponent::HandleCancelInput()
{
	if (activeQuickSlot == EQuickSlot::EQS_None)
		return false;

	cancelledSlots.Add(activeQuickSlot);

	CloseRadial();
	activeQuickSlot = EQuickSlot::EQS_None;
	holdTime = 0.f;

	UE_LOG(LogWindless, Log, TEXT("[퀵슬롯] 사용자 취소 - 아이템 미사용"));
	return true;
}

void UQuickSlotComponent::ForceCancel()
{
	if (activeQuickSlot != EQuickSlot::EQS_None)
		cancelledSlots.Add(activeQuickSlot);

	CloseRadial();
	activeQuickSlot = EQuickSlot::EQS_None;
	holdTime = 0.f;
}

void UQuickSlotComponent::NotifyItemUsed(FName itemId)
{
	FItemData data;
	if (inventory && inventory->GetItemData(itemId, data) && data.cooldown > 0.f)
		StartCooldown(itemId, data.cooldown);
}

float UQuickSlotComponent::GetCooldownRemaining(FName itemId) const
{
	const double* endTime = cooldownEndTimes.Find(itemId);
	if (!endTime || !GetWorld())
		return 0.f;

	return FMath::Max(0.f, (float)(*endTime - GetWorld()->GetTimeSeconds()));
}

float UQuickSlotComponent::GetSlotCooldownRatio(EQuickSlot quickSlot) const
{
	if (!inventory)
		return 0.f;

	const int64 uid = inventory->GetQuickSlotInstanceUID(quickSlot);
	if (uid == 0)
		return 0.f;

	const FItemInstance inst = inventory->FindInstance(uid);
	const float* duration = cooldownDurations.Find(inst.itemId);
	if (!duration || *duration <= 0.f)
		return 0.f;

	return FMath::Clamp(GetCooldownRemaining(inst.itemId) / *duration, 0.f, 1.f);
}

void UQuickSlotComponent::BeginPlay()
{
	Super::BeginPlay();

	character = Cast<AWindlessCharacter>(GetOwner());
	if (character)
	{
		inventory = character->FindComponentByClass<UInventoryComponent>();
		bodyParts = character->FindComponentByClass<UBodyPartComponent>();
	}
}

bool UQuickSlotComponent::ValidateSlotCall(EQuickSlot quickSlot, int64& outUID, FItemData& outData) const
{
	if (!inventory)
		return false;

	outUID = inventory->GetQuickSlotInstanceUID(quickSlot);
	if (outUID == 0)
		return false;

	const FItemInstance inst = inventory->FindInstance(outUID);
	if (!inventory->GetItemData(inst.itemId, outData))
		return false;

	if (GetCooldownRemaining(outData.itemId) > 0.f)
		return false;

	if (!PassesUseCondition(outData))
		return false;

	return true;
}

bool UQuickSlotComponent::PassesUseCondition(const FItemData& data) const
{
	/* conditionCode 0 = 조건 없음. 기획 확정 전까지는 확장 지점만 유지 */
	switch (data.conditionCode)
	{
		case 0:
			return true;

			// TODO(기획 확정 후): 1 = 전투 중 사용 불가, 2 = 세이브 구역 전용 등 DT 정의값 매핑
		default:
			return true;
	}
}

void UQuickSlotComponent::OpenRadial(EQuickSlot quickSlot)
{
	if (!character || !inventory || !radialWidgetClass)
		return;

	APlayerController* controller = Cast<APlayerController>(character->GetController());
	if (!controller)
		return;

	const int64 uid = inventory->GetQuickSlotInstanceUID(quickSlot);
	if (uid == 0)
		return;

	radialWidget = CreateWidget<UBodyPartRadialWidget>(controller, radialWidgetClass);
	if (!radialWidget)
		return;

	TArray<EBodyPart> usableParts;
	inventory->GetUsableBodyParts(uid, usableParts);

	radialWidget->AddToViewport(50);
	radialWidget->InitializeRadial(usableParts);
	RefreshSectorStatus();

	selectionVector = FVector2D::ZeroVector;
	bSelectionStarted = false;
	currentPart = EBodyPart::EBP_None;
	bRadialOpen = true;

	controller->bShowMouseCursor = false;
	controller->SetInputMode(FInputModeGameOnly());

	character->SetQuickSlotActive(true);
	OnHighlightChanged.Broadcast(quickSlot);
}

void UQuickSlotComponent::CloseRadial()
{
	if (radialWidget)
	{
		radialWidget->RemoveFromParent();
		radialWidget = nullptr;
	}

	bRadialOpen = false;
	bSelectionStarted = false;
	currentPart = EBodyPart::EBP_None;
	selectionVector = FVector2D::ZeroVector;

	if (character)
		character->SetQuickSlotActive(false);

	OnHighlightChanged.Broadcast(EQuickSlot::EQS_None); // 하이라이트 해제
}

void UQuickSlotComponent::SwapPendingItem(EQuickSlot quickSlot)
{
	int64 uid = 0;
	FItemData data;
	if (!ValidateSlotCall(quickSlot, uid, data))
		return;

	if (!inventory->RequiresBodyPartSelection(uid))
		return;

	activeQuickSlot = quickSlot;

	TArray<EBodyPart> usableParts;
	inventory->GetUsableBodyParts(uid, usableParts);

	if (radialWidget)
	{
		radialWidget->InitializeRadial(usableParts);
		RefreshSectorStatus();
	}

	OnHighlightChanged.Broadcast(quickSlot);
	currentPart = EBodyPart::EBP_None;
	if (radialWidget)
		radialWidget->UpdateSelection(selectionVector, ResolvePartFromVector());
}

void UQuickSlotComponent::RefreshSectorStatus()
{
	if (!radialWidget || !bodyParts)
		return;

	static const EBodyPart allParts[] =
	{
		EBodyPart::EBP_Head,    EBodyPart::EBP_Chest,
		EBodyPart::EBP_LeftArm, EBodyPart::EBP_RightArm,
		EBodyPart::EBP_LeftLeg, EBodyPart::EBP_RightLeg
	};

	for (const EBodyPart part : allParts)
	{
		radialWidget->UpdatePartStatus(
			part,
			bodyParts->GetPartHealth(part),
			bodyParts->GetPartMaxHealth(part),
			bodyParts->IsPartInjuredAny(part));
	}
}

void UQuickSlotComponent::UpdateSelection(float deltaTime)
{
	if (!character || !radialWidget)
		return;

	APlayerController* controller = Cast<APlayerController>(character->GetController());
	if (!controller)
		return;

	float deltaX = 0.f;
	float deltaY = 0.f;
	controller->GetInputMouseDelta(deltaX, deltaY);

	/* 1) 마우스 이동량 누적 */
	selectionVector.X += deltaX * quickSlotSensitivity;
	selectionVector.Y += deltaY * quickSlotSensitivity;

	/* 2) 선택 활성화 검증 */
	if (!bSelectionStarted)
	{
		const float frameDelta = FVector2D(deltaX, deltaY).Size() * quickSlotSensitivity;
		if (frameDelta < inputDetectThreshold)
		{
			currentPart = EBodyPart::EBP_None;
			return;
		}
		bSelectionStarted = true;
	}

	/* 3) 선택 벡터 클램프 */
	if (selectionVector.SizeSquared() > maxRadius * maxRadius)
		selectionVector = selectionVector.GetSafeNormal() * maxRadius;

	/* 4~5) 데드존 검증 및 방향 판정 */
	const EBodyPart hoveredPart = ResolvePartFromVector();

	/* 6) 대상 유효성 검증 */
	currentPart = radialWidget->IsPartSelectable(hoveredPart) ? hoveredPart : EBodyPart::EBP_None;

	/* 7) 실시간 UI 갱신 */
	radialWidget->UpdateSelection(selectionVector, hoveredPart);
	RefreshSectorStatus();
}

EBodyPart UQuickSlotComponent::ResolvePartFromVector() const
{
	if (selectionVector.SizeSquared() <= deadZoneRadius * deadZoneRadius)
		return EBodyPart::EBP_Chest;

	float angle = FMath::RadiansToDegrees(FMath::Atan2(selectionVector.Y, selectionVector.X));
	if (angle < 0.f)
		angle += 360.f;

	if (angle >= 54.f && angle < 126.f)
		return EBodyPart::EBP_Head;

	if (angle >= 126.f && angle < 198.f)
		return EBodyPart::EBP_LeftArm;

	if (angle >= 198.f && angle < 270.f)
		return EBodyPart::EBP_LeftLeg;

	if (angle >= 270.f && angle < 342.f)
		return EBodyPart::EBP_RightLeg;

	return EBodyPart::EBP_RightArm;
}

void UQuickSlotComponent::ConfirmUse()
{
	const EQuickSlot quickSlot = activeQuickSlot;
	const EBodyPart part = currentPart;

	CloseRadial();
	activeQuickSlot = EQuickSlot::EQS_None;
	holdTime = 0.f;

	if (part == EBodyPart::EBP_None)
	{
		UE_LOG(LogWindless, Log, TEXT("[퀵슬롯] 대상 부위 미지정 - 아이템 미사용 종료"));
		return;
	}

	UseWithPart(quickSlot, part);
}

void UQuickSlotComponent::UseWithPart(EQuickSlot quickSlot, EBodyPart part)
{
	int64 uid = 0;
	FItemData data;
	if (!ValidateSlotCall(quickSlot, uid, data))
		return;

	if (!character)
		return;

	const float castTime = inventory->GetItemCastTime(uid);
	if (!character->BeginItemCast(uid, part, castTime))
		return;

	UE_LOG(LogWindless, Log, TEXT("[퀵슬롯] 사용 개시 - 슬롯=%d, 부위=%d, 시전=%.2f초"), (int32)quickSlot, (int32)part, castTime);
}

void UQuickSlotComponent::StartCooldown(FName itemId, float cooldown)
{
	if (!GetWorld() || cooldown <= 0.f)
		return;

	cooldownEndTimes.Add(itemId, GetWorld()->GetTimeSeconds() + cooldown);
	cooldownDurations.Add(itemId, cooldown);
}
