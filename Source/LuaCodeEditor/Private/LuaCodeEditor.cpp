#include "Misc/CoreDelegates.h"
#include "ToolMenus.h"
#include "LuaCodeEditorStyle.h"
#include "LuaCodeProject.h"
#include "LuaCodeProjectEditor.h"
#include "ToolMenu.h"
#include "ToolMenuEntry.h"
#include "ToolMenuSection.h"

static const FName LuaCodeEditorTabName( TEXT( "LuaCodeEditor" ) );

#define LOCTEXT_NAMESPACE "LuaCodeEditor"

class FLuaCodeEditor : public IModuleInterface
{
private:
	static TSharedRef<SDockTab> SpawnCodeEditorTab(const FSpawnTabArgs& TabArgs)
	{
		TSharedRef<FLuaCodeProjectEditor> NewCodeProjectEditor(new FLuaCodeProjectEditor());
		NewCodeProjectEditor->InitCodeEditor(EToolkitMode::Standalone, TSharedPtr<class IToolkitHost>(), GetMutableDefault<ULuaCodeProject>());

		return FGlobalTabmanager::Get()->GetMajorTabForTabManager(NewCodeProjectEditor->GetTabManager().ToSharedRef()).ToSharedRef();
	}

	static void OpenCodeEditor()
	{
		SpawnCodeEditorTab(FSpawnTabArgs(TSharedPtr<SWindow>(), FTabId()));	
	}

public:
	virtual void OnPostEngineInit()
	{
		if (UToolMenus::IsToolMenuUIEnabled())
		{
			UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
			FToolMenuSection& Section = Menu->FindOrAddSection("FileProject");

			FToolMenuOwnerScoped OwnerScoped(this);
			{
				FToolMenuEntry& MenuEntry = Section.AddMenuEntry(
					"EditLuaCode",
					LOCTEXT("LuaCodeEditorTabTitle", "Edit Lua Code"),
					LOCTEXT("LuaCodeEditorTooltipText", "Open the Code Editor tab."),
					FSlateIcon(FLuaCodeEditorStyle::Get().GetStyleSetName(), "CodeEditor.TabIcon"),
					FUIAction
					(
						FExecuteAction::CreateStatic(&FLuaCodeEditor::OpenCodeEditor)
					)
				);
				MenuEntry.InsertPosition = FToolMenuInsert(NAME_None, EToolMenuInsertType::First);
			}
		}
	}

	virtual void StartupModule() override
	{
		FLuaCodeEditorStyle::Initialize();
		
		// Register a tab spawner so that our tab can be automatically restored from layout files
		FGlobalTabmanager::Get()->RegisterTabSpawner( LuaCodeEditorTabName, FOnSpawnTab::CreateStatic( &FLuaCodeEditor::SpawnCodeEditorTab ) )
				.SetDisplayName( LOCTEXT( "LuaCodeEditorTabTitle", "Edit Lua Code" ) )
				.SetTooltipText( LOCTEXT( "LuaCodeEditorTooltipText", "Open the Code Editor tab." ) )
				.SetIcon(FSlateIcon(FLuaCodeEditorStyle::Get().GetStyleSetName(), "CodeEditor.TabIcon"));

		FCoreDelegates::OnPostEngineInit.AddRaw(this, &FLuaCodeEditor::OnPostEngineInit);
	}


	virtual void ShutdownModule() override
	{
		// Unregister the tab spawner
		FGlobalTabmanager::Get()->UnregisterTabSpawner( LuaCodeEditorTabName );

		FCoreDelegates::OnPostEngineInit.RemoveAll(this);
		UToolMenus::UnregisterOwner(this);

		FLuaCodeEditorStyle::Shutdown();
	}
};

IMPLEMENT_MODULE( FLuaCodeEditor, LuaCodeEditor )

#undef LOCTEXT_NAMESPACE