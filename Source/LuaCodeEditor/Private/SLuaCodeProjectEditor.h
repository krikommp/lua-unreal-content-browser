#pragma once

#include "Widgets/SCompoundWidget.h"

template <typename ItemType> class STreeView;

class SLuaCodeProjectEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLuaCodeProjectEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, class ULuaCodeProject* InCodeProject);

private:
	/** Begin SWidget interface */
	void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;
	/** End SWidget interface */

	TSharedRef<class ITableRow> OnGenerateRow(class ULuaCodeProjectItem* Item, const TSharedRef<class STableViewBase>& TableView);

	void OnGetChildren(class ULuaCodeProjectItem* Item, TArray<class ULuaCodeProjectItem*>& OutChildItems);

	EVisibility GetThrobberVisibility() const;

	FName GetIconForItem(class ULuaCodeProjectItem* Item) const;

	void HandleMouseButtonDoubleClick(class ULuaCodeProjectItem* Item) const;

private:
	class ULuaCodeProject* CodeProject;

	TSharedPtr<STreeView<class ULuaCodeProjectItem*>> ProjectTree;
};