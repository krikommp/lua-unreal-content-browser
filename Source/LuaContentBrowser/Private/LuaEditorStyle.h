#pragma once

#define LUA_EDITOR_STYLE "LuaEditorStyle"

class FLuaEditorStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static void ReloadTextures();

	static const ISlateStyle&Get();

	static FName GetStyleSetName();

private:
	static void AddBrushManually();

	static TSharedRef<class FSlateStyleSet> Create();

	static TSharedPtr<class FSlateStyleSet> StyleInstance;
};
