#include "UI/InventoryWidget.h"
#include "UI/ItemSlotWidget.h"
#include "UI/ItemDragDrop.h"
#include "UI/EquipmentSlotWidget.h"
#include "UI/MyItemContextMenuWidget.h"
#include "UI/ItemSplitPopupWidget.h"
#include "UI/ItemInfoPopupWidget.h" 
#include "UI/ItemHoverInfoWidget.h"
#include "UI/QuickSlot/ConsumableSlotWidget.h"
#include "UI/ItemGridWidget.h"
#include "UI/BodyPart/BodyPartSlotWidget.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/InventoryComponent.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/BodyPartComponent.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/Pawn.h"
#include "UI/WindlessUIStyle.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/AttributeComponent.h"
#include "Characters/WindlessCharacter.h"
#include "Components/TextBlock.h"

/** 화면 틀: 왼쪽 장비 | 가운데 가방 칸과 설명 | 오른쪽 상자 자리. 왼쪽 둘이 화면 너비의 2/3다(1920 기준 픽셀) */
namespace BagLayout
{
	const float Side = 64.f; // 화면 양옆 여백
	const float Top = 122.f; // 머리말 줄 아래
	const float Bottom = 72.f; // 키 안내 줄 위
	const float Gap = 8.f; // 틀 사이 절반 간격
	const float EquipEnd = 0.30f; // 장비 틀이 끝나는 곳(화면 너비 비율)
	const float BagEnd = 0.66f; // 가방 틀이 끝나는 곳
	const float CellSize = 92.f; // 가방 칸 한 칸
	const float GridTop = Top + 46.f;
	const float DetailTop = GridTop + CellSize * 6.f + 34.f;
	const float DollScale = 1.12f;
}

/** 가방 화면에 세이브포인트 UI와 같은 스타일을 입힌다. 위젯 블루프린트의 이름으로 찾는다. */
static void StyleInventory(UUserWidget* widget)
{
	using namespace BagLayout;

	UWidgetTree* tree = widget->WidgetTree;
	if (!tree || tree->FindWidget(TEXT("WI_Header")))
		return;

	WindlessUI::StyleTree(widget);

	for (const TCHAR* name : { TEXT("Image"), TEXT("Image1") }) // 가방 격자와 보관함 격자의 바탕
	{
		if (UImage* panel = Cast<UImage>(tree->FindWidget(name)))
		{
			panel->SetBrush(WindlessUI::Box(FLinearColor(0.f, 0.f, 0.f, 0.35f), FLinearColor::Transparent)); // 테두리는 칸이 그린다
			panel->SetColorAndOpacity(FLinearColor::White);
		}
	}
	WindlessUI::BodyText(Cast<UTextBlock>(tree->FindWidget(TEXT("TextBlock_139"))));
	if (UImage* silhouette = Cast<UImage>(tree->FindWidget(TEXT("CharacterSilhouette")))) // 검은 덩어리 대신 옅은 금색 윤곽
		silhouette->SetBrushTintColor(FSlateColor(WindlessUI::Alpha(WindlessUI::GoldDim, 0.2f)));

	// 세이브포인트 보관함 쪽(Dimmer가 꺼진 것)은 세이브포인트 메뉴가 머리말과 틀을 이미 그린다
	UImage* dimmer = Cast<UImage>(tree->FindWidget(TEXT("Dimmer")));
	UCanvasPanel* root = Cast<UCanvasPanel>(tree->RootWidget);
	if (!root || !dimmer || dimmer->GetVisibility() == ESlateVisibility::Collapsed)
		return;

	auto place = [root](UWidget* child, const FAnchors& anchors, const FMargin& offsets, const FVector2D& alignment, bool bAutoSize, int32 zOrder = 0)
	{
		UCanvasPanelSlot* slot = root->AddChildToCanvas(child); // 다른 곳에 있던 위젯은 거기서 빠져 이리로 온다
		slot->SetAnchors(anchors);
		slot->SetOffsets(offsets);
		slot->SetAlignment(alignment);
		slot->SetAutoSize(bAutoSize);
		slot->SetZOrder(zOrder);
	};
	auto image = [tree](const FSlateBrush& brush)
	{
		UImage* result = tree->ConstructWidget<UImage>();
		result->SetBrush(brush);
		result->SetVisibility(ESlateVisibility::HitTestInvisible);
		return result;
	};
	// 틀 하나: 어두운 바탕 + 얇은 금색 테두리 + 왼쪽 위 작은 제목 + 제목 아래 금색 줄
	auto frame = [&](float from, float to, float left, float right, const TCHAR* title)
	{
		place(image(WindlessUI::Box(FLinearColor(0.f, 0.f, 0.f, 0.38f), WindlessUI::Alpha(WindlessUI::GoldDim, 0.35f))),
			FAnchors(from, 0.f, to, 1.f), FMargin(left, Top, right, Bottom), FVector2D::ZeroVector, false, -1);
		place(WindlessUI::Caption(widget, FText::FromString(title)),
			FAnchors(from, 0.f), FMargin(left + 20.f, Top + 14.f, 0.f, 0.f), FVector2D::ZeroVector, true);
		place(image(WindlessUI::Material(TEXT("MI_UI_DividerGold"))),
			FAnchors(from, 0.f, to, 0.f), FMargin(left + 20.f, Top + 36.f, right + 20.f, 1.f), FVector2D::ZeroVector, false);
	};

	if (UCanvasPanelSlot* dimmerSlot = Cast<UCanvasPanelSlot>(dimmer->Slot))
		dimmerSlot->SetZOrder(-3);
	place(image(WindlessUI::Material(TEXT("MI_UI_PageBackdrop"))), FAnchors(0.f, 0.f, 1.f, 1.f), FMargin(0.f), FVector2D::ZeroVector, false, -2);

	UWidget* header = WindlessUI::Header(widget, FText::FromString(TEXT("소지품")), FText::FromString(TEXT("가방")));
	header->Rename(TEXT("WI_Header"));
	place(header, FAnchors(0.f, 0.f), FMargin(Side, 36.f, 0.f, 0.f), FVector2D::ZeroVector, true);
	place(image(WindlessUI::Material(TEXT("MI_UI_DividerGold"))), FAnchors(0.f, 0.f, 1.f, 0.f), FMargin(Side, 106.f, Side, 1.f), FVector2D::ZeroVector, false);

	TArray<TPair<FText, FText>> hints;
	hints.Emplace(FText::FromString(TEXT("Tab")), FText::FromString(TEXT("닫기")));
	hints.Emplace(FText::FromString(TEXT("F")), FText::FromString(TEXT("장착 / 퀵슬롯")));
	hints.Emplace(FText::FromString(TEXT("R")), FText::FromString(TEXT("돌리기")));
	hints.Emplace(FText::FromString(TEXT("Q")), FText::FromString(TEXT("버리기")));
	hints.Emplace(FText::FromString(TEXT("우클릭")), FText::FromString(TEXT("메뉴")));
	place(WindlessUI::KeyHints(widget, hints), FAnchors(0.f, 1.f), FMargin(Side, -28.f, 0.f, 0.f), FVector2D(0.f, 1.f), true);

	UTextBlock* mark = tree->ConstructWidget<UTextBlock>();
	mark->SetText(FText::FromString(TEXT("Windless")));
	WindlessUI::Text(mark, 9.f, 340, WindlessUI::Alpha(WindlessUI::GoldDim, 0.45f));
	place(mark, FAnchors(1.f, 1.f), FMargin(-Side, -32.f, 0.f, 0.f), FVector2D(1.f, 1.f), true);

	// ---- 틀 셋
	frame(0.f, EquipEnd, Side, Gap, TEXT("장비"));
	frame(EquipEnd, BagEnd, Gap, Gap, TEXT("가방 칸"));
	frame(BagEnd, 1.f, Gap, Side, TEXT("상자"));

	// ---- 장비: 인형을 틀 가운데에 키워서 놓는다. 인형 캔버스 위쪽 빈 80은 없앤다
	if (UCanvasPanel* doll = Cast<UCanvasPanel>(tree->FindWidget(TEXT("DollCanvas"))))
	{
		for (UPanelSlot* panelSlot : doll->GetSlots())
		{
			if (UCanvasPanelSlot* dollSlot = Cast<UCanvasPanelSlot>(panelSlot))
				dollSlot->SetPosition(dollSlot->GetPosition() - FVector2D(0.f, 80.f));
		}
	}
	if (USizeBox* dollBox = Cast<USizeBox>(tree->FindWidget(TEXT("SizeBox_Doll"))))
	{
		dollBox->SetHeightOverride(630.f);
		dollBox->SetRenderTransformPivot(FVector2D(0.5f, 0.f));
		dollBox->SetRenderScale(FVector2D(DollScale, DollScale));
		// 장비 틀의 가운데 = (Side + EquipEnd*W - Gap) / 2
		place(dollBox, FAnchors(EquipEnd * 0.5f, 0.f), FMargin((Side - Gap) * 0.5f, Top + 84.f, 0.f, 0.f), FVector2D(0.5f, 0.f), true);
	}

	// 장비 아래 상태 줄: 값은 열 때마다 UInventoryWidget::NativeConstruct가 채운다
	UHorizontalBox* status = tree->ConstructWidget<UHorizontalBox>();
	const TCHAR* labels[] = { TEXT("체력"), TEXT("스태미나"), TEXT("혼돈"), TEXT("탈피석") };
	for (int32 index = 0; index < UE_ARRAY_COUNT(labels); ++index)
	{
		UVerticalBox* item = tree->ConstructWidget<UVerticalBox>();
		item->AddChildToVerticalBox(WindlessUI::Caption(widget, FText::FromString(labels[index])));
		UTextBlock* value = tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("WI_Stat%d"), index));
		value->SetText(FText::FromString(TEXT("-")));
		WindlessUI::Text(value, 17.f, 40, WindlessUI::Gold); // 세이브포인트 메뉴의 값 글자
		item->AddChildToVerticalBox(value);
		UHorizontalBoxSlot* itemSlot = status->AddChildToHorizontalBox(item);
		itemSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	place(image(WindlessUI::Material(TEXT("MI_UI_DividerGold"))), FAnchors(0.f, 1.f, EquipEnd, 1.f), FMargin(Side + 20.f, -(Bottom + 84.f), Gap + 20.f, 1.f), FVector2D::ZeroVector, false);
	place(status, FAnchors(0.f, 1.f, EquipEnd, 1.f), FMargin(Side + 24.f, -(Bottom + 70.f), Gap + 24.f, 54.f), FVector2D::ZeroVector, false);

	// ---- 가방 칸: 칸을 키워 틀을 채운다
	if (UItemGridWidget* grid = Cast<UItemGridWidget>(tree->FindWidget(TEXT("playerGrid"))))
		grid->SetCellSize(CellSize);
	const float bagCenter = (EquipEnd + BagEnd) * 0.5f;
	if (USizeBox* gridPanel = Cast<USizeBox>(tree->FindWidget(TEXT("GridPanel"))))
	{
		gridPanel->ClearWidthOverride();
		gridPanel->ClearHeightOverride();
		gridPanel->ClearMinDesiredWidth();
		gridPanel->ClearMinDesiredHeight();
		place(gridPanel, FAnchors(bagCenter, 0.f), FMargin(0.f, GridTop, 0.f, 0.f), FVector2D(0.5f, 0.f), true);
	}

	// 가방 칸 아래 설명: 왼쪽 아이콘 틀, 오른쪽에 마우스를 올린 아이템의 정보(ShowHoverInfo가 꽂는다)
	place(WindlessUI::Caption(widget, FText::FromString(TEXT("설명"))), FAnchors(EquipEnd, 0.f), FMargin(Gap + 20.f, DetailTop - 22.f, 0.f, 0.f), FVector2D::ZeroVector, true);
	place(image(WindlessUI::Material(TEXT("MI_UI_DividerGold"))), FAnchors(EquipEnd, 0.f, BagEnd, 0.f), FMargin(Gap + 20.f, DetailTop - 2.f, Gap + 20.f, 1.f), FVector2D::ZeroVector, false);
	place(image(WindlessUI::Tile(0.04f, 0.45f)), FAnchors(EquipEnd, 0.f), FMargin(Gap + 24.f, DetailTop + 12.f, 84.f, 84.f), FVector2D::ZeroVector, false);
	UImage* detailIcon = tree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("WI_DetailIcon"));
	detailIcon->SetVisibility(ESlateVisibility::Collapsed);
	place(detailIcon, FAnchors(EquipEnd, 0.f), FMargin(Gap + 30.f, DetailTop + 18.f, 72.f, 72.f), FVector2D::ZeroVector, false);
	UBorder* detail = tree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("WI_Detail"));
	detail->SetBrushColor(FLinearColor::Transparent);
	detail->SetPadding(FMargin(0.f));
	detail->SetVisibility(ESlateVisibility::HitTestInvisible);
	UTextBlock* detailHint = tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WI_DetailHint"));
	detailHint->SetText(FText::FromString(TEXT("아이템에 마우스를 올리면 여기에 정보가 나옵니다.")));
	WindlessUI::Text(detailHint, 11.f, 0, WindlessUI::Alpha(WindlessUI::GoldDim, 0.6f));
	place(detailHint, FAnchors(EquipEnd, 0.f), FMargin(Gap + 128.f, DetailTop + 14.f, 0.f, 0.f), FVector2D::ZeroVector, true);
	place(detail, FAnchors(EquipEnd, 0.f, BagEnd, 1.f), FMargin(Gap + 128.f, DetailTop + 8.f, Gap + 20.f, Bottom + 12.f), FVector2D::ZeroVector, false);

	// ---- 상자 자리: 나중에 상자 파밍 칸이 들어온다. 지금은 자리만 표시한다
	UTextBlock* chestHint = tree->ConstructWidget<UTextBlock>();
	chestHint->SetText(FText::FromString(TEXT("열어 둔 상자가 없습니다.")));
	WindlessUI::Text(chestHint, 11.f, 0, WindlessUI::Alpha(WindlessUI::GoldDim, 0.6f));
	place(chestHint, FAnchors(BagEnd, 0.f), FMargin(Gap + 20.f, Top + 52.f, 0.f, 0.f), FVector2D::ZeroVector, true);

	// 옛 틀(가로 상자)에는 보관함만 남는다. 보관함이 켜지면 상자 틀 안에 나오게 옮겨 둔다
	if (UWidget* old = tree->FindWidget(TEXT("HorizontalBox")))
	{
		if (UCanvasPanelSlot* oldSlot = Cast<UCanvasPanelSlot>(old->Slot))
		{
			oldSlot->SetAnchors(FAnchors(BagEnd, 0.f, 1.f, 1.f));
			oldSlot->SetOffsets(FMargin(Gap, Top + 46.f, Side, Bottom + 12.f));
		}
	}
	for (const TCHAR* name : { TEXT("EquipOverlay"), TEXT("InventoryOverlay") }) // 내용이 빠진 옛 칸
	{
		if (UWidget* empty = tree->FindWidget(name))
			empty->SetVisibility(ESlateVisibility::Collapsed);
	}
}


void UInventoryWidget::OpenContextMenu(UInventoryComponent* inventory, int64 uid, EEquipSlot slot, const FVector2D& screenPos)
{
	if (!inventory || !contextMenuClass || uid == 0)
		return;

	CloseAllPopups();

	popupInventory = inventory;

	contextMenu = CreateWidget<UMyItemContextMenuWidget>(GetOwningPlayer(), contextMenuClass);
	if (!contextMenu)
		return;

	contextMenu->AddToViewport(popupZOrder);
	contextMenu->SetupContextMenu(this, inventory, uid, slot, screenPos);
}


void UInventoryWidget::CloseContextMenu()
{
	if (!contextMenu)
		return;

	contextMenu->RemoveFromParent();
	contextMenu = nullptr;

	SetKeyboardFocus();
}

void UInventoryWidget::OpenSplitPopup(UInventoryComponent* inventory, int64 uid, int32 targetX, int32 targetY, bool bRotated)
{
	if (!inventory || !splitPopupClass || uid == 0)
		return;

	CloseSplitPopup();

	popupInventory = inventory;

	splitPopup = CreateWidget<UItemSplitPopupWidget>(GetOwningPlayer(), splitPopupClass);
	if (!splitPopup)
		return;

	splitPopup->AddToViewport(popupZOrder + 1);
	splitPopup->SetupPopup(this, inventory, uid, targetX, targetY, bRotated);
}

void UInventoryWidget::CloseSplitPopup()
{
	if (!splitPopup)
		return;

	splitPopup->RemoveFromParent();
	splitPopup = nullptr;

	SetKeyboardFocus();
}

void UInventoryWidget::OpenInfoPopup(UInventoryComponent* inventory, int64 uid)
{
	if (!inventory || !infoPopupClass || uid == 0)
		return;

	CloseInfoPopup();

	popupInventory = inventory;

	infoPopup = CreateWidget<UItemInfoPopupWidget>(GetOwningPlayer(), infoPopupClass);
	if (!infoPopup)
		return;

	infoPopup->AddToViewport(popupZOrder + 1);
	infoPopup->SetupInfo(this, inventory, uid);
}

void UInventoryWidget::CloseInfoPopup()
{
	if (!infoPopup)
		return;

	infoPopup->RemoveFromParent();
	infoPopup = nullptr;

	SetKeyboardFocus();
}

void UInventoryWidget::CloseAllPopups()
{
	CancelHoverInfo();
	CloseInfoPopup();
	CloseSplitPopup();
	CloseContextMenu();
	popupInventory = nullptr;
}

bool UInventoryWidget::IsAnyPopupOpen() const
{
	return contextMenu != nullptr || splitPopup != nullptr || infoPopup != nullptr;
}

void UInventoryWidget::SetHoveredItem(UInventoryComponent* inventory, int64 uid)
{
	hoveredInventory = inventory;
	hoveredUID = uid;
}

void UInventoryWidget::BeginHoverInfo(UInventoryComponent* inventory, int64 uid)
{
	CancelHoverInfo();

	if (uid == 0 || !inventory || !hoverInfoClass || IsAnyPopupOpen())
		return;

	pendingHoverInventory = inventory;
	pendingHoverUID = uid;

	if (hoverInfoDelay <= 0.f)
	{
		ShowHoverInfo();
		return;
	}

	UWorld* world = GetWorld();
	if (!world)
		return;

	world->GetTimerManager().SetTimer(hoverInfoTimer, this, &UInventoryWidget::ShowHoverInfo, hoverInfoDelay, false);
}

void UInventoryWidget::CancelHoverInfo()
{
	if (UWorld* world = GetWorld())
		world->GetTimerManager().ClearTimer(hoverInfoTimer);

	pendingHoverInventory = nullptr;
	pendingHoverUID = 0;
	CloseHoverInfo();
}

void UInventoryWidget::SetStorageInventory(UInventoryComponent* storage)
{
	if (storageInventory && storageInventory != storage)
		storageInventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);

	storageInventory = storage;

	if (storagePanel)
		storagePanel->SetVisibility(storage ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (storageGrid)
		storageGrid->InitializeGrid(storage, this);

	if (storage)
	{
		storage->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
		storage->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::HandleInventoryChanged);
	}
}

void UInventoryWidget::RefreshAllGrids()
{
	if (playerGrid)
		playerGrid->RefreshGrid();

	if (storageGrid && storageInventory)
		storageGrid->RefreshGrid();
}

float UInventoryWidget::GetCellSize() const
{
	return playerGrid ? playerGrid->GetCellSize() : 48.f;
}

void UInventoryWidget::HideAllPlacementHighlights()
{
	if (playerGrid)
		playerGrid->HidePlacementHighlight();

	if (storageGrid)
		storageGrid->HidePlacementHighlight();
}

bool UInventoryWidget::RequestQuickTransfer(UInventoryComponent* source, int64 instanceUID)
{
	UInventoryComponent* target = GetQuickTransferTarget(source);
	if (!target)
		return false;

	CancelHoverInfo();
	return source->QuickTransferTo(target, instanceUID);
}

UInventoryComponent* UInventoryWidget::GetQuickTransferTarget(UInventoryComponent* source) const
{
	if (!source || !storageInventory)
		return nullptr;

	if (source == owningInventory)
		return storageInventory;

	if (source == storageInventory)
		return owningInventory;

	return nullptr;
}

void UInventoryWidget::BeginConsumableDrag(UItemDragDrop* op)
{
	if (!op || !owningInventory)
		return;

	if (!owningInventory->RequiresBodyPartSelection(op->instanceUID)) // 부위 지정 아이템만 대상
		return;

	op->OnDrop.AddDynamic(this, &UInventoryWidget::HandleConsumableDragEnded);
	op->OnDragCancelled.AddDynamic(this, &UInventoryWidget::HandleConsumableDragEnded);

	SetBodyPartModeActive(true);
}

void UInventoryWidget::SetBodyPartModeActive(bool bActive)
{
	if (bBodyPartModeActive == bActive)
		return;

	bBodyPartModeActive = bActive;

	if (equipSlotPanel)
		equipSlotPanel->SetVisibility(bActive ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	if (bodyPartPanel)
		bodyPartPanel->SetVisibility(bActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bActive)
		RefreshBodyPartSlots();
	else
	{
		for (UBodyPartSlotWidget* slot : bodyPartSlots)
		{
			if (slot)
				slot->ClearHighlight();
		}
	}
}

void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	StyleInventory(this); // 화면이 만들어지기 전에 한 번. 여기서 넣은 위젯은 순서대로 그려진다
}

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 장비 아래 상태 줄(가방 화면에만 있다)
	const AWindlessCharacter* character = Cast<AWindlessCharacter>(GetOwningPlayerPawn());
	if (UAttributeComponent* attributes = character ? character->GetAttributes() : nullptr)
	{
		const FString values[] = {
			FString::Printf(TEXT("%.0f%%"), attributes->GetHealthPercent() * 100.f),
			FString::Printf(TEXT("%.0f/%.0f"), attributes->GetStamina(), attributes->GetMaxStamina()),
			FString::Printf(TEXT("%.0f/%.0f"), attributes->GetChaos(), attributes->GetMaxChaos()),
			FString::FromInt(attributes->GetMoltStones()) };
		for (int32 index = 0; index < UE_ARRAY_COUNT(values); ++index)
		{
			if (UTextBlock* value = Cast<UTextBlock>(GetWidgetFromName(*FString::Printf(TEXT("WI_Stat%d"), index))))
				value->SetText(FText::FromString(values[index]));
		}
	}

	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	SetKeyboardFocus();

	if (APawn* pawn = GetOwningPlayerPawn())
		owningInventory = pawn->FindComponentByClass<UInventoryComponent>();

	if (!owningInventory)
		return;

	if (playerGrid)
		playerGrid->InitializeGrid(owningInventory, this);

	if (storagePanel)
		storagePanel->SetVisibility(storageInventory ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (storageGrid && storageInventory)
		storageGrid->InitializeGrid(storageInventory, this);

	CollectEquipmentSlots();
	CollectConsumableSlots();
	CollectBodyPartSlots();
	SetBodyPartModeActive(false);

	owningInventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
	owningInventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::HandleInventoryChanged);
	owningInventory->OnEquipmentChanged.RemoveDynamic(this, &UInventoryWidget::HandleEquipmentChanged);
	owningInventory->OnEquipmentChanged.AddDynamic(this, &UInventoryWidget::HandleEquipmentChanged);
	owningInventory->OnQuickSlotChanged.RemoveDynamic(this, &UInventoryWidget::HandleQuickSlotChanged);
	owningInventory->OnQuickSlotChanged.AddDynamic(this, &UInventoryWidget::HandleQuickSlotChanged);

	if (storageInventory)
	{
		storageInventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
		storageInventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::HandleInventoryChanged);
	}
}

void UInventoryWidget::NativeDestruct()
{
	CancelHoverInfo();
	CloseAllPopups();
	SetBodyPartModeActive(false);

	if (owningInventory)
	{
		owningInventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
		owningInventory->OnEquipmentChanged.RemoveDynamic(this, &UInventoryWidget::HandleEquipmentChanged);
		owningInventory->OnQuickSlotChanged.RemoveDynamic(this, &UInventoryWidget::HandleQuickSlotChanged);
	}

	Super::NativeDestruct();
}

FReply UInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

FReply UInventoryWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

FReply UInventoryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 가방 여닫기 키(Tab). 위젯이 포커스를 쥐고 있으면 Tab이 게임 입력으로 가지 않아 여기서 닫는다.
	// 누르고 있어 반복된 입력은 무시한다(열자마자 닫히지 않게)
	if (InKeyEvent.GetKey() == EKeys::Tab)
	{
		AWindlessCharacter* character = Cast<AWindlessCharacter>(GetOwningPlayerPawn());
		if (character && !InKeyEvent.IsRepeat() && !IsAnyPopupOpen())
			character->CloseInventoryFromUI();
		return FReply::Handled();
	}

	if (IsAnyPopupOpen())
	{
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			CloseAllPopups();
			return FReply::Handled();
		}
		return FReply::Handled();
	}

	if (!owningInventory || hoveredUID == 0)
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	const FKey key = InKeyEvent.GetKey();

	if (key == EKeys::F) // 장착
	{
		FItemData data;
		const FItemInstance inst = owningInventory->FindInstance(hoveredUID);
		if (owningInventory->GetItemData(inst.itemId, data))
		{
			if (data.category == EItemCategory::EIC_Equipment) // 장비 아이템 장착
			{
				owningInventory->EquipItem(hoveredUID);
				return FReply::Handled();
			}
			if (data.category == EItemCategory::EIC_Consumable) // 소모성 아이템 장착
			{
				owningInventory->QuickEquipToQuickSlot(hoveredUID);
				return FReply::Handled();
			}
		}
	}

	if (key == EKeys::Q) // 버리기
	{
		owningInventory->DropItem(hoveredUID);
		return FReply::Handled();
	}

	if (key == EKeys::R) // 회전 
	{
		owningInventory->RotateItem(hoveredUID);
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UInventoryWidget::HandleInventoryChanged()
{
	CancelHoverInfo();
	RefreshAllGrids();
	RefreshEquipmentSlots();
	RefreshConsumableSlots();
	ValidatePopupTargets();
}

void UInventoryWidget::HandleEquipmentChanged()
{
	RefreshEquipmentSlots();
	RefreshAllGrids();
}

void UInventoryWidget::HandleQuickSlotChanged(EQuickSlot quickSlot)
{
	RefreshConsumableSlots();
	RefreshAllGrids();
}

void UInventoryWidget::HandleConsumableDragEnded(UDragDropOperation* operation)
{
	SetBodyPartModeActive(false);
}

void UInventoryWidget::CollectEquipmentSlots()
{
	equipmentSlots.Empty();
	if (!WidgetTree || !owningInventory)
		return;

	WidgetTree->ForEachWidget([this](UWidget* widget)
	{
		if (UEquipmentSlotWidget* slot = Cast<UEquipmentSlotWidget>(widget))
		{
			slot->InitializeSlot(owningInventory, this);
			equipmentSlots.Add(slot);
		}
	});
}

void UInventoryWidget::RefreshEquipmentSlots()
{
	for (UEquipmentSlotWidget* slot : equipmentSlots)
	{
		if (slot)
			slot->RefreshSlot();
	}
}

void UInventoryWidget::CollectConsumableSlots()
{
	consumableSlots.Empty();
	if (!WidgetTree || !owningInventory)
		return;

	WidgetTree->ForEachWidget([this](UWidget* widget)
		{
			if (UConsumableSlotWidget* slot = Cast<UConsumableSlotWidget>(widget))
			{
				slot->InitializeSlot(owningInventory, this);
				consumableSlots.Add(slot);
			}
		});
}

void UInventoryWidget::RefreshConsumableSlots()
{
	for (UConsumableSlotWidget* slot : consumableSlots)
	{
		if (slot)
			slot->RefreshSlot();
	}
}

void UInventoryWidget::CollectBodyPartSlots()
{
	bodyPartSlots.Empty();
	if (!WidgetTree || !owningInventory)
		return;

	UBodyPartComponent* bodyParts = nullptr;
	if (APawn* pawn = GetOwningPlayerPawn())
		bodyParts = pawn->FindComponentByClass<UBodyPartComponent>();

	WidgetTree->ForEachWidget([this, bodyParts](UWidget* widget)
		{
			if (UBodyPartSlotWidget* slot = Cast<UBodyPartSlotWidget>(widget))
			{
				slot->InitializeSlot(owningInventory, bodyParts, this);
				bodyPartSlots.Add(slot);
			}
		});
}

void UInventoryWidget::RefreshBodyPartSlots()
{
	for (UBodyPartSlotWidget* slot : bodyPartSlots)
	{
		if (slot)
			slot->RefreshSlot();
	}
}

void UInventoryWidget::ValidatePopupTargets()
{
	if (popupInventory)
	{
		if (contextMenu && !popupInventory->FindInstance(contextMenu->GetTargetUID()).IsValid())
			CloseContextMenu();

		if (splitPopup && !popupInventory->FindInstance(splitPopup->GetTargetUID()).IsValid())
			CloseSplitPopup();

		if (infoPopup && !popupInventory->FindInstance(infoPopup->GetTargetUID()).IsValid())
			CloseInfoPopup();
	}

	/* 호버 팝업은 자체 인벤토리를 사용 */
	if (hoverInfo)
	{
		if (!pendingHoverInventory || !pendingHoverInventory->FindInstance(hoverInfo->GetTargetUID()).IsValid())
			CancelHoverInfo();
		else
			hoverInfo->RefreshInfo();
	}
}

void UInventoryWidget::ShowHoverInfo()
{
	if (!pendingHoverInventory || !hoverInfoClass || pendingHoverUID == 0 || IsAnyPopupOpen())
		return;

	if (!pendingHoverInventory->FindInstance(pendingHoverUID).IsValid())
		return;

	CloseHoverInfo();

	hoverInfo = CreateWidget<UItemHoverInfoWidget>(GetOwningPlayer(), hoverInfoClass);
	if (!hoverInfo)
		return;

	UBorder* detail = Cast<UBorder>(GetWidgetFromName(TEXT("WI_Detail")));
	if (detail) // 가방 화면: 가방 칸 아래 설명 칸에 붙인다
	{
		hoverInfo->bDocked = true;
		detail->SetContent(hoverInfo);

		FItemData data;
		UImage* detailIcon = Cast<UImage>(GetWidgetFromName(TEXT("WI_DetailIcon")));
		if (detailIcon && pendingHoverInventory->GetItemData(pendingHoverInventory->FindInstance(pendingHoverUID).itemId, data) && data.icon)
		{
			detailIcon->SetBrushFromTexture(data.icon);
			detailIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		if (UWidget* hint = GetWidgetFromName(TEXT("WI_DetailHint")))
			hint->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
		hoverInfo->AddToViewport(popupZOrder + 2);

	hoverInfo->SetupInfo(this, pendingHoverInventory, pendingHoverUID);
	hoverInfo->UpdatePosition();
}

void UInventoryWidget::CloseHoverInfo()
{
	if (!hoverInfo)
		return;

	hoverInfo->RemoveFromParent();
	hoverInfo = nullptr;

	if (UWidget* detailIcon = GetWidgetFromName(TEXT("WI_DetailIcon")))
		detailIcon->SetVisibility(ESlateVisibility::Collapsed);
	if (UWidget* hint = GetWidgetFromName(TEXT("WI_DetailHint")))
		hint->SetVisibility(ESlateVisibility::HitTestInvisible);
}


