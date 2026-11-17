// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Windless : ModuleRules
{
	public Windless(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "HairStrandsCore", "Niagara", "GeometryCollectionEngine", "UMG", "AIModule", "LevelSequence", "MovieScene", "Slate", "SlateCore", "DeveloperSettings", "NavigationSystem", "GameplayTasks" });

        PrivateDependencyModuleNames.AddRange(new string[] {  });

        if (Target.bBuildEditor) // blockout grid hotkey (Windless.cpp)
            PrivateDependencyModuleNames.AddRange(new string[] { "UnrealEd", "LevelEditor" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
