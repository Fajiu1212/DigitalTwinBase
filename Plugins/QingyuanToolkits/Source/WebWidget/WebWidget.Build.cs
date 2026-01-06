using UnrealBuildTool;

public class WebWidget : ModuleRules
{
	public WebWidget(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

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
				"SlateCore"
			}
		);
	}
}