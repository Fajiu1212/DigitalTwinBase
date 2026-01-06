using UnrealBuildTool;

public class ExtendEditorStyle : ModuleRules
{
	public ExtendEditorStyle(ReadOnlyTargetRules Target) : base(Target)
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
				"InputCore"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Projects",
				"UnrealEd",
				"EditorStyle"
			}
		);
	}
}