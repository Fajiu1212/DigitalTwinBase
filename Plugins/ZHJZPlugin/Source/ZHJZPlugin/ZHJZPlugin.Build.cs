// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZHJZPlugin : ModuleRules
{
	public ZHJZPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				ModuleDirectory + "/Public"
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				ModuleDirectory + "/Private"
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"EnhancedInput", 
				"PakFile", 
				"GameplayTags", 
				"PakFile",
				"JsonLibrary", 
				"GeometryFramework",
				"DynamicMesh",
				"GeometryScriptingCore",
				"DynamicMesh",
				"AwsaWebUI"
			}
			);
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "RenderCore",
                "InputCore",
				"UMG",
				"QingyuanToolkits",
				"AwsaWebUI", 
				"PlacementPreview",
				"Json",
				"JsonUtilities"
            }
			);

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "ChunkDownloader",
				"HTTP",
				"PakFile"
			}
        );

        DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
