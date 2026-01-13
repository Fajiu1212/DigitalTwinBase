using UnrealBuildTool;

public class ZHJZCore : ModuleRules
{
	public ZHJZCore(ReadOnlyTargetRules Target) : base(Target)
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
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"GameplayTags",
				"UMG",
			}
		);
	}
}