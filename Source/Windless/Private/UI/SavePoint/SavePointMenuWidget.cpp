#include "UI/SavePoint/SavePointMenuWidget.h"
#include "UI/SavePoint/SavePointNavButtonWidget.h"
#include "UI/SavePoint/LevelUpWidget.h"
#include "UI/SavePoint/RepairWidget.h"
#include "UI/SavePoint/CraftWidget.h"
#include "UI/SavePoint/FastTravelWidget.h"
#include "UI/InventoryWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/AttributeComponent.h"  
#include "Components/RefineComponent.h"
#include "Windless/DebugMacro.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Items/Storage/SharedStorageSubsystem.h" 
#include "GameFramework/Pawn.h"

void USavePointMenuWidget::InitializeMenu(const FText& savePointName, int32 currencyAmount)
{
	savePointTitle = savePointName.IsEmpty() ? defaultSavePointName : savePointName;

	if (currencyText)
		currencyText->SetText(FText::AsNumber(currencyAmount));

	if (APawn* pawn = GetOwningPlayerPawn())
	{
		if (UAttributeComponent* attributes = pawn->FindComponentByClass<UAttributeComponent>())
		{
			attributes->onMoltingChanged.RemoveDynamic(this, &USavePointMenuWidget::HandleMoltingChanged);
			attributes->onMoltingChanged.AddDynamic(this, &USavePointMenuWidget::HandleMoltingChanged);
		}
	}

	OpenPage(ESavePointMenuPage::ESMP_None);
}

void USavePointMenuWidget::OpenPage(ESavePointMenuPage newPage)
{
	currentPage = newPage;
	const bool bFullscreen = (newPage != ESavePointMenuPage::ESMP_None);

	if (pageRoot)
		pageRoot->SetVisibility(bFullscreen ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (fieldShade)
		fieldShade->SetVisibility(bFullscreen ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);

	if (bFullscreen)
	{
		lastOpenedPage = newPage;
		if (pageSwitcher)
			pageSwitcher->SetActiveWidgetIndex(static_cast<int32>(newPage) - 1);

		if (newPage == ESavePointMenuPage::ESMP_Storage)
			SetupStoragePage();

		if (newPage == ESavePointMenuPage::ESMP_LevelUp && levelUpWidget)
			levelUpWidget->RefreshDisplay();

		if (newPage == ESavePointMenuPage::ESMP_Repair && repairWidget)
			repairWidget->RefreshDisplay();

		if (newPage == ESavePointMenuPage::ESMP_Craft && craftWidget)
			craftWidget->RefreshDisplay();

		if (newPage == ESavePointMenuPage::ESMP_Travel && fastTravelWidget)
			fastTravelWidget->RefreshDisplay();

		RefreshCurrency();
	}

	RefreshHeader();
	RefreshNavSelection();
	FocusCurrentNavDeferred();
	OnPageChangedBP(newPage, bFullscreen);
}

void USavePointMenuWidget::Back()
{
	if (currentPage != ESavePointMenuPage::ESMP_None)
		OpenPage(ESavePointMenuPage::ESMP_None);
	else
		onCloseRequested.Broadcast();
}

void USavePointMenuWidget::FocusCurrentNav()
{
	const ESavePointMenuPage target = (currentPage != ESavePointMenuPage::ESMP_None) ? currentPage : lastOpenedPage;

	USavePointNavButtonWidget* nav = FindNavButton(target);
	if (!nav && navButtons.Num() > 0)
		nav = navButtons[0];

	if (nav && nav->GetButton())
		nav->GetButton()->SetKeyboardFocus();
}

void USavePointMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);

	// 트리 전체에서 기능 버튼 수집
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* widget)
			{
				if (USavePointNavButtonWidget* nav = Cast<USavePointNavButtonWidget>(widget))
				{
					nav->onNavClicked.AddDynamic(this, &USavePointMenuWidget::HandleNavClicked);
					navButtons.Add(nav);
				}
			});
	}

#if !UE_BUILD_SHIPPING
	const int32 expectedPages = static_cast<int32>(ESavePointMenuPage::ESMP_MAX) - 1;
	if (pageSwitcher && pageSwitcher->GetNumWidgets() != expectedPages)
		UE_LOG(LogWindless, Error, TEXT("[세이브포인트 메뉴] pageSwitcher 자식 수 %d (필요 %d)"), pageSwitcher->GetNumWidgets(), expectedPages);
#endif
}

FReply USavePointMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (backKeys.Contains(InKeyEvent.GetKey()))
	{
		Back();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply USavePointMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FocusCurrentNav();
	return FReply::Handled();
}

void USavePointMenuWidget::RefreshHeader()
{
	const USavePointNavButtonWidget* nav = FindNavButton(currentPage);

	if (titleText)
		titleText->SetText(nav ? nav->GetLabel() : savePointTitle);

	if (subtitleText)
		subtitleText->SetText(nav ? nav->GetSubtitle() : fieldSubtitle);
}

void USavePointMenuWidget::RefreshNavSelection()
{
	for (USavePointNavButtonWidget* nav : navButtons)
	{
		if (nav)
			nav->SetSelected(nav->GetPage() == currentPage);
	}
}

void USavePointMenuWidget::FocusCurrentNavDeferred()
{
	FocusCurrentNav();

	if (UWorld* world = GetWorld())
	{
		world->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					FocusCurrentNav();
				}));
	}
}

void USavePointMenuWidget::SetupStoragePage()
{
	if (!storageInventoryWidget)
		return;

	UWorld* world = GetWorld();
	USharedStorageSubsystem* storage = world ? world->GetSubsystem<USharedStorageSubsystem>() : nullptr;
	if (!storage)
		return;

	storageInventoryWidget->SetStorageInventory(storage->GetStorageInventory());
	storageInventoryWidget->RefreshAllGrids();
}

void USavePointMenuWidget::RefreshCurrency()
{
	APawn* pawn = GetOwningPlayerPawn();
	UAttributeComponent* attributes = pawn ? pawn->FindComponentByClass<UAttributeComponent>() : nullptr;

	if (currencyText && attributes)
		currencyText->SetText(FText::AsNumber(attributes->GetMoltStones()));
}

void USavePointMenuWidget::HandleNavClicked(ESavePointMenuPage page)
{
	OpenPage(page);
}

USavePointNavButtonWidget* USavePointMenuWidget::FindNavButton(ESavePointMenuPage page) const
{
	for (USavePointNavButtonWidget* nav : navButtons)
	{
		if (nav && nav->GetPage() == page)
			return nav;
	}
	return nullptr;
}

void USavePointMenuWidget::HandleMoltingChanged(int32 newLevel, float expPercent)
{
	RefreshCurrency();
}
