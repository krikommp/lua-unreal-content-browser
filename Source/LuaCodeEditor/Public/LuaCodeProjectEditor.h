#pragma once
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

class FLuaCodeProjectEditorToolbar;
class FDocumentTracker;
class FTabInfo;
class ULuaCodeProject;
class ULuaCodeProjectItem;

class LUACODEEDITOR_API FLuaCodeProjectEditor : public FWorkflowCentricApplication, public FGCObject
{
public:
	FLuaCodeProjectEditor();

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	// End of FAssetEditorToolkit

	// FSerializableObject interface
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override
	{
		return TEXT("FLuaCodeProjectEditor");
	}
	// End of FSerializableObject interface

	static TSharedPtr<FLuaCodeProjectEditor> Get()
	{
		return CodeEditor.Pin();
	}

public:
	/** Initialize the code editor */
	void InitCodeEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, class ULuaCodeProject* CodeProject);

	/** Try to open a new file for editing */
	void OpenFileForEditing(class ULuaCodeProjectItem* Item);

	/** Get the current project being edited by this code editor */
	ULuaCodeProject* GetCodeProjectBeingEdited() const { return LuaCodeProjectBeingEdited.Get(); }

	TSharedRef<SWidget> CreateCodeEditorWidget(TSharedRef<FTabInfo> TabInfo, ULuaCodeProjectItem* Item);

	void RegisterToolbarTab(const TSharedRef<class FTabManager>& TabManager);

	/** Access the toolbar builder for this editor */
	TSharedPtr<class FLuaCodeProjectEditorToolbar> GetToolbarBuilder() { return ToolbarBuilder; }

	bool Save();

	bool SaveAll();

private:
	void BindCommands();

	void Save_Internal();

	void SaveAll_Internal();

	bool CanSave() const;

	bool CanSaveAll() const;
protected:
	TSharedPtr<FDocumentTracker> DocumentManager;

	/** The code project we are currently editing */
	TWeakObjectPtr<ULuaCodeProject> LuaCodeProjectBeingEdited;

	TSharedPtr<class FLuaCodeProjectEditorToolbar> ToolbarBuilder;

	static TWeakPtr<FLuaCodeProjectEditor> CodeEditor;
};
