using UnrealBuildTool;

public class AwsaWebUI : ModuleRules
{
    public AwsaWebUI(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "JsonLibrary",
                "Json",
                "JsonUtilities",
                "UMG", 
                "QingyuanToolkits",
                "WebUI",
                "GameplayTags"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "AwsaPipe"
            }
        );
    }
}