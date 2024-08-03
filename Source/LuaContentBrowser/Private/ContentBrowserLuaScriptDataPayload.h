#pragma once
#include "ContentBrowserItemData.h"
#include "AssetRegistry/AssetData.h"

class FAssetThumbnail;
class ULuaFile;

class LUACONTENTBROWSER_API FContentBrowserLuaScriptFolderItemDataPayload : public IContentBrowserItemDataPayload
{
public:
	explicit FContentBrowserLuaScriptFolderItemDataPayload(const FName InInternalPath)
		: InternalPath(InInternalPath)
	{
	}

	FName GetInternalPath() const
	{
		return InternalPath;
	}

	const FString& GetFilename() const;

private:
	FName InternalPath;

	mutable bool bHasCachedFilename = false;
	mutable FString CachedFilename;
};

class LUACONTENTBROWSER_API FContentBrowserLuaScriptFileItemDataPayload : public IContentBrowserItemDataPayload
{
public:
	FContentBrowserLuaScriptFileItemDataPayload(const FName InInternalPath, ULuaFile* InScript);

	FName GetInternalPath() const
	{
		return InternalPath;
	}

	ULuaFile* GetScript() const
	{
		return Script.Get();
	}

	const FAssetData& GetAssetData() const
	{
		return AssetData;
	}

	const FString& GetFilename() const;

	void UpdateThumbnail(FAssetThumbnail& InThumbnail) const;

private:
	FName InternalPath;

	TWeakObjectPtr<ULuaFile> Script;

	FAssetData AssetData;

	mutable bool bHasCachedFilename = false;
	mutable FString CachedFilename;
};