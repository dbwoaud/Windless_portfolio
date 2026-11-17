#include "UI/MyItemContextMenuWidget.h"
#include "UI/WindlessUIStyle.h"
#include "UI/InventoryWidget.h"
#include "Characters/WindlessCharacter.h"
#include "Components/InventoryComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"

#define LOCTEXT_NAMESPACE "MyItemContextMenu"

void UMyItemContextMenuWidget::SetupContextMenu(UInventoryWidget* inOwnerWidget, UInventoryComponent* inInventory, int64 uid, EEquipSlot slot, const FVector2D& screenPos)
{
	owningInventoryWidget = inOwnerWidget;
	owningInventory = inInventory;
	targetUID = uid;
	targetSlot = slot;
	cursorScreenPos = screenPos;

	if (!owningInventory)
	{
		CloseMenu();
		return;
	}

	const FItemInstance inst = owningInventory->FindInstance(targetUID);
	FItemData data;
	if (!inst.IsValid() || !owningInventory->GetItemData(inst.itemId, data))
	{
		CloseMenu();
		return;
	}

	if (headerText)
	{
		headerText->SetText(data.itemName);
		headerText->SetColorAndOpacity(FSlateColor(FLinearColor(owningInventory->GetGradeColor(data.grade))));
	}

	BuildActions(inst, data);
	PlaceMenuAtCursor();
	SetKeyboardFocus();
}

FText UMyItemContextMenuWidget::GetBodyPartDisplayName(EBodyPart part)
{
	switch (part)
	{
		case EBodyPart::EBP_Head:     return LOCTEXT("PartHead", "머리");
		case EBodyPart::EBP_Chest:    return LOCTEXT("PartChest", "가슴");
		case EBodyPart::EBP_LeftArm:  return LOCTEXT("PartLeftArm", "왼팔");
		case EBodyPart::EBP_RightArm: return LOCTEXT("PartRightArm", "오른팔");
		case EBodyPart::EBP_LeftLeg:  return LOCTEXT("PartLeftLeg", "왼다리");
		case EBodyPart::EBP_RightLeg: return LOCTEXT("PartRightLeg", "오른다리");
		default:                      return LOCTEXT("PartNone", "-");
	}
}

void UMyItemContextMenuWidget::CloseMenu()
{
	ClearActions();
	if (owningInventoryWidget)
		owningInventoryWidget->CloseContextMenu();
	else
		RemoveFromParent();
}

void UMyItemContextMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	WindlessUI::StyleTree(this);
	SetIsFocusable(true);
}

FReply UMyItemContextMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseMenu();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UMyItemContextMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (menuBorder && menuBorder->GetCachedGeometry().IsUnderLocation(InMouseEvent.GetScreenSpacePosition()))
		return FReply::Handled();

	CloseMenu();
	return FReply::Handled();
}

void UMyItemContextMenuWidget::ClearActions()
{
	for (UContextMenuButtonWidget* button : spawnedButtons)
	{
		if (button)
		{
			button->OnContextActionClicked.Unbind();
			button->RemoveFromParent();
		}
	}
	spawnedButtons.Empty();

	if (actionBox)
		actionBox->ClearChildren();
}

void UMyItemContextMenuWidget::BuildActions(const FItemInstance& inst, const FItemData& data)
{
	ClearActions();

	if (!owningInventory)
		return;

	const bool bBroken = owningInventory->IsInstanceBroken(inst.instanceUID);
	if (data.category == EItemCategory::EIC_Equipment)
	{
		if (inst.bIsEquipped)
			AddActionButton(EItemContextAction::EICA_Unequip, LOCTEXT("Unequip", "해제"), true);
		else
			AddActionButton(EItemContextAction::EICA_Equip, LOCTEXT("Equip", "장착"), !bBroken);
	}

	if (data.category == EItemCategory::EIC_Consumable && data.effectType != EItemEffect::EIE_Ammo)
	{
		if (owningInventory->RequiresBodyPartSelection(inst.instanceUID))
		{
			TArray<EBodyPart> usableParts;
			owningInventory->GetUsableBodyParts(inst.instanceUID, usableParts);
			AddActionButton(EItemContextAction::EICA_UsePartMenu, LOCTEXT("UsePart", "부위 선택 사용"), usableParts.Num() > 0);
		}
		else
			AddActionButton(EItemContextAction::EICA_Use, LOCTEXT("Use", "사용"), true);
	}

	if (data.bStackable && inst.currentStackCount >= 2 && !inst.bIsEquipped)
		AddActionButton(EItemContextAction::EICA_Split, LOCTEXT("Split", "나누기"), true);

	if (data.bDroppable)
		AddActionButton(EItemContextAction::EICA_Drop, LOCTEXT("Drop", "버리기"), owningInventory->CanDropItem(inst.instanceUID));

	AddActionButton(EItemContextAction::EICA_Info, LOCTEXT("Info", "정보"), true);
}

void UMyItemContextMenuWidget::BuildBodyPartActions()
{
	ClearActions();

	if (!owningInventory)
		return;

	TArray<EBodyPart> usableParts;
	owningInventory->GetUsableBodyParts(targetUID, usableParts);

	for (const EBodyPart part : usableParts)
		AddActionButton(EItemContextAction::EICA_UsePart, GetBodyPartDisplayName(part), true, part);

	AddActionButton(EItemContextAction::EICA_Back, LOCTEXT("Back", "◀ 뒤로"), true);
}

void UMyItemContextMenuWidget::AddActionButton(EItemContextAction action, const FText& label, bool bEnabled, EBodyPart part)
{
	if (!actionBox || !buttonWidgetClass)
		return;

	UContextMenuButtonWidget* button = CreateWidget<UContextMenuButtonWidget>(this, buttonWidgetClass);
	if (!button)
		return;

	button->SetupButton(action, label, bEnabled, part);
	button->OnContextActionClicked.BindUObject(this, &UMyItemContextMenuWidget::HandleActionClicked);

	actionBox->AddChildToVerticalBox(button);
	spawnedButtons.Add(button);
}

void UMyItemContextMenuWidget::PlaceMenuAtCursor()
{
	if (!menuBorder)
		return;

	UCanvasPanelSlot* menuSlot = Cast<UCanvasPanelSlot>(menuBorder->Slot);
	if (!menuSlot)
		return;

	menuBorder->ForceLayoutPrepass();
	const FVector2D menuSize = menuBorder->GetDesiredSize();
	FVector2D pixelPos = FVector2D::ZeroVector;
	FVector2D viewportPos = FVector2D::ZeroVector;
	USlateBlueprintLibrary::AbsoluteToViewport(this, cursorScreenPos, pixelPos, viewportPos);

	const FVector2D viewportSize = UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).GetLocalSize();

	FVector2D finalPos = viewportPos;
	finalPos.X = FMath::Clamp(finalPos.X, screenEdgePadding.X, FMath::Max(screenEdgePadding.X, viewportSize.X - menuSize.X - screenEdgePadding.X));
	finalPos.Y = FMath::Clamp(finalPos.Y, screenEdgePadding.Y, FMath::Max(screenEdgePadding.Y, viewportSize.Y - menuSize.Y - screenEdgePadding.Y));

	menuSlot->SetAutoSize(true);
	menuSlot->SetAlignment(FVector2D::ZeroVector);
	menuSlot->SetPosition(finalPos);
}

void UMyItemContextMenuWidget::HandleActionClicked(EItemContextAction action, EBodyPart part)
{
	if (!owningInventory)
	{
		CloseMenu();
		return;
	}

	const FItemInstance inst = owningInventory->FindInstance(targetUID);

	switch (action)
	{
		case EItemContextAction::EICA_Equip:
			owningInventory->EquipItem(targetUID);
			break;

		case EItemContextAction::EICA_Unequip:
			owningInventory->UnequipSlot(inst.bIsEquipped ? inst.equipSlot : targetSlot);
			break;

		case EItemContextAction::EICA_Use:
			RequestItemCast(EBodyPart::EBP_None);
			break;

		case EItemContextAction::EICA_UsePartMenu:
			BuildBodyPartActions();
			PlaceMenuAtCursor();
			return;

		case EItemContextAction::EICA_Back:
		{
			FItemData data;
			if (owningInventory->GetItemData(inst.itemId, data))
				BuildActions(inst, data);
			PlaceMenuAtCursor();
			return;
		}

		case EItemContextAction::EICA_UsePart:
			RequestItemCast(part);
			break;

		case EItemContextAction::EICA_Split:
			if (owningInventoryWidget)
				owningInventoryWidget->OpenSplitPopup(owningInventory, targetUID);
			break;

		case EItemContextAction::EICA_Drop:
			owningInventory->DropItem(targetUID);
			break;

		case EItemContextAction::EICA_Info:
			if (owningInventoryWidget)
				owningInventoryWidget->OpenInfoPopup(owningInventory, targetUID);
			break;

		default:
			break;
	}

	CloseMenu();
}

void UMyItemContextMenuWidget::RequestItemCast(EBodyPart part)
{
	AWindlessCharacter* character = Cast<AWindlessCharacter>(owningInventory->GetOwner());
	if (!character)
		return;

	const float castTime = owningInventory->GetItemCastTime(targetUID);
	character->BeginItemCast(targetUID, part, castTime);
}
