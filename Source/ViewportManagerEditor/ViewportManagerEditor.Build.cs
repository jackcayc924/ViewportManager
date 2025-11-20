// Copyright jackcayc924 2025. All Rights Reserved.

using UnrealBuildTool;

public class ViewportManagerEditor : ModuleRules
{
	public ViewportManagerEditor(ReadOnlyTargetRules Target) : base(Target)
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
				"ViewportManager",
				"UMG",
				"Blutility",
				"EnhancedInput"
			}
		);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"EditorWidgets",
				"UnrealEd",
				"ToolMenus",
				"Slate",
				"SlateCore",
				"PropertyEditor",
				"AssetTools",
				"ContentBrowser",
				"UMGEditor",
				"BlueprintGraph",
				"KismetCompiler",
				"EditorSubsystem",
				"InputCore",
				"Projects"
			}
		);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}

