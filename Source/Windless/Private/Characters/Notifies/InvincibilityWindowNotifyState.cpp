#include "Characters/Notifies/InvincibilityWindowNotifyState.h"
#include "Characters/BaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UInvincibilityWindowNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (ABaseCharacter* character = MeshComp ? Cast<ABaseCharacter>(MeshComp->GetOwner()) : nullptr)
		character->SetInvincible(true);
}

void UInvincibilityWindowNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (ABaseCharacter* character = MeshComp ? Cast<ABaseCharacter>(MeshComp->GetOwner()) : nullptr)
		character->SetInvincible(false);
}

FString UInvincibilityWindowNotifyState::GetNotifyName_Implementation() const
{
	return TEXT("Invincible");
}
