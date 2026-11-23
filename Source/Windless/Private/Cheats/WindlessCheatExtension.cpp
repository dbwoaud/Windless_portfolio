#include "Cheats/WindlessCheatExtension.h"
#include "Characters/WindlessCharacter.h"
#include "GameFramework/PlayerController.h"

UWindlessCheatExtension::UWindlessCheatExtension()
{
#if UE_WITH_CHEAT_MANAGER
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		UClass* extensionClass = GetClass();
		UCheatManager::RegisterForOnCheatManagerCreated(FOnCheatManagerCreated::FDelegate::CreateLambda(
			[extensionClass](UCheatManager* cheatManager)
			{
				if (!cheatManager || !extensionClass || extensionClass->HasAnyClassFlags(CLASS_Abstract))
					return;

				cheatManager->AddCheatManagerExtension(NewObject<UCheatManagerExtension>(cheatManager, extensionClass));
			}));
	}
#endif
}

APawn* UWindlessCheatExtension::GetPlayerPawn() const
{
	const APlayerController* playerController = GetPlayerController();
	return playerController ? playerController->GetPawn() : nullptr;
}

AWindlessCharacter* UWindlessCheatExtension::GetPlayerCharacter() const
{
	return Cast<AWindlessCharacter>(GetPlayerPawn());
}