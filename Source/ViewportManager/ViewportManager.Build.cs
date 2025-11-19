// Copyright jackcayc924 2025. All Rights Reserved.

using UnrealBuildTool;

public class ViewportManager : ModuleRules
{
	public ViewportManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
		);
				
		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);
			
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"UMG",
				"Slate",
				"SlateCore",
				"InputCore",
				"EnhancedInput",
				"DeveloperSettings"
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}

