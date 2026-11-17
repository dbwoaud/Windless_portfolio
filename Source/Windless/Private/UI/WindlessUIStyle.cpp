#include "UI/WindlessUIStyle.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Materials/MaterialInterface.h"

namespace WindlessUI
{
	FSlateBrush Box(const FLinearColor& fill, const FLinearColor& outline, float width)
	{
		FSlateBrush brush;
		brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		brush.TintColor = FSlateColor(fill);
		brush.OutlineSettings = FSlateBrushOutlineSettings(0.f, FSlateColor(outline), width);
		return brush;
	}

	FSlateBrush Tile(float fillAlpha, float outlineAlpha)
	{
		return Box(Alpha(GoldDim, fillAlpha), Alpha(GoldDim, outlineAlpha));
	}

	FSlateBrush Popup()
	{
		return Box(FLinearColor(0.012f, 0.011f, 0.008f, 0.96f), Alpha(GoldDim, 0.45f));
	}

	FSlateBrush Circle(const FLinearColor& fill, const FLinearColor& outline)
	{
		FSlateBrush brush; // 테두리 설정 기본값이 "높이의 절반" 둥글기라 정사각형이면 원이 된다
		brush.DrawAs = ESlateBrushDrawType::RoundedBox;
		brush.TintColor = FSlateColor(fill);
		brush.OutlineSettings.Color = FSlateColor(outline);
		brush.OutlineSettings.Width = 1.f;
		return brush;
	}

	FSlateBrush Material(const TCHAR* name)
	{
		FSlateBrush brush;
		brush.ImageType = ESlateBrushImageType::FullColor;
		brush.SetResourceObject(LoadObject<UMaterialInterface>(nullptr,
			*FString::Printf(TEXT("/Game/Windless/UI/SavePoint/Materials/%s.%s"), name, name)));
		return brush;
	}

	void Text(UTextBlock* text, float size, int32 letterSpacing, const FLinearColor& color)
	{
		if (!text)
			return;

		FSlateFontInfo font = text->GetFont();
		font.Size = size;
		font.LetterSpacing = letterSpacing;
		text->SetFont(font);
		text->SetColorAndOpacity(FSlateColor(color));
	}

	void BodyText(UTextBlock* text) { Text(text, 11.f, 0, Body); }

	void EmptyIcon(UImage* image)
	{
		if (!image)
			return;

		image->SetBrushTintColor(FSlateColor(Alpha(GoldDim, 0.07f)));
		image->SetColorAndOpacity(FLinearColor::White);
	}

	void Bar(UProgressBar* bar)
	{
		if (!bar)
			return;

		FProgressBarStyle style = bar->GetWidgetStyle();
		style.BackgroundImage.TintColor = FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.6f));
		style.FillImage.TintColor = FSlateColor(Gold);
		bar->SetWidgetStyle(style);
		bar->SetFillColorAndOpacity(FLinearColor::White);
	}

	static UTextBlock* MakeText(UWidgetTree* tree, const FText& value)
	{
		UTextBlock* text = tree->ConstructWidget<UTextBlock>();
		text->SetText(value);
		return text;
	}

	static USpacer* MakeSpacer(UWidgetTree* tree, float width, float height)
	{
		USpacer* spacer = tree->ConstructWidget<USpacer>();
		spacer->SetSize(FVector2D(width, height));
		return spacer;
	}

	static UImage* MakeImage(UWidgetTree* tree, const FSlateBrush& brush)
	{
		UImage* image = tree->ConstructWidget<UImage>();
		image->SetBrush(brush);
		return image;
	}

	static void Align(UOverlaySlot* slot, EHorizontalAlignment horizontal = HAlign_Fill, EVerticalAlignment vertical = VAlign_Fill)
	{
		slot->SetHorizontalAlignment(horizontal);
		slot->SetVerticalAlignment(vertical);
	}

	UTextBlock* Caption(UUserWidget* owner, const FText& text)
	{
		UTextBlock* caption = MakeText(owner->WidgetTree, text);
		Text(caption, 10.f, 340, Alpha(GoldDim, 0.65f));
		return caption;
	}

	UWidget* Header(UUserWidget* owner, const FText& subtitle, const FText& title)
	{
		UWidgetTree* tree = owner->WidgetTree;
		UHorizontalBox* row = tree->ConstructWidget<UHorizontalBox>();

		// 둥근 틀 안의 작은 고리 (세이브포인트 머리말 아이콘)
		USizeBox* iconBox = tree->ConstructWidget<USizeBox>();
		iconBox->SetWidthOverride(56.f);
		iconBox->SetHeightOverride(56.f);
		UOverlay* icon = tree->ConstructWidget<UOverlay>();
		Align(icon->AddChildToOverlay(MakeImage(tree, Circle(FLinearColor(0.f, 0.f, 0.f, 0.5f), Alpha(GoldDim, 0.45f)))));
		FSlateBrush ring = Circle(FLinearColor::Transparent, FLinearColor(0.760525f, 0.637597f, 0.381326f, 0.85f));
		ring.ImageSize = FVector2D(16.f, 16.f);
		Align(icon->AddChildToOverlay(MakeImage(tree, ring)), HAlign_Center, VAlign_Center);
		iconBox->AddChild(icon);
		row->AddChildToHorizontalBox(iconBox);
		row->AddChildToHorizontalBox(MakeSpacer(tree, 18.f, 1.f));

		UVerticalBox* texts = tree->ConstructWidget<UVerticalBox>();
		UTextBlock* subtitleText = MakeText(tree, subtitle);
		Text(subtitleText, 11.f, 300, Alpha(GoldDim, 0.6f));
		texts->AddChildToVerticalBox(subtitleText);
		UTextBlock* titleText = MakeText(tree, title);
		Text(titleText, 27.f, 100, Cream);
		titleText->SetShadowOffset(FVector2D(0.f, 2.f));
		titleText->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.9f));
		texts->AddChildToVerticalBox(titleText);
		row->AddChildToHorizontalBox(texts)->SetVerticalAlignment(VAlign_Center);
		return row;
	}

	UWidget* KeyHints(UUserWidget* owner, const TArray<TPair<FText, FText>>& hints)
	{
		UWidgetTree* tree = owner->WidgetTree;
		UHorizontalBox* row = tree->ConstructWidget<UHorizontalBox>();

		for (const TPair<FText, FText>& hint : hints)
		{
			// 키 글자를 둥근 틀에 넣는다. 글자가 길면 틀이 옆으로 늘어난다
			USizeBox* keyBox = tree->ConstructWidget<USizeBox>();
			keyBox->SetMinDesiredWidth(26.f);
			keyBox->SetHeightOverride(26.f);
			UOverlay* key = tree->ConstructWidget<UOverlay>();
			Align(key->AddChildToOverlay(MakeImage(tree, Circle(FLinearColor::Transparent, Alpha(GoldDim, 0.5f)))));
			UTextBlock* keyText = MakeText(tree, hint.Key);
			Text(keyText, 10.f, 0, Gold);
			UOverlaySlot* keySlot = key->AddChildToOverlay(keyText);
			Align(keySlot, HAlign_Center, VAlign_Center);
			keySlot->SetPadding(FMargin(8.f, 0.f));
			keyBox->AddChild(key);
			row->AddChildToHorizontalBox(keyBox)->SetVerticalAlignment(VAlign_Center);
			row->AddChildToHorizontalBox(MakeSpacer(tree, 10.f, 1.f));

			UTextBlock* label = MakeText(tree, hint.Value);
			BodyText(label);
			row->AddChildToHorizontalBox(label)->SetVerticalAlignment(VAlign_Center);
			row->AddChildToHorizontalBox(MakeSpacer(tree, 34.f, 1.f));
		}
		return row;
	}

	void StyleTree(UUserWidget* owner)
	{
		if (!owner || !owner->WidgetTree)
			return;

		owner->WidgetTree->ForEachWidget([](UWidget* widget)
		{
			const FName name = widget->GetFName();

			if (UButton* button = Cast<UButton>(widget))
			{
				// 제작 목록 줄과 같은 버튼: 옅은 금색 바탕에 얇은 금색 테두리
				FButtonStyle style = button->GetStyle();
				style.Normal = Tile(0.03f, 0.2f);
				style.Hovered = Tile(0.12f, 0.2f);
				style.Pressed = Tile(0.22f, 0.2f);
				style.Disabled = Tile(0.f, 0.08f);
				button->SetStyle(style);
				if (UTextBlock* label = Cast<UTextBlock>(button->GetChildAt(0)))
					label->SetColorAndOpacity(FSlateColor(ButtonText));
			}
			else if (UBorder* border = Cast<UBorder>(widget))
			{
				if (name == TEXT("popupBorder") || name == TEXT("menuBorder"))
				{
					border->SetBrush(Popup());
					border->SetBrushColor(FLinearColor::White);
				}
				else if (name == TEXT("descriptionPanel"))
				{
					border->SetBrush(Tile());
					border->SetBrushColor(FLinearColor::White);
					border->SetPadding(FMargin(12.f));
				}
			}
			else if (UTextBlock* text = Cast<UTextBlock>(widget))
			{
				if (name == TEXT("categoryText"))
					Text(text, 10.f, 280, Alpha(GoldDim, 0.65f));
				else if (name == TEXT("itemNameText"))
					Text(text, 20.f, 60, Cream);
				else if (name == TEXT("statText") || name == TEXT("descriptionText") || name == TEXT("amountText"))
					Text(text, 12.f, 0, Body);
				else if (name == TEXT("headerText")) // 색은 등급 색을 쓰는 곳이 있어 건드리지 않는다
				{
					FSlateFontInfo font = text->GetFont();
					font.Size = 13.f;
					font.LetterSpacing = 120;
					text->SetFont(font);
				}
			}
		});
	}
}

void WindlessUI::NameLabel(UUserWidget* owner, UImage* icon, const FText& name)
{
	UTextBlock* label = owner ? Cast<UTextBlock>(owner->GetWidgetFromName(TEXT("WI_Name"))) : nullptr;
	UOverlay* overlay = icon ? Cast<UOverlay>(icon->GetParent()) : nullptr;
	if (!label && overlay && owner && owner->WidgetTree && !name.IsEmpty())
	{
		label = owner->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WI_Name"));
		Text(label, 10.f, 0, Cream);
		label->SetAutoWrapText(true);
		label->SetJustification(ETextJustify::Center);
		label->SetVisibility(ESlateVisibility::HitTestInvisible);
		UOverlaySlot* labelSlot = overlay->AddChildToOverlay(label);
		labelSlot->SetHorizontalAlignment(HAlign_Fill);
		labelSlot->SetVerticalAlignment(VAlign_Center);
		labelSlot->SetPadding(FMargin(5.f));
	}
	if (label)
		label->SetText(name);
}
