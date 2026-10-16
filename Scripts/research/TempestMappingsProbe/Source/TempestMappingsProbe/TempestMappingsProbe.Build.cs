using UnrealBuildTool;
public class TempestMappingsProbe : ModuleRules
{
 public TempestMappingsProbe(ReadOnlyTargetRules Target) : base(Target)
 {
  PCHUsage=PCHUsageMode.NoPCHs;
  // UE5.4's allocator header assumes Clang's feature-test macro with newer
  // MSVC versions. Keep this compatibility definition local to this probe.
  if(Target.Platform==UnrealTargetPlatform.Win64)
   PublicDefinitions.Add("__has_feature(x)=0");
  PrivateDependencyModuleNames.AddRange(new string[]{"Core","CoreUObject","Engine","Json"});
 }
}
