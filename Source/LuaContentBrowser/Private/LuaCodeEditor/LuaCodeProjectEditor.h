#pragma once
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

class FDocumentTracker;
class ULuaCodeProject;

class FLuaCodeProjectEditor : public FWorkflowCentricApplication, public FGCObject
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

	

private:
	TSharedRef<FDocumentTracker> DocumentManager;

	TWeakObjectPtr<ULuaCodeProject> CodeProjectBeingEdited;

	static TWeakPtr<FLuaCodeProjectEditor> CodeEditor;
};
