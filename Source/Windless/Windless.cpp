// Copyright Epic Games, Inc. All Rights Reserved.

#include "Windless.h"
#include "DebugMacro.h" 
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Framework/Commands/Commands.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "LevelEditor.h"
#include "Materials/MaterialParameterCollection.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "WindlessEditor"

// ponytail: editor-only code lives in the game module under WITH_EDITOR. Move to a WindlessEditor module when a second editor tool appears
class FWindlessEditorCommands : public TCommands<FWindlessEditorCommands>
{
public:
	FWindlessEditorCommands()
		: TCommands<FWindlessEditorCommands>(TEXT("Windless"), LOCTEXT("Windless", "Windless"), NAME_None, FAppStyle::GetAppStyleSetName())
	{
	}

	virtual void RegisterCommands() override
	{
		// N: not bound by the level editor or level viewport by default. Rebind in Editor Preferences > Keyboard Shortcuts > Windless
		UI_COMMAND(toggleBlockoutGrid, "Toggle Blockout Grid", "Show or hide the 1 m grid lines on blockout materials", EUserInterfaceActionType::Button, FInputChord(EKeys::N));
	}

	TSharedPtr<FUICommandInfo> toggleBlockoutGrid;
};

#undef LOCTEXT_NAMESPACE

static void ToggleBlockoutGrid()
{
	UWorld* world = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	UMaterialParameterCollection* collection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Blockout/Materials/MPC_Blockout.MPC_Blockout"));
	if (!world || !collection)
		return;

	// the value lives on the editor world only, so PIE and packaged games stay at the collection default (off)
	static const FName gridOn(TEXT("GridOn"));
	const bool bOn = UKismetMaterialLibrary::GetScalarParameterValue(world, collection, gridOn) > 0.5f;
	UKismetMaterialLibrary::SetScalarParameterValue(world, collection, gridOn, bOn ? 0.f : 1.f);
	GEditor->RedrawLevelEditingViewports();
}

class FWindlessModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		if (!GIsEditor || IsRunningCommandlet())
			return;

		FWindlessEditorCommands::Register();
		FLevelEditorModule& levelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		levelEditor.GetGlobalLevelEditorActions()->MapAction(FWindlessEditorCommands::Get().toggleBlockoutGrid, FExecuteAction::CreateStatic(&ToggleBlockoutGrid));
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FWindlessModule, Windless, "Windless" );
#else
IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, Windless, "Windless" );
#endif
DEFINE_LOG_CATEGORY(LogWindless);