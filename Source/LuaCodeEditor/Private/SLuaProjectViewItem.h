#pragma once

#include "Widgets/SCompoundWidget.h"

class SLuaProjectViewItem : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLuaProjectViewItem) {}

	SLATE_ARGUMENT(FName, IconName)

	SLATE_ARGUMENT(FText, Text)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};
