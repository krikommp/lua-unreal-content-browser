#include "LuaCodeProjectEditorToolbar.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LuaCodeProjectEditorCommands.h"

void FLuaCodeProjectEditorToolbar::AddEditorToolbar(TSharedPtr<FExtender> Extender)
{
	check(CodeProjectEditor.IsValid());
	TSharedPtr<FLuaCodeProjectEditor> CodeProjectEditorPtr = CodeProjectEditor.Pin();

	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		CodeProjectEditorPtr->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP( this, &FLuaCodeProjectEditorToolbar::FillEditorToolbar ) );
}

void FLuaCodeProjectEditorToolbar::FillEditorToolbar(FToolBarBuilder& ToolbarBuilder)
{
	TSharedPtr<FLuaCodeProjectEditor> CodeProjectEditorPtr = CodeProjectEditor.Pin();

	ToolbarBuilder.BeginSection(TEXT("FileManagement"));
	{
		ToolbarBuilder.AddToolBarButton(FLuaCodeProjectEditorCommands::Get().Save);
		ToolbarBuilder.AddToolBarButton(FLuaCodeProjectEditorCommands::Get().SaveAll);
	}
	ToolbarBuilder.EndSection();
}
