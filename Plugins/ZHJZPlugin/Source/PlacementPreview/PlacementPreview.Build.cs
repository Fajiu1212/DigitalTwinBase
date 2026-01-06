using UnrealBuildTool;

public class PlacementPreview : ModuleRules
{
	public PlacementPreview(ReadOnlyTargetRules Target) : base(Target)
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
				"UMG",
				"ApplicationCore",
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