#include "SLuaCodeProjectEditor.h"
#include "LuaCodeEditorStyle.h"
#include "LuaCodeProject.h"
#include "LuaCodeProjectEditor.h"
#include "Framework/Views/TableViewMetadata.h"
#include "SLuaProjectViewItem.h"
#include "DirectoryScanner.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Views/STreeView.h"

#define LOCTEXT_NAMESPACE "LuaCodeProjectEditor"

void SLuaCodeProjectEditor::Construct(const FArguments& InArgs, ULuaCodeProject* InCodeProject)
{
	check(InCodeProject);
	CodeProject = InCodeProject;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FLuaCodeEditorStyle::Get().GetBrush("ProjectEditor.Border"))
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			[
				SAssignNew(ProjectTree, STreeView<ULuaCodeProjectItem*>)
				.TreeItemsSource(&ToRawPtrTArrayUnsafe(CodeProject->Children))
				.OnGenerateRow(this, &SLuaCodeProjectEditor::OnGenerateRow)
				.OnGetChildren(this, &SLuaCodeProjectEditor::OnGetChildren)
				.OnMouseButtonDoubleClick(this, &SLuaCodeProjectEditor::HandleMouseButtonDoubleClick)
			]
			+SOverlay::Slot()
			.VAlign(VAlign_Bottom)
			.Padding(10.0f)
			[
				SNew(SThrobber)
				.Visibility(this, &SLuaCodeProjectEditor::GetThrobberVisibility)
			]
		]
	];

	InCodeProject->RescanChildren();
}

void SLuaCodeProjectEditor::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if(FDirectoryScanner::Tick())
	{
		ProjectTree->SetTreeItemsSource(&ToRawPtrTArrayUnsafe(CodeProject->Children));
	}

	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

FName SLuaCodeProjectEditor::GetIconForItem(ULuaCodeProjectItem* Item) const
{
	switch(Item->Type)
	{
	case ELuaCodeProjectItemType::Project:
		return "ProjectEditor.Icon.Project";
	case ELuaCodeProjectItemType::Folder:
		return "ProjectEditor.Icon.Folder";
	case ELuaCodeProjectItemType::File:
		return "ProjectEditor.Icon.File";
	default:
		return "ProjectEditor.Icon.GenericFile";
	}
}

TSharedRef<ITableRow> SLuaCodeProjectEditor::OnGenerateRow(ULuaCodeProjectItem* Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(STableRow<ULuaCodeProjectItem*>, OwnerTable)
		[
			SNew(SLuaProjectViewItem)
			.Text(FText::FromString(Item->Name))
			.IconName(GetIconForItem(Item))
		];
}

void SLuaCodeProjectEditor::OnGetChildren(ULuaCodeProjectItem* Item, TArray<ULuaCodeProjectItem*>& OutChildItems)
{
	OutChildItems = Item->Children;
}

EVisibility SLuaCodeProjectEditor::GetThrobberVisibility() const
{
	return FDirectoryScanner::IsScanning() ? EVisibility::Visible : EVisibility::Hidden; 
}

void SLuaCodeProjectEditor::HandleMouseButtonDoubleClick(ULuaCodeProjectItem* Item) const
{
	if(Item->Type == ELuaCodeProjectItemType::File)
	{
		FLuaCodeProjectEditor::Get()->OpenFileForEditing(Item);
	}
}


#undef LOCTEXT_NAMESPACE
