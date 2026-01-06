using UnrealBuildTool;

public class ExtendSlate : ModuleRules
{
	public ExtendSlate(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
				"ToolMenus",
				"EditorStyle",
				"ExtendEditorStyle",
				"ExtendEditorCommand"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore"
			}
		);
	}
}