#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CharacterDialogueTypes.generated.h"

USTRUCT(BlueprintType)
struct FDialogueLine : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText speakerName; // 화자 이름

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = true))
	FText dialogueText; // 대사 본문
};