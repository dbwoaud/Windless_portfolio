#pragma once

#include "CoreMinimal.h"
#include "UI/ItemInfoWidgetBase.h"
#include "ItemHoverInfoWidget.generated.h"

class UCanvasPanel;

UCLASS()
class WINDLESS_API UItemHoverInfoWidget : public UItemInfoWidgetBase
{
	GENERATED_BODY()

public:
	/* 배치 */
	bool bDocked = false; // 가방 화면의 설명 칸에 붙어 있는지 여부(커서를 따라가지 않고 바탕을 그리지 않는다)
	void UpdatePosition(); // 커서 위치를 기준으로 팝업을 배치하는 함수

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void HandleInvalidTarget() override; // 대상 아이템이 유효하지 않을 때 실행되는 함수


	/* 바인딩 위젯 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> rootCanvas; // 팝업 루트 캔버스


	/* 설정값 */
	UPROPERTY(EditAnywhere, Category = "HoverInfo")
	FVector2D cursorOffset = FVector2D(20.f, 20.f); // 커서로부터의 표시 간격

	UPROPERTY(EditAnywhere, Category = "HoverInfo")
	FVector2D screenEdgePadding = FVector2D(8.f, 8.f); // 화면 가장자리 여백

	UPROPERTY(EditAnywhere, Category = "HoverInfo")
	bool bFollowCursor = true; // 커서를 따라다닐지 여부
};