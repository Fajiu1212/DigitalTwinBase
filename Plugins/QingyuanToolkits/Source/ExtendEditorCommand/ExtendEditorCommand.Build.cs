using UnrealBuildTool;

public class ExtendEditorCommand : ModuleRules
{
    public ExtendEditorCommand(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory + "/Public"
            }
        );


        PrivateIncludePaths.AddRange(
            new string[]
            {
                ModuleDirectory + "/Private"
            }
        );
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "ExtendEditorStyle",
                "InputCore"
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