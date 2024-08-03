// Copyright Epic Games, Inc. All Rights Reserved.

#include "LuaContentBrowserModule.h"

#include "AssetToolsModule.h"
#include "AssetTypeActions_LuaFile.h"
#include "IAssetTools.h"
#include "LuaContentBrowserDataSource.h"
#include "LuaEditorStyle.h"
#include "LuaCodeEditor/LuaCodeProjectEditor.h"

static const FName LuaCodeEditorTabName( TEXT( "LuaCodeEditor" ) );

#define LOCTEXT_NAMESPACE "LuaContentBrowserModule"

TSharedRef<SDockTab> FLuaContentBrowserModule::SpawnLuaCodeEditorTab(const FSpawnTabArgs& TagArgs)
{
	TSharedRef<FLuaCodeProjectEditor> NewLuaCodeProjectEditor(new FLuaCodeProjectEditor());
	// NewLuaCodeProjectEditor->InitCodeEditor(EToolkitMode::Standalone, TSharedPtr<class IToolkitHost>(), GetMutableDefault<UCodeProject>());

	return FGlobalTabmanager::Get()->GetMajorTabForTabManager(NewLuaCodeProjectEditor->GetTabManager().ToSharedRef()).ToSharedRef();
}

void FLuaContentBrowserModule::OpenLuaCodeEditor()
{
	SpawnLuaCodeEditorTab(FSpawnTabArgs(TSharedPtr<SWindow>(), FTabId()));
}

void FLuaContentBrowserModule::StartupModule()
{
	FLuaEditorStyle::Initialize();
	ClassDataSource.Reset(NewObject<ULuaContentBrowserDataSource>(GetTransientPackage(), "LuaData"));
	ClassDataSource->Initialize();

	// 注册菜单
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	LuaFileAssetTypeActions = MakeShareable(new FAssetTypeActions_LuaFile());
	AssetTools.RegisterAssetTypeActions(LuaFileAssetTypeActions.ToSharedRef());

	FGlobalTabmanager::Get()->RegisterTabSpawner( LuaCodeEditorTabName, FOnSpawnTab::CreateStatic( &FLuaContentBrowserModule::SpawnLuaCodeEditorTab) )
		.SetDisplayName( LOCTEXT( "LuaCodeEditorTabTitle", "Edit Source Code" ) )
		.SetTooltipText( LOCTEXT( "LuaCodeEditorTooltipText", "Open the Code Editor tab." ) )
		.SetIcon(FSlateIcon(FLuaEditorStyle::Get().GetStyleSetName(), "LuaIcon.LuaScript"));

	FCoreDelegates::OnPostEngineInit.AddRaw( this, &FLuaContentBrowserModule::OnPostEngineInit );
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

void FLuaContentBrowserModule::OnPostEngineInit()
{
	if (UToolMenus::IsToolMenuUIEnabled())
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		FToolMenuSection& Section = Menu->FindOrAddSection("FileProject");

		FToolMenuOwnerScoped OwnerScoped(this);
		{
			FToolMenuEntry& MenuEntry = Section.AddMenuEntry(
				"EditLuaSourceCoed",
				LOCTEXT("LuaCodeEditorTabTitle", "Edit Source Code"),
				LOCTEXT("LuaCodeEditorTooltipText", "Open the Code Editor tab."),
				FSlateIcon(FLuaEditorStyle::Get().GetStyleSetName(), "LuaIcon.LuaScript"),
				FUIAction
				(
					FExecuteAction::CreateStatic(&FLuaContentBrowserModule::OpenLuaCodeEditor)
				)
				);
			MenuEntry.InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::First);
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLuaContentBrowserModule, LuaContentBrowserEditorMode)