#include "Components/DialogueComponent.h"
#include "Windless/DebugMacro.h"
#include "Characters/WindlessCharacter.h"
#include "Dialogue/DialogueWidget.h"
#include "Dialogue/CharacterDialogueTypes.h"
#include "EnhancedInputComponent.h"
#include "Engine/DataTable.h"

UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueComponent::BindDialogueInputs(UEnhancedInputComponent* enhancedInputComponent)
{
	if (!enhancedInputComponent)
		return;

	if (advanceDialogueAction)
		enhancedInputComponent->BindAction(advanceDialogueAction, ETriggerEvent::Started, this, &UDialogueComponent::AdvanceDialogue);
}

void UDialogueComponent::StartDialogue(UDataTable* dialogueTable)
{
	if (bDialogueActive || !dialogueTable || !ownerCharacter)
		return;

	if (!dialogueWidget && dialogueWidgetClass) // 대화창을 생성해야 하는 경우
	{
		APlayerController* playerController = Cast<APlayerController>(ownerCharacter->GetController());
		if (playerController)
		{
			dialogueWidget = CreateWidget<UDialogueWidget>(playerController, dialogueWidgetClass);
			if (dialogueWidget)
				dialogueWidget->AddToViewport(10);
		}
	}

	if (!dialogueWidget)
		return;

	currentTable = dialogueTable;
	lineRowNames = dialogueTable->GetRowNames();
	lineRowNames.Sort([](const FName& A, const FName& B) { return A.ToString() < B.ToString(); });

	if (lineRowNames.Num() == 0)
		return;

	currentLineIndex = 0;
	bDialogueActive = true;

	ownerCharacter->SetDialogueInputMode(true);
	dialogueWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

	UE_LOG(LogWindless, Log, TEXT("[대화] 시작 - 총 %d줄"), lineRowNames.Num());
	ShowCurrentLine();
}

void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
	ownerCharacter = Cast<AWindlessCharacter>(GetOwner());
}

void UDialogueComponent::AdvanceDialogue()
{
	if (!bDialogueActive)
		return;

	++currentLineIndex;
	if (lineRowNames.IsValidIndex(currentLineIndex))
		ShowCurrentLine();
	else
		EndDialogue();
}

void UDialogueComponent::ShowCurrentLine()
{
	if (!currentTable || !dialogueWidget || !lineRowNames.IsValidIndex(currentLineIndex))
		return;

	const FName rowName = lineRowNames[currentLineIndex];
	const FDialogueLine* line = currentTable->FindRow<FDialogueLine>(rowName, TEXT("ShowCurrentLine"));
	if (line)
		dialogueWidget->SetLine(line->speakerName, line->dialogueText);
}

void UDialogueComponent::EndDialogue()
{
	bDialogueActive = false;

	if (dialogueWidget)
		dialogueWidget->SetVisibility(ESlateVisibility::Collapsed);

	if (ownerCharacter)
		ownerCharacter->SetDialogueInputMode(false);

	UE_LOG(LogWindless, Log, TEXT("대화 종료"));
	onDialogueFinished.Broadcast();
}