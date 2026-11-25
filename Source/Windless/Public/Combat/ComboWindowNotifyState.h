#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ComboWindowNotifyState.generated.h"

class UCombatComponent;

UCLASS(meta = (DisplayName = "Combo Window"))
class WINDLESS_API UComboWindowNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	static UCombatComponent* FindCombat(USkeletalMeshComponent* meshComp); // 캐릭터의 전투 컴포넌트를 찾는 함수
};