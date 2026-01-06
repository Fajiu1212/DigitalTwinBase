using UnrealBuildTool;

public class QingyuanToolkits : ModuleRules
{
	public QingyuanToolkits(ReadOnlyTargetRules Target) : base(Target)
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
				"CoreUObject",
				"Engine",
				"InputCore",
				"Json",
				"JsonUtilities",
				"ApplicationCore",
				"PakFile",
				"Slate",
				"UMG", 
				"Slate", 
				"SlateCore"
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"HTTP"
			}
		);

		if (Target.bBuildEditor == true)
		{
			PrivateIncludePathModuleNames.AddRange(
				new string[]
				{
					"UnrealEd"
				}
			);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"EditorFramework",
					"UnrealEd"
				}
			);
		}

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}