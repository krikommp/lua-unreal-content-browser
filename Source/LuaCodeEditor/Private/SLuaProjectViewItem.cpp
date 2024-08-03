#include "SLuaProjectViewItem.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "LuaCodeEditorStyle.h"


#define LOCTEXT_NAMESPACE "LuaProjectViewItem"


void SLuaProjectViewItem::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(1.0f)
		.AutoWidth()
		[
			SNew(SImage)
			.Image(FLuaCodeEditorStyle::Get().GetBrush(InArgs._IconName))
		]
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(1.0f)
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(InArgs._Text)
		]
	];
}


#undef LOCTEXT_NAMESPACE
