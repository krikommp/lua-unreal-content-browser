#pragma once
#include "ContentBrowserDataSource.h"
#include "ContentBrowserItemData.h"
#include "ContentBrowserLuaScriptDataPayload.h"

class IAssetTypeActions;
class UContentBrowserDataSource;
class FContentBrowserItemData;
class FContentBrowserLuaScriptFileItemDataPayload;
class FContentBrowserLuaScriptFolderItemDataPayload;
class UContentBrowserDataSource;

namespace ContentBrowserLuaScriptData
{
	TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload> GetScriptFolderItemPayload(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem);
	TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> GetScriptFileItemDataPayload(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem);
	bool EditItem(IAssetTypeActions* InClassTypeActions, const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem);
	bool EditScriptFileItem(IAssetTypeActions* InScriptTypeActions, TSharedRef<const FContentBrowserLuaScriptFileItemDataPayload> InScriptPayload);
	bool GetItemAttribute(IAssetTypeActions* InScriptTypeActions, const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue);
	bool GetItemAttributes(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const bool InIncludeMetaData, FContentBrowserItemDataAttributeValues& OutAttributeValues);
	bool GetScriptFolderItemAttribute(const FContentBrowserLuaScriptFolderItemDataPayload& InFolderPayload, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue);bool GetScriptFileItemAttribute(IAssetTypeActions* InScriptTypeActions, const FContentBrowserLuaScriptFileItemDataPayload& InScriptPayload, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue);
	bool GetItemPhysicalPath(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FString& OutDiskPath);
	bool UpdateItemThumbnail(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FAssetThumbnail& InThumbnail);
	bool CanRenameItem(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const FString* InNewName, FText* OutErrorMsg);
	bool RenameItem(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const FString& InItemsNewName);
	bool MoveItem(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const FName InDestPath);
	bool DeleteItems(const UContentBrowserDataSource* InOwnerDataSource, TArrayView<const FContentBrowserItemData> InItems);
}
