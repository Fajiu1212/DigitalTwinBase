// Copyright CengJia.  Year : 2021

using System.Collections.Generic;
using System.IO;
using UnrealBuildTool;

public class OpenCVPlayer : ModuleRules
{

    public OpenCVPlayer(ReadOnlyTargetRules Target) : base(Target)
	{
		if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty/include"));
            PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty/include/opencv2"));
     
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty/lib/opencv_handle.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "ThirdParty/lib/opencv_world490.lib"));
            
            string DllDirectory = Path.Combine(ModuleDirectory, "ThirdParty/dll/");
            
            List<string> Dlls = new List<string>();
            Dlls.Add("opencv_world490.dll");
            Dlls.Add("opencv_videoio_ffmpeg490_64.dll");
            Dlls.Add("opencv_videoio_msmf490_64.dll");
            Dlls.Add("opencv_handle.dll");
            PublicDelayLoadDLLs.AddRange(Dlls);
            foreach (var Dll in Dlls)
	            RuntimeDependencies.Add(Path.Combine(DllDirectory, Dll));
        }

        //PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
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
				"RenderCore",
				"RHI",
				"UMG"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
