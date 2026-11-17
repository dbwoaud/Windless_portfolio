#include "Combat/ComboWindowNotifyState.h"
#include "Components/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Windless/DebugMacro.h"

void UComboWindowNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UCombatComponent* combat = FindCombat(MeshComp);
	if (combat)
		combat->OpenComboWindow();
}

void UComboWindowNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UCombatComponent* combat = FindCombat(MeshComp))
		combat->CloseComboWindow();
}

FString UComboWindowNotifyState::GetNotifyName_Implementation() const
{
	return TEXT("ComboWindow");
}

UCombatComponent* UComboWindowNotifyState::FindCombat(USkeletalMeshComponent* meshComp)
{
	AActor* owner = meshComp ? meshComp->GetOwner() : nullptr;
	return owner ? owner->FindComponentByClass<UCombatComponent>() : nullptr;
}
