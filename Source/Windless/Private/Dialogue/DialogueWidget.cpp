#include "Dialogue/DialogueWidget.h"
#include "Components/TextBlock.h"

void UDialogueWidget::SetLine(const FText& speaker, const FText& line)
{
	if (speakerText)
	{
		speakerText->SetText(speaker);
		speakerText->SetVisibility(speaker.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	if (dialogueText)
		dialogueText->SetText(line);
}
