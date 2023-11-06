#include "AssetTypeActions_LuaFile.h"

#include "LuaFile.h"
#include "SourceCodeNavigation.h"
#include "LuaContentBrowserUtil.h"
#include "LuaEditorStyle.h"

#define LOCTEXT_NAMESPACE "LuaContentBrowser"

FText FAssetTypeActions_LuaFile::GetName() const
{
	return LOCTEXT("LuaFileAssetName", "Lua File");
}

UClass* FAssetTypeActions_LuaFile::GetSupportedClass() const
{
	return ULuaFile::StaticClass();
}

void FAssetTypeActions_LuaFile::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	TArray<FString> FilesToOpen;
	for (UObject* Object : InObjects)
	{
		const ULuaFile* LuaFile = Cast<ULuaFile>(Object);

		FilesToOpen.Add(LUA::ConvertInternalPathToAbsolutePath(LuaFile->EntryPath));
	}
	FSourceCodeNavigation::OpenSourceFiles(FilesToOpen);
}

uint32 FAssetTypeActions_LuaFile::GetCategories()
{
	return EAssetTypeCategories::Basic;
}

const FSlateBrush* FAssetTypeActions_LuaFile::GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const
{
	return FLuaEditorStyle::Get().GetBrush("LuaIcon.LuaScript");
}

const FSlateBrush* FAssetTypeActions_LuaFile::GetThumbnailBrush(const FAssetData& InAssetData,
	const FName InClassName) const
{
	return FLuaEditorStyle::Get().GetBrush("LuaThumbnail.LuaScript");
}

#undef LOCTEXT_NAMESPACE
