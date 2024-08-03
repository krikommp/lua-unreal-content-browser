// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class ULuaContentBrowserDataSource;
class FAssetTypeActions_LuaFile;

/**
 * This is the module definition for the editor mode. You can implement custom functionality
 * as your plugin module starts up and shuts down. See IModuleInterface for more extensibility options.
 */
class FLuaContentBrowserModule : public IModuleInterface
{
private:
	static TSharedRef<SDockTab> SpawnLuaCodeEditorTab(const FSpawnTabArgs& TagArgs);
	static void OpenLuaCodeEditor();
	
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual void OnPostEngineInit();

private:
	TStrongObjectPtr<ULuaContentBrowserDataSource> ClassDataSource;
	TSharedPtr<FAssetTypeActions_LuaFile> LuaFileAssetTypeActions;
};
