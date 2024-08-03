// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LuaContentBrowser : ModuleRules
{
	public LuaContentBrowser(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"AssetTools",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"EditorFramework",
				"EditorStyle",
				"UnrealEd",
				"LevelEditor",
				"InteractiveToolsFramework",
				"EditorInteractiveToolsFramework",
				"Projects",
				"ContentBrowserData",
				"AssetTools",
				"ToolMenus",
				"DirectoryWatcher",
				"LuaCodeEditor"
			}
		);
	}
}