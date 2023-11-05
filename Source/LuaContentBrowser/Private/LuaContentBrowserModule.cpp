// Copyright Epic Games, Inc. All Rights Reserved.

#include "LuaContentBrowserModule.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_LuaFile.h"
#include "IAssetTools.h"
#include "LuaContentBrowserDataSource.h"
#include "LuaEditorStyle.h"

#define LOCTEXT_NAMESPACE "LuaContentBrowserModule"

void FLuaContentBrowserModule::StartupModule()
{
	FLuaEditorStyle::Initialize();
	ClassDataSource.Reset(NewObject<ULuaContentBrowserDataSource>(GetTransientPackage(), "LuaData"));
	ClassDataSource->Initialize();
}

void FLuaContentBrowserModule::ShutdownModule()
{
	ClassDataSource.Reset();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLuaContentBrowserModule, LuaContentBrowserEditorMode)