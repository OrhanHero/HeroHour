using UnrealBuildTool;
public class HeroHourEditorTarget : TargetRules
{
    public HeroHourEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.Add("HeroHourPrototype");
    }
}
