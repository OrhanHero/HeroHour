// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HeroHour : ModuleRules
{
	public HeroHour(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Uncomment if you are using Slate UI.
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features.
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");
	}
}
