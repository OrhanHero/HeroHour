using UnrealBuildTool;

public class HeroHourEditorTarget : TargetRules
{
    public HeroHourEditorTarget(TargetInfo target) : base(target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("HeroHourPrototypeUE");
    }
}
