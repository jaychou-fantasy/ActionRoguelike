// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ActionRoguelike : ModuleRules
{
	public ActionRoguelike(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 让 #include "FileName.h" 可以在功能子目录中解析
		PublicIncludePaths.AddRange(
			new string[] {
				"ActionRoguelike",
				"ActionRoguelike/ActionSystem",
				"ActionRoguelike/AI",
				"ActionRoguelike/Animation",
				"ActionRoguelike/Core",
				"ActionRoguelike/Pickups",
				"ActionRoguelike/Player",
				"ActionRoguelike/Projectiles",
				"ActionRoguelike/SaveSystem",
				"ActionRoguelike/UI",
				"ActionRoguelike/World"
			}
		);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "AIModule", "EnhancedInput", "Slate","SlateCore","UMG","GameplayTags"});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
