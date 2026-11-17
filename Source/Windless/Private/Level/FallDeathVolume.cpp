#include "Level/FallDeathVolume.h"
#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Windless/DebugMacro.h"

AFallDeathVolume::AFallDeathVolume()
{
	bTriggerOnce = false;
	bPlayerOnly = false;

	if (triggerBox)
	{
		triggerBox->SetBoxExtent(FVector(500.f, 500.f, 100.f));
		triggerBox->ShapeColor = FColor::Red;
		triggerBox->SetCanEverAffectNavigation(false);
	}
}

void AFallDeathVolume::HandleTriggerEnter(AActor* activator)
{
	Super::HandleTriggerEnter(activator);

	ABaseCharacter* character = Cast<ABaseCharacter>(activator);
	if (!character)
		return;

	UE_LOG(LogWindless, Log, TEXT("[낙사] %s 낙사 영역 %s 진입"), *character->GetName(), *GetName());
	character->DieByFall();
}