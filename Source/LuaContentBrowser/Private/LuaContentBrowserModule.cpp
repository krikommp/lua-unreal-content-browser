// Copyright Epic Games, Inc. All Rights Reserved.

#include "LuaContentBrowserModule.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_LuaFile.h"
#include "IAssetTools.h"
#include "LuaContentBrowserDataSource.h"
#include "LuaEditorStyle.h"

static const FName LuaCodeEditorTabName( TEXT( "LuaCodeEditor" ) );

#define LOCTEXT_NAMESPACE "LuaContentBrowserModule"

void FLuaContentBrowserModule::StartupModule()
{
	FLuaEditorStyle::Initialize();
	ClassDataSource.Reset(NewObject<ULuaContentBrowserDataSource>(GetTransientPackage(), "LuaData"));
	ClassDataSource->Initialize();

	// 注册菜单
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	LuaFileAssetTypeActions = MakeShareable(new FAssetTypeActions_LuaFile());
	AssetTools.RegisterAssetTypeActions(LuaFileAssetTypeActions.ToSharedRef());
}

void FLuaContentBrowserModule::ShutdownModule()
{
	ClassDataSource.Reset();
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::Get().GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(LuaFileAssetTypeActions.ToSharedRef());
	}

	FGlobalTabmanager::Get()->UnregisterTabSpawner( LuaCodeEditorTabName );

	FCoreDelegates::OnPostEngineInit.RemoveAll(this);
	UToolMenus::UnregisterOwner(this);

	FLuaEditorStyle::Shutdown();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLuaContentBrowserModule, LuaContentBrowserEditorMode)