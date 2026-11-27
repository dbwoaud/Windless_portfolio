#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "DropItemSpawnNotify.generated.h"

/*
 * 버리기 몽타주에서 드롭 액터를 생성할 시점을 알리는 노티파이
 */

UCLASS(meta = (DisplayName = "Drop Item Spawn"))
class WINDLESS_API UDropItemSpawnNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};