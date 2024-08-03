#include "LuaCodeEditorCustomization.h"

ULuaCodeEditorCustomization::ULuaCodeEditorCustomization(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const FLuaCodeEditorControlCustomization& ULuaCodeEditorCustomization::GetControl(const FName& ControlCustomizationName)
{
	static FLuaCodeEditorControlCustomization Default;

	return Default;
}

const FLuaCodeEditorTextCustomization& ULuaCodeEditorCustomization::GetText(const FName& TextCustomizationName)
{
	static FLuaCodeEditorTextCustomization Default;

	return Default;
}
