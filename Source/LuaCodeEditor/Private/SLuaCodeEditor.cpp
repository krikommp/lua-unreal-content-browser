// Copyright Epic Games, Inc. All Rights Reserved.

#include "SLuaCodeEditor.h"
#include "Misc/FileHelper.h"
#include "Widgets/Layout/SGridPanel.h"
#include "LuaCodeProjectItem.h"
#include "LuaRichTextSyntaxHighlighterTextLayoutMarshaller.h"
#include "SLuaCodeEditableText.h"


#define LOCTEXT_NAMESPACE "LuaCodeEditor"

FString ConvertVirtualPathToDiskPath(const FString& VirtualPath)
{
	// 确保路径以 "/Game/" 开头
	FString CorrectedPath = VirtualPath;
	if (!CorrectedPath.StartsWith(TEXT("/Game")))
	{
		return CorrectedPath;
	}

	// 去掉 "/Game" 前缀并添加到内容目录路径
	FString RelativeContentPath = CorrectedPath.RightChop(5); // 去掉 "/Game"
	FString FullPath = FPaths::ProjectContentDir() + RelativeContentPath;

	return FullPath;
}

void SLuaCodeEditor::Construct(const FArguments& InArgs, ULuaCodeProjectItem* InLuaCodeProjectItem)
{
	bDirty = false;

	check(InLuaCodeProjectItem);
	CodeProjectItem = InLuaCodeProjectItem;

	FString FileText = "File Loading, please wait";
	FString FullPath = ConvertVirtualPathToDiskPath(*InLuaCodeProjectItem->Path);
	FFileHelper::LoadFileToString(FileText, *FullPath);

	TSharedRef<FLuaRichTextSyntaxHighlighterTextLayoutMarshaller> RichTextMarshaller = FLuaRichTextSyntaxHighlighterTextLayoutMarshaller::Create(
			FLuaRichTextSyntaxHighlighterTextLayoutMarshaller::FSyntaxTextStyle()
			);

	HorizontalScrollbar = 
		SNew(SScrollBar)
		.Orientation(Orient_Horizontal)
		.Thickness(FVector2D(14.0f, 14.0f));

	VerticalScrollbar = 
		SNew(SScrollBar)
		.Orientation(Orient_Vertical)
		.Thickness(FVector2D(14.0f, 14.0f));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FLuaCodeEditorStyle::Get().GetBrush("TextEditor.Border"))
		[
			SNew(SGridPanel)
			.FillColumn(0, 1.0f)
			.FillRow(0, 1.0f)
			+SGridPanel::Slot(0, 0)
			[
				SAssignNew(CodeEditableText, SLuaCodeEditableText)
				.Text(FText::FromString(FileText))
				.Marshaller(RichTextMarshaller)
				.HScrollBar(HorizontalScrollbar)
				.VScrollBar(VerticalScrollbar)
				.OnTextChanged(this, &SLuaCodeEditor::OnTextChanged)
			]
			+SGridPanel::Slot(1, 0)
			[
				VerticalScrollbar.ToSharedRef()
			]
			+SGridPanel::Slot(0, 1)
			[
				HorizontalScrollbar.ToSharedRef()
			]
		]
	];
}

void SLuaCodeEditor::OnTextChanged(const FText& NewText)
{
	bDirty = true;
}

bool SLuaCodeEditor::Save() const
{
	if(bDirty)
	{
		bool bResult = FFileHelper::SaveStringToFile(CodeEditableText->GetText().ToString(), *CodeProjectItem->Path);
		if(bResult)
		{
			bDirty = false;
		}

		return bResult;
	}
	return true;
}

bool SLuaCodeEditor::CanSave() const
{
	return bDirty;
}

void SLuaCodeEditor::GotoLineAndColumn(int32 LineNumber, int32 ColumnNumber)
{
	FTextLocation Location(LineNumber, ColumnNumber);
	CodeEditableText->GoTo(Location);
	CodeEditableText->ScrollTo(Location);
}

#undef LOCTEXT_NAMESPACE
