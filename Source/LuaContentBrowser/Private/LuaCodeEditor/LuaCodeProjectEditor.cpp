#include "LuaCodeProjectEditor.h"
#include "LuaCodeProject.h"

#define LOCTEXT_NAMESPACE "LuaCodeEditor"

FLuaCodeProjectEditor::FLuaCodeProjectEditor()
{
}

void FLuaCodeProjectEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FWorkflowCentricApplication::RegisterTabSpawners(InTabManager);
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

FString FLuaCodeProjectEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("LuaCodeEditor");
}

FLinearColor FLuaCodeProjectEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}

void FLuaCodeProjectEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(CodeProjectBeingEdited);
}


//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE