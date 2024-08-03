#include "LuaCodeProjectEditorCommands.h"
#include "LuaCodeEditorStyle.h"


#define LOCTEXT_NAMESPACE "LuaCodeProjectEditorCommands"


FLuaCodeProjectEditorCommands::FLuaCodeProjectEditorCommands() 
	: TCommands<FLuaCodeProjectEditorCommands>("CodeEditor", LOCTEXT("General", "General"), NAME_None, FLuaCodeEditorStyle::GetStyleSetName())
{
}


void FLuaCodeProjectEditorCommands::RegisterCommands()
{
	UI_COMMAND(Save, "Save", "Save the currently active document.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::S));
	UI_COMMAND(SaveAll, "Save All", "Save all open documents.", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::S));
}


#undef LOCTEXT_NAMESPACE
