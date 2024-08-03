// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class LuaCodeEditor : ModuleRules
{
	public LuaCodeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Public")
				// ... add public include paths required here ...
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(ModuleDirectory, "Private"),
				// ... add other private include paths required here ...
			}
		);

		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"SlateCore",
				"Slate",
				"AssetTools",
				"EditorFramework",
				"UnrealEd",
				"PropertyEditor",
				"Kismet", // for FWorkflowCentricApplication
				"InputCore",
				"DirectoryWatcher",
				"LevelEditor",
				"Engine",
				"ToolMenus",
			}
		);
	}
}