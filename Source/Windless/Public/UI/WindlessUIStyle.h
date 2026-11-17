#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UUserWidget;
class UWidget;

/**
 * 세이브포인트 UI(WBP_SavePointMenuWidget, WBP_SavePointNavButtonWidget, WBP_CraftWidget)에서 뽑은 공용 스타일.
 * 가방과 퀵슬롯이 같은 값을 쓴다. 값을 바꾸려면 여기만 고친다.
 */
namespace WindlessUI
{
	/* 색 (위젯 블루프린트에 적힌 선형 값 그대로) */
	inline const FLinearColor GoldDim(0.564712f, 0.391573f, 0.147027f); // 작은 글자와 테두리
	inline const FLinearColor Gold(0.838799f, 0.693872f, 0.381326f); // 값, 키 글자, 강조
	inline const FLinearColor Cream(0.838799f, 0.760525f, 0.590619f); // 제목
	inline const FLinearColor Body(0.577581f, 0.527115f, 0.428691f, 0.9f); // 본문
	inline const FLinearColor ButtonText = FLinearColor(FColor(190, 182, 164, 255)); // 버튼 글자 (내비 버튼 기본)

	inline FLinearColor Alpha(FLinearColor color, float alpha) { color.A = alpha; return color; }

	/* 붓 */
	WINDLESS_API FSlateBrush Box(const FLinearColor& fill, const FLinearColor& outline, float width = 1.f); // 각진 상자
	WINDLESS_API FSlateBrush Tile(float fillAlpha = 0.04f, float outlineAlpha = 0.3f); // 금색 얇은 테두리 칸 (제작 버튼과 같은 모양)
	WINDLESS_API FSlateBrush Popup(); // 팝업 바탕
	WINDLESS_API FSlateBrush Circle(const FLinearColor& fill, const FLinearColor& outline); // 둥근 틀 (머리말 아이콘, 키 표시)
	WINDLESS_API FSlateBrush Material(const TCHAR* name); // 세이브포인트 UI 재질 (MI_UI_PageBackdrop 등)

	/* 글자: 크기, 자간, 색 */
	WINDLESS_API void Text(UTextBlock* text, float size, int32 letterSpacing, const FLinearColor& color);
	WINDLESS_API void BodyText(UTextBlock* text); // 본문 (11)

	WINDLESS_API void EmptyIcon(UImage* image); // 빈 칸 표시: 회색 상자 대신 옅은 금색
	WINDLESS_API void Bar(UProgressBar* bar); // 내구도 막대: 금색
	WINDLESS_API void NameLabel(UUserWidget* owner, UImage* icon, const FText& name); // 아이콘 자리에 이름을 적는다. 빈 글자면 지운다

	/* 조립 */
	WINDLESS_API UTextBlock* Caption(UUserWidget* owner, const FText& text); // 구역 위 작은 제목 (10, 자간 340)
	WINDLESS_API UWidget* Header(UUserWidget* owner, const FText& subtitle, const FText& title); // 화면 왼쪽 위 머리말
	WINDLESS_API UWidget* KeyHints(UUserWidget* owner, const TArray<TPair<FText, FText>>& hints); // 화면 아래 키 안내 줄

	/** 위젯 안의 버튼, 팝업 바탕, 이름이 정해진 글자에 공용 스타일을 입힌다. */
	WINDLESS_API void StyleTree(UUserWidget* owner);
}
