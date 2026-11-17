#include "UI/SavePoint/LevelUpWidget.h"
#include "Characters/WindlessCharacter.h"
#include "Characters/LevelUpTypes.h"
#include "Components/AttributeComponent.h"
#include "Components/BodyPartComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void ULevelUpWidget::RefreshDisplay()
{
	AWindlessCharacter* player = GetPlayer();
	UAttributeComponent* attributes = player ? player->FindComponentByClass<UAttributeComponent>() : nullptr;
	if (!attributes)
		return;

	const int32 level = attributes->GetCurrentLevel();
	const int32 stones = attributes->GetMoltStones();
	const int32 cost = attributes->GetMoltCost();

	if (levelText)
		levelText->SetText(FText::AsNumber(level));

	if (nextLevelText)
		nextLevelText->SetText(FText::AsNumber(level + 1));

	if (stoneText)
		stoneText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), stones, cost)));

	if (stoneBar)
		stoneBar->SetPercent(attributes->GetMoltingExpPercent());

	if (moltButton)
		moltButton->SetIsEnabled(attributes->CanMolt());

	if (statPreviewText)
	{
		const float stamina = attributes->GetMaxStamina();
		const float attack = attributes->GetBaseAttackPower();
		const float will = attributes->GetWillPower();

		const UBodyPartComponent* bodyParts = player->FindComponentByClass<UBodyPartComponent>();
		auto partMax = [bodyParts](EBodyPart part) -> float
			{
				return bodyParts ? bodyParts->GetPartMaxHealth(part) : 0.f;
			};

		const float head = partMax(EBodyPart::EBP_Head);
		const float chest = partMax(EBodyPart::EBP_Chest);
		const float leftArm = partMax(EBodyPart::EBP_LeftArm);
		const float rightArm = partMax(EBodyPart::EBP_RightArm);
		const float leftLeg = partMax(EBodyPart::EBP_LeftLeg);
		const float rightLeg = partMax(EBodyPart::EBP_RightLeg);

		statPreviewText->SetText(FText::FromString(FString::Printf(
			TEXT("머리 체력\t%.0f -> %.0f\n")
			TEXT("가슴 체력\t%.0f -> %.0f\n")
			TEXT("왼팔 체력\t%.0f -> %.0f\n")
			TEXT("오른팔 체력\t%.0f -> %.0f\n")
			TEXT("왼다리 체력\t%.0f -> %.0f\n")
			TEXT("오른다리 체력\t%.0f -> %.0f\n")
			TEXT("스태미나\t%.0f -> %.0f\n")
			TEXT("공격력\t%.0f -> %.0f\n")
			TEXT("정신력\t%.0f -> %.0f"),
			head, head + WindlessLevelUp::HeadHealthPerLevel,
			chest, chest + WindlessLevelUp::ChestHealthPerLevel,
			leftArm, leftArm + WindlessLevelUp::LimbHealthPerLevel,
			rightArm, rightArm + WindlessLevelUp::LimbHealthPerLevel,
			leftLeg, leftLeg + WindlessLevelUp::LimbHealthPerLevel,
			rightLeg, rightLeg + WindlessLevelUp::LimbHealthPerLevel,
			stamina, stamina + WindlessLevelUp::StaminaPerLevel,
			attack, attack + WindlessLevelUp::AttackPowerPerLevel,
			will, will + WindlessLevelUp::WillPowerPerLevel)));
	}

	if (inventoryPreviewText)
	{
		int32 curW = 0, curH = 0, nextW = 0, nextH = 0;
		WindlessLevelUp::GetInventorySizeForLevel(level, curW, curH);
		WindlessLevelUp::GetInventorySizeForLevel(level + 1, nextW, nextH);

		inventoryPreviewText->SetText((nextW != curW || nextH != curH)
			? FText::FromString(FString::Printf(TEXT("소지품 %dx%d -> %dx%d"), curW, curH, nextW, nextH))
			: FText::GetEmpty());
	}
}

void ULevelUpWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (moltButton)
		moltButton->OnClicked.AddDynamic(this, &ULevelUpWidget::HandleMoltClicked);
}

void ULevelUpWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDisplay();
}

void ULevelUpWidget::HandleMoltClicked()
{
	AWindlessCharacter* player = GetPlayer();
	if (player && player->TryMolt())
		OnMoltSucceededBP();

	RefreshDisplay();
}

AWindlessCharacter* ULevelUpWidget::GetPlayer() const
{
	return Cast<AWindlessCharacter>(GetOwningPlayerPawn());
}
