#include "ContentBrowserLuaScriptDataPayload.h"

#include "IAssetTypeActions.h"
#include "LuaContentBrowserUtil.h"
#include "SourceCodeNavigation.h"
#include "LuaFile.h"
#include "AssetToolsModule.h"

FContentBrowserLuaScriptFileItemDataPayload::FContentBrowserLuaScriptFileItemDataPayload(const FName InInternalPath,ULuaFile* InScript)
	: InternalPath(InInternalPath), Script(InScript), AssetData(InScript)
{
}

const FString& FContentBrowserLuaScriptFolderItemDataPayload::GetFilename() const
{
	if (!bHasCachedFilename)
	{
		// Split the class path into its component parts
		TArray<FString> ClassPathParts;
		InternalPath.ToString().ParseIntoArray(ClassPathParts, TEXT("/"), true);

		// We need to have at least two sections (a root, and a module name) to be able to resolve a file system path
		if (ClassPathParts.Num() >= 2)
		{
			// Get the base file path to the module, and then append any remaining parts of the class path (as the remaining parts mirror the file system)
			if (FSourceCodeNavigation::FindModulePath(ClassPathParts[1], CachedFilename))
			{
				for (int32 PathPartIndex = 2; PathPartIndex < ClassPathParts.Num(); ++PathPartIndex)
				{
					CachedFilename /= ClassPathParts[PathPartIndex];
				}

				CachedFilename = FPaths::ConvertRelativePathToFull(CachedFilename);
			}
		}

		bHasCachedFilename = true;
	}
	return CachedFilename;
}

const FString& FContentBrowserLuaScriptFileItemDataPayload::GetFilename() const
{
	if (!bHasCachedFilename)
	{
		CachedFilename = LUA::ConvertInternalPathToAbsolutePath(Script->EntryPath);
		bHasCachedFilename = true;
	}
	return CachedFilename;
}

void FContentBrowserLuaScriptFileItemDataPayload::UpdateThumbnail(FAssetThumbnail& InThumbnail) const
{
	UE_LOG(LogTemp, Warning, TEXT("FContentBrowserLuaScriptFileItemDataPayload::UpdateThumbnail"));
	InThumbnail.SetAsset(AssetData);
}
