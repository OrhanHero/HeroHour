using UnrealBuildTool;
public class HeroHourPrototype : ModuleRules
{
    public HeroHourPrototype(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore", "GameplayTags" });
    }
}
