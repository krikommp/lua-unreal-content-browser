#include "LuaCodeProjectEditor.h"

#include "LuaCodeProject.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "WorkflowOrientedApp/ApplicationMode.h"
#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "SLuaCodeEditor.h"
#include "SLuaCodeProjectEditor.h"
#include "LuaCodeEditorStyle.h"
#include "LuaCodeProjectEditorCommands.h"
#include "LuaCodeProjectEditorToolbar.h"

#define LOCTEXT_NAMESPACE "LuaCodeEditor"

TWeakPtr<FLuaCodeProjectEditor> FLuaCodeProjectEditor::CodeEditor;

const FName LuaCodeEditorAppName = FName(TEXT("LuaCodeEditorApp"));

namespace LuaCodeEditorModes
{
	// Mode identifiers
	static const FName StandardMode(TEXT("StandardMode"));
};

namespace LuaCodeEditorTabs
{
	// Tab identifiers
	static const FName ProjectViewID(TEXT("ProjectView"));
	static const FName CodeViewID(TEXT("Document"));
};

struct FLuaCodeTabSummoner : public FDocumentTabFactoryForObjects<ULuaCodeProjectItem>
{
public:
	DECLARE_DELEGATE_RetVal_TwoParams(TSharedRef<SWidget>, FOnCreateCodeEditorWidget, TSharedRef<FTabInfo>, ULuaCodeProjectItem*);

public:
	FLuaCodeTabSummoner(TSharedPtr<class FLuaCodeProjectEditor> InCodeProjectEditorPtr, FOnCreateCodeEditorWidget CreateCodeEditorWidgetCallback)
		: FDocumentTabFactoryForObjects<ULuaCodeProjectItem>(LuaCodeEditorTabs::CodeViewID, InCodeProjectEditorPtr)
		, CodeProjectEditorPtr(InCodeProjectEditorPtr)
		, OnCreateCodeEditorWidget(CreateCodeEditorWidgetCallback)
	{
	}

	virtual void OnTabActivated(TSharedPtr<SDockTab> Tab) const override
	{
		TSharedRef<SLuaCodeEditor> CodeEditor = StaticCastSharedRef<SLuaCodeEditor>(Tab->GetContent());
		//	InCodeProjectEditorPtr.Pin()->OnCodeEditorFocused(CodeEditor);
	}

	virtual void OnTabRefreshed(TSharedPtr<SDockTab> Tab) const override
	{
		TSharedRef<SLuaCodeEditor> GraphEditor = StaticCastSharedRef<SLuaCodeEditor>(Tab->GetContent());
		//	GraphEditor->NotifyItemChanged();
	}

	virtual void SaveState(TSharedPtr<SDockTab> Tab, TSharedPtr<FTabPayload> Payload) const override
	{
		TSharedRef<SLuaCodeEditor> GraphEditor = StaticCastSharedRef<SLuaCodeEditor>(Tab->GetContent());

		//	ULuaCodeProjectItem* Graph = FTabPayload_UObject::CastChecked<ULuaCodeProjectItem>(Payload);
		//	BlueprintEditorPtr.Pin()->GetBlueprintObj()->LastEditedDocuments.Add(FEditedDocumentInfo(Graph, ViewLocation, ZoomAmount));
	}
protected:
	virtual TAttribute<FText> ConstructTabNameForObject(ULuaCodeProjectItem* DocumentID) const override
	{
		return FText::FromString(DocumentID->Name);
	}

	virtual TSharedRef<SWidget> CreateTabBodyForObject(const FWorkflowTabSpawnInfo& Info, ULuaCodeProjectItem* DocumentID) const override
	{
		check(Info.TabInfo.IsValid());
		return OnCreateCodeEditorWidget.Execute(Info.TabInfo.ToSharedRef(), DocumentID);
	}

	virtual const FSlateBrush* GetTabIconForObject(const FWorkflowTabSpawnInfo& Info, ULuaCodeProjectItem* DocumentID) const override
	{
		return FLuaCodeEditorStyle::Get().GetBrush("ProjectEditor.Icon.File");
	}

	/*	virtual TSharedRef<FGenericTabHistory> CreateTabHistoryNode(TSharedPtr<FTabPayload> Payload) override
		{
			return MakeShareable(new FSourceTabHistory(SharedThis(this), Payload));
		}*/

protected:
	TWeakPtr<class FLuaCodeProjectEditor> CodeProjectEditorPtr;
	FOnCreateCodeEditorWidget OnCreateCodeEditorWidget;
};

struct FLuaProjectViewSummoner : public FWorkflowTabFactory
{
public:
	FLuaProjectViewSummoner(TSharedPtr<class FAssetEditorToolkit> InHostingApp)
		: FWorkflowTabFactory(LuaCodeEditorTabs::ProjectViewID, InHostingApp)
	{
		TabLabel = LOCTEXT("LuaProjectTabLabel", "LuaProject");

		bIsSingleton = true;

		ViewMenuDescription = LOCTEXT("LuaProjectTabMenu_Description", "LuaProject");
		ViewMenuTooltip = LOCTEXT("LuaProjectTabMenu_ToolTip", "Shows the Lua project panel");
	}

	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override
	{
		TSharedPtr<FLuaCodeProjectEditor> CodeEditorPtr = StaticCastSharedPtr<FLuaCodeProjectEditor>(HostingApp.Pin());
		return SNew(SLuaCodeProjectEditor, CodeEditorPtr->GetCodeProjectBeingEdited());
	}
};

class FBasicCodeEditorMode : public FApplicationMode
{
public:
	FBasicCodeEditorMode(TSharedPtr<class FLuaCodeProjectEditor> InCodeEditor, FName InModeName);

	// FApplicationMode interface
	virtual void RegisterTabFactories(TSharedPtr<FTabManager> InTabManager) override;
	// End of FApplicationMode interface

protected:
	TWeakPtr<FLuaCodeProjectEditor> MyCodeEditor;
	FWorkflowAllowedTabSet TabFactories;
};

FBasicCodeEditorMode::FBasicCodeEditorMode(TSharedPtr<class FLuaCodeProjectEditor> InCodeEditor, FName InModeName)
	: FApplicationMode(InModeName)
{
	MyCodeEditor = InCodeEditor;

	TabFactories.RegisterFactory(MakeShareable(new FLuaProjectViewSummoner(InCodeEditor)));

	TabLayout = FTabManager::NewLayout("Standalone_CodeEditor_Layout_v1.2")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()
				->SetSizeCoefficient(0.9f)
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->SetHideTabWell(true)
					->AddTab(LuaCodeEditorTabs::ProjectViewID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.8f)
					->SetHideTabWell(false)
					->AddTab(LuaCodeEditorTabs::CodeViewID, ETabState::ClosedTab)
				)
			)
		);

	InCodeEditor->GetToolbarBuilder()->AddEditorToolbar(ToolbarExtender);
}

void FBasicCodeEditorMode::RegisterTabFactories(TSharedPtr<FTabManager> InTabManager)
{
	TSharedPtr<FLuaCodeProjectEditor> Editor = MyCodeEditor.Pin();
	
	Editor->RegisterToolbarTab(InTabManager.ToSharedRef());

	Editor->PushTabFactories(TabFactories);

	FApplicationMode::RegisterTabFactories(InTabManager);
}

FLuaCodeProjectEditor::FLuaCodeProjectEditor()
{
}

void FLuaCodeProjectEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	DocumentManager->SetTabManager(InTabManager);

	FWorkflowCentricApplication::RegisterTabSpawners(InTabManager);
}

void FLuaCodeProjectEditor::RegisterToolbarTab(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
}

void FLuaCodeProjectEditor::InitCodeEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, ULuaCodeProject* CodeProject)
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseOtherEditors(CodeProject, this);
	LuaCodeProjectBeingEdited = CodeProject;

	TSharedPtr<FLuaCodeProjectEditor> ThisPtr(SharedThis(this));
	if (!DocumentManager.IsValid())
	{
		DocumentManager = MakeShareable(new FDocumentTracker);
		DocumentManager->Initialize(ThisPtr);
	}

	CodeEditor = ThisPtr;

	TSharedRef<FDocumentTabFactory> GraphEditorFactory = MakeShareable(new FLuaCodeTabSummoner(ThisPtr, FLuaCodeTabSummoner::FOnCreateCodeEditorWidget::CreateSP(this, &FLuaCodeProjectEditor::CreateCodeEditorWidget)));
	DocumentManager->RegisterDocumentFactory(GraphEditorFactory);

	if (!ToolbarBuilder.IsValid())
	{
		ToolbarBuilder = MakeShareable(new FLuaCodeProjectEditorToolbar(SharedThis(this)));
	}

	FLuaCodeProjectEditorCommands::Register();

	// Initialize the asset editor and spawn nothing (dummy layout)
	InitAssetEditor(Mode, InitToolkitHost, LuaCodeEditorAppName, FTabManager::FLayout::NullLayout, /*bCreateDefaultStandaloneMenu=*/ true, /*bCreateDefaultToolbar=*/ true, CodeProject);

	BindCommands();
	
	// Create the modes and activate one (which will populate with a real layout)
	AddApplicationMode(
		LuaCodeEditorModes::StandardMode, 
		MakeShareable(new FBasicCodeEditorMode(ThisPtr, LuaCodeEditorModes::StandardMode)));
	SetCurrentMode(LuaCodeEditorModes::StandardMode);

	RegenerateMenusAndToolbars();
}

void FLuaCodeProjectEditor::BindCommands()
{
	ToolkitCommands->MapAction(FLuaCodeProjectEditorCommands::Get().Save,
			FExecuteAction::CreateSP(this, &FLuaCodeProjectEditor::Save_Internal),
			FCanExecuteAction::CreateSP(this, &FLuaCodeProjectEditor::CanSave)
			);

	ToolkitCommands->MapAction(FLuaCodeProjectEditorCommands::Get().SaveAll,
			FExecuteAction::CreateSP(this, &FLuaCodeProjectEditor::SaveAll_Internal),
			FCanExecuteAction::CreateSP(this, &FLuaCodeProjectEditor::CanSaveAll)
			);
}

void FLuaCodeProjectEditor::OpenFileForEditing(ULuaCodeProjectItem* Item)
{
	TSharedRef<FTabPayload_UObject> Payload = FTabPayload_UObject::Make(Item);
	DocumentManager->OpenDocument(Payload, FDocumentTracker::OpenNewDocument);
}

FName FLuaCodeProjectEditor::GetToolkitFName() const
{
	return FName("LuaCodeEditor");
}

FText FLuaCodeProjectEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Lua Code Editor");
}

FText FLuaCodeProjectEditor::GetToolkitName() const
{
	return LOCTEXT("CodeAppToolkitName", "Lua Code Editor");
}

FText FLuaCodeProjectEditor::GetToolkitToolTipText() const
{
	return LOCTEXT("CodeAppLabel", "Lua Code Editor");
}

FLinearColor FLuaCodeProjectEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

FString FLuaCodeProjectEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("LuaCodeEditor");
}

void FLuaCodeProjectEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(LuaCodeProjectBeingEdited);
}

TSharedRef<SWidget> FLuaCodeProjectEditor::CreateCodeEditorWidget(TSharedRef<FTabInfo> TabInfo, ULuaCodeProjectItem* Item)
{
	return SNew(SLuaCodeEditor, Item);
}

void FLuaCodeProjectEditor::Save_Internal()
{
	Save();
}

bool FLuaCodeProjectEditor::Save()
{
	if(DocumentManager.IsValid() && DocumentManager->GetActiveTab().IsValid())
	{
		TSharedRef<SLuaCodeEditor> CodeEditorRef = StaticCastSharedRef<SLuaCodeEditor>(DocumentManager->GetActiveTab()->GetContent());
		return CodeEditorRef->Save();
	}

	return false;
}

bool FLuaCodeProjectEditor::CanSave() const
{
	if(DocumentManager.IsValid() && DocumentManager->GetActiveTab().IsValid())
	{
		TSharedRef<SWidget> Content = DocumentManager->GetActiveTab()->GetContent();
		TSharedRef<SLuaCodeEditor> CodeEditorRef = StaticCastSharedRef<SLuaCodeEditor>(Content);
		return CodeEditorRef->CanSave();
	}

	return false;
}

void FLuaCodeProjectEditor::SaveAll_Internal()
{
	SaveAll();
}

bool FLuaCodeProjectEditor::SaveAll()
{
	bool bResult = true;

	if(DocumentManager.IsValid())
	{
		TArray<TSharedPtr<SDockTab>> AllTabs = DocumentManager->GetAllDocumentTabs();
		for(auto& Tab : AllTabs)
		{
			if(Tab.IsValid())
			{
				TSharedRef<SLuaCodeEditor> CodeEditorRef = StaticCastSharedRef<SLuaCodeEditor>(Tab->GetContent());
				if(!CodeEditorRef->Save())
				{
					bResult = false;
				}
			}
		}
	}

	return bResult;
}

bool FLuaCodeProjectEditor::CanSaveAll() const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
