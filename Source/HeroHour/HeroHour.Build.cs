// HeroHour.Build.cs
// UE5 Module Build Configuration with Epic Online Services (EOS) Integration

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
            "JsonUtilities",
            // Online Subsystem & EOS
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "OnlineSubsystemEOS"
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
            "DeveloperSettings",
            // Networking
            "NetCore",
            "PacketHandler",
            // Online Subsystem EOS (private for implementation details)
            "OnlineSubsystemEOS",
            "OnlineSubsystem"
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
            ModuleDirectory + "/Public/UI",
            ModuleDirectory + "/Public/Online"
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
        // EOS Definitions
        PublicDefinitions.Add("WITH_EOS=1");
        PublicDefinitions.Add("EOS_SDK_VERSION=1.16");
    }
}