using UnrealBuildTool;

public class HeroHourTarget : TargetRules
{
    public HeroHourTarget(TargetInfo target) : base(target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("HeroHourPrototypeUE");
    }
}
