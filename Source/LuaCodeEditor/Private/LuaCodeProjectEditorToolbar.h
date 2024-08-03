#pragma once

#include "LuaCodeProjectEditor.h"

class FToolBarBuilder;

class FLuaCodeProjectEditorToolbar: public TSharedFromThis<FLuaCodeProjectEditorToolbar>
{
public:
	FLuaCodeProjectEditorToolbar(TSharedPtr<class FLuaCodeProjectEditor> InCodeProjectEditor)
		: CodeProjectEditor(InCodeProjectEditor) {}

	void AddEditorToolbar(TSharedPtr<FExtender> Extender);

private:
	void FillEditorToolbar(FToolBarBuilder& ToolbarBuilder);

protected:
	/** Pointer back to the code editor tool that owns us */
	TWeakPtr<class FLuaCodeProjectEditor> CodeProjectEditor;
};
