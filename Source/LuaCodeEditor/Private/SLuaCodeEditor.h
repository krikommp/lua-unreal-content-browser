#pragma once

#include "Widgets/SCompoundWidget.h"

class SLuaCodeEditableText;
class SScrollBar;

class SLuaCodeEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLuaCodeEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, class ULuaCodeProjectItem* InCodeProjectItem);

	bool Save() const;

	bool CanSave() const;

	void GotoLineAndColumn(int32 LineNumber, int32 ColumnNumber);

private:
	void OnTextChanged(const FText& NewText);

protected:
	class ULuaCodeProjectItem* CodeProjectItem;

	TSharedPtr<SScrollBar> HorizontalScrollbar;
	TSharedPtr<SScrollBar> VerticalScrollbar;

	TSharedPtr<class SLuaCodeEditableText> CodeEditableText;

	mutable bool bDirty;
};
