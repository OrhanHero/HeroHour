using UnrealBuildTool;
public class HeroHourTarget : TargetRules
{
    public HeroHourTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.Add("HeroHourPrototype");
    }
}
