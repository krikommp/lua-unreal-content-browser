#include "LuaEditorStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FLuaEditorStyle::StyleInstance = nullptr;
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush( StyleInstance->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon32x32(32.0f, 32.0f);
const FVector2D Icon128x128(128.0f, 128.0f);

void FLuaEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();

		AddBrushManually();

		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FLuaEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FLuaEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(LUA_EDITOR_STYLE);
	return StyleSetName;
}

void FLuaEditorStyle::AddBrushManually()
{
	StyleInstance->Set("LuaThumbnail.LuaScript", new IMAGE_BRUSH(TEXT("IconResources/Lua@256"), Icon128x128));
	StyleInstance->Set("LuaIcon.LuaScript", new IMAGE_BRUSH(TEXT("IconResources/Lua@128"), Icon16x16));
}

TSharedRef<FSlateStyleSet> FLuaEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(LUA_EDITOR_STYLE));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("LuaContentBrowser")->GetBaseDir() / TEXT("Resources"));

	return Style;
}


void FLuaEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FLuaEditorStyle::Get()
{
	return *StyleInstance;
}