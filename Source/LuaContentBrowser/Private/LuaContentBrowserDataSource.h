// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ContentBrowserDataMenuContexts.h"
#include "ContentBrowserDataSource.h"
#include "ContentBrowserLuaScriptData.h"
#include "LuaScriptHierarchy.h"
#include "UObject/Object.h"
#include "LuaContentBrowserDataSource.generated.h"

class FAssetTypeActions_LuaFile;
class FContentBrowserLuaScriptFolderItemDataPayload;
class FContentBrowserLuaScriptFileItemDataPayload;
class FLuaScriptHierarchy;
class ULuaFile;

USTRUCT()
struct FContentBrowserLuaScriptDataFilter
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TSet<TObjectPtr<ULuaFile>> ValidScripts;

	UPROPERTY()
	TSet<FName> ValidFolders;
};

DECLARE_DELEGATE_OneParam(FOnNewScriptRequested, const FName /*SelectedPath*/);
DECLARE_DELEGATE_TwoParams(FOnNewAssetRequested, const FName /*SelectedPath*/, TWeakObjectPtr<UClass> /*FactoryClass*/);

UCLASS()
class LUACONTENTBROWSER_API ULuaContentBrowserDataSource : public UContentBrowserDataSource
{
	GENERATED_BODY()

	FContentBrowserItemData CreateScriptFolderItem(const FName InFolderPath);
	FContentBrowserItemData CreateScriptFileItem(ULuaFile* InFile);
	TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> GetScriptFileItemPayload(const FContentBrowserItemData& InItem) const;
	TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload> GetScriptFolderItemPayload(const FContentBrowserItemData& InItem) const;
	void MakeContextMenu(UToolMenu* Menu, const TArray<FName>& InSelectedScriptPaths, FOnNewScriptRequested InOnNewScriptRequested);
	void OnNewAssetRequested(const FName InPath, UContentBrowserDataMenuContext_AddNewMenu::FOnBeginItemCreation InOnBeginItemCreation);
	void OnScriptHierarchyChanged();
	bool OnValidateItemName(const FContentBrowserItemData& InItem, const FString& InProposedName, FText* OutErrorMsg);
	FContentBrowserItemData OnFinalizeCreateAsset(const FContentBrowserItemData& InItemData, const FString& InProposedName, FText* OutErrorMsg);

public:
	virtual void Initialize(const bool InAutoRegister = true);
	virtual void CompileFilter(const FName InPath, const FContentBrowserDataFilter& InFilter, FContentBrowserDataCompiledFilter& OutCompiledFilter) override;
	virtual void BuildRootPathVirtualTree() override;
	virtual void Tick(const float InDeltaTime) override;
	virtual void EnumerateItemsMatchingFilter(const FContentBrowserDataCompiledFilter& InFilter, TFunctionRef<bool(FContentBrowserItemData&&)> InCallback) override;
	virtual void EnumerateItemsAtPath(const FName InPath, const EContentBrowserItemTypeFilter InItemTypeFilter, TFunctionRef<bool(FContentBrowserItemData&&)> InCallback) override;
	virtual bool EnumerateItemsForObjects(const TArrayView<UObject*> InObjects, TFunctionRef<bool(FContentBrowserItemData&&)> InCallback) override;
	virtual bool GetItemAttribute(const FContentBrowserItemData& InItem, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue) override;
	virtual bool GetItemAttributes(const FContentBrowserItemData& InItem, const bool InIncludeMetaData, FContentBrowserItemDataAttributeValues& OutAttributeValues) override;
	virtual bool GetItemPhysicalPath(const FContentBrowserItemData& InItem, FString& OutDiskPath) override;
	virtual bool UpdateThumbnail(const FContentBrowserItemData& InItem, FAssetThumbnail& InThumbnail) override;
	virtual bool CanEditItem(const FContentBrowserItemData& InItem, FText* OutErrorMsg) override;
	virtual bool EditItem(const FContentBrowserItemData& InItem) override;
	virtual bool CanRenameItem(const FContentBrowserItemData& InItem, const FString* InNewName, FText* OutErrorMsg) override;
	virtual bool RenameItem(const FContentBrowserItemData& InItem, const FString& InNewName, FContentBrowserItemData& OutNewItem) override;
	virtual bool CanMoveItem(const FContentBrowserItemData& InItem, const FName InDestPath, FText* OutErrorMsg) override;
	virtual bool MoveItem(const FContentBrowserItemData& InItem, const FName InDestPath) override;
	virtual bool CanDeleteItem(const FContentBrowserItemData& InItem, FText* OutErrorMsg) override;
	virtual bool DeleteItem(const FContentBrowserItemData& InItem) override;
	virtual bool BulkDeleteItems(TArrayView<const FContentBrowserItemData> InItems) override;

	static void ExecuteNewClass(FName InPath, FOnNewScriptRequested InOnNewScriptRequested);
private:
	void PopulateAddNewContextMenu(UToolMenu* InMenu);

	TSharedPtr<FLuaScriptHierarchy> LuaScriptHierarchy;
	TArray<FString> RootContentPaths;
	TSharedPtr<FAssetTypeActions_LuaFile> FileTypeAction;
};