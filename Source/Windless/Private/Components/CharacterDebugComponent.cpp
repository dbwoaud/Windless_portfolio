#include "Components/CharacterDebugComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/BodyPartComponent.h"
#include "Characters/BodyPartTypes.h"
#include "EnhancedInputComponent.h"
#include "Windless/DebugMacro.h"

UCharacterDebugComponent::UCharacterDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterDebugComponent::BindDebugInputs(UEnhancedInputComponent* enhancedInputComponent)
{
	if (!enhancedInputComponent)
		return;

	/* 정신력 디버그 */
	if (recoverChaosAction)
		enhancedInputComponent->BindAction(recoverChaosAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugRecoverChaos);
	if (loseChaosAction)
		enhancedInputComponent->BindAction(loseChaosAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugLoseChaos);

	/* 부위별 대미지 디버그 */
	if (damagePartAction)
		enhancedInputComponent->BindAction(damagePartAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugDamageSelectedPart);
	if (cyclePartAction)
		enhancedInputComponent->BindAction(cyclePartAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugCyclePart);
	if (healPartsAction)
		enhancedInputComponent->BindAction(healPartsAction, ETriggerEvent::Started, this, &UCharacterDebugComponent::DebugHealAllParts);
}

void UCharacterDebugComponent::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* owner = GetOwner())
	{
		ownerAttributes = owner->FindComponentByClass<UAttributeComponent>();
		ownerBodyParts = owner->FindComponentByClass<UBodyPartComponent>();
	}
}

void UCharacterDebugComponent::DebugRecoverChaos()
{
	if (ownerAttributes)
		ownerAttributes->AddChaos(-chaosDebugStep);
}

void UCharacterDebugComponent::DebugLoseChaos()
{
	if (ownerAttributes)
		ownerAttributes->AddChaos(+chaosDebugStep);
}

void UCharacterDebugComponent::DebugDamageSelectedPart()
{
	if (ownerBodyParts)
		ownerBodyParts->ApplyDamageToPart((EBodyPart)selectedPart, partDamageStep);
}

void UCharacterDebugComponent::DebugCyclePart()
{
	selectedPart++;
	if (selectedPart > (int32)EBodyPart::EBP_RightLeg)
		selectedPart = (int32)EBodyPart::EBP_Head;

	static const TCHAR* names[] = { TEXT(""), TEXT("머리"), TEXT("가슴"), TEXT("왼팔"), TEXT("오른팔"), TEXT("왼다리"), TEXT("오른다리") };
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(50, 3.f, FColor::White,
			FString::Printf(TEXT("[디버그] 선택 부위: %s"), names[selectedPart]));
	}
}

void UCharacterDebugComponent::DebugHealAllParts()
{
	if (ownerBodyParts)
		ownerBodyParts->HealAllParts();
}