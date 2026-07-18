// HeroHour.Build.cs
// UE5 Module Build Configuration

using UnrealBuildTool;

public class HeroHour : ModuleRules
{
    public HeroHour(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks",
            "AIModule",
            "NavigationSystem",
            "UMG",
            "Slate",
            "SlateCore",
            "Niagara",
            "WorldPartition",
            "MassEntity",
            "StateTreeModule",
            "GameFeatures",
            "NetCore",
            "ReplicationGraph",
            "Json",
            "JsonUtilities"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "Renderer",
            "RenderCore",
            "RHI",
            "Projects",
            "DeveloperSettings"
        });

        // F# Integration
        if (Target.Type == TargetRules.TargetType.Editor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }

        // Public include paths
        PublicIncludePaths.AddRange(new string[]
        {
            ModuleDirectory + "/Public",
            ModuleDirectory + "/Public/Simulation",
            ModuleDirectory + "/Public/GameplayAbility",
            ModuleDirectory + "/Public/AI",
            ModuleDirectory + "/Public/Network",
            ModuleDirectory + "/Public/Data",
            ModuleDirectory + "/Public/UI"
        });

        // C++ Standard
        CppStandard = CppStandardVersion.Cpp20;

        // Optimization
        bUseUnity = true;
        bUsePCHFiles = true;

        // Definitions
        PublicDefinitions.Add("HEROHOUR_API=DLLEXPORT");
        PublicDefinitions.Add("WITH_GAMEPLAY_ABILITIES=1");
        PublicDefinitions.Add("WITH_MASS_ENTITY=1");
        PublicDefinitions.Add("WITH_STATE_TREE=1");
    }
}