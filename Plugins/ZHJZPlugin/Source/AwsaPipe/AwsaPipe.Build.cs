using UnrealBuildTool;

public class AwsaPipe : ModuleRules
{
    public AwsaPipe(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory + "/Public"
            });
		
        PrivateIncludePaths.AddRange(new string[]
            {
                ModuleDirectory + "/Private"
            }
        );
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            }
        );
    }
}