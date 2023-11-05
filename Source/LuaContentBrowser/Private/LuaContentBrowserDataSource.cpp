#include "LuaContentBrowserDataSource.h"

#include "AssetTypeActions_LuaFile.h"
#include "ContentBrowserDataMenuContexts.h"
#include "LuaScriptHierarchy.h"
#include "ContentBrowserDataUtils.h"
#include "ContentBrowserLuaScriptData.h"
#include "ContentBrowserLuaScriptDataPayload.h"
#include "LuaContentBrowserUtil.h"
#include "LuaFile.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LuaContentBrowserDataSource)
#define LOCTEXT_NAMESPACE "LuaContentBrowserDataSource"

namespace ContentBrowserLuaScriptData
{
	FContentBrowserItemData CreateScriptFolderItem(UContentBrowserDataSource* InOwnerDataSource,
	                                               const FName InVirtualPath, const FName InFolderPath)
	{
		static const FName GameRootPath = "/Classes_Game";
		static const FName EngineRootPath = "/Classes_Engine";

		const FString FolderItemName = FPackageName::GetShortName(InFolderPath);

		FText FolderDisplayNameOverride;
		if (InFolderPath == GameRootPath)
		{
			FolderDisplayNameOverride = LOCTEXT("GameFolderDisplayName", "Lue Scripts");
		}
		else if (InFolderPath == EngineRootPath)
		{
			FolderDisplayNameOverride = LOCTEXT("EngineFolderDisplayName", "Engine Lua Scripts");
		}
		else
		{
			FolderDisplayNameOverride = ContentBrowserDataUtils::GetFolderItemDisplayNameOverride(
				InFolderPath, FolderItemName, true);
		}

		return FContentBrowserItemData(InOwnerDataSource,
		                               EContentBrowserItemFlags::Type_Folder | EContentBrowserItemFlags::Category_Class,
		                               InVirtualPath, *FolderItemName, MoveTemp(FolderDisplayNameOverride),
		                               MakeShared<FContentBrowserLuaScriptFolderItemDataPayload>(InFolderPath));
	}

	FContentBrowserItemData CreateScriptFileItem(UContentBrowserDataSource* InOwnerDataSource,
	                                             const FName InVirtualPath, const FName InScriptPath,
	                                             ULuaFile* InScript)
	{
		return FContentBrowserItemData(InOwnerDataSource,
		                               EContentBrowserItemFlags::Type_File | EContentBrowserItemFlags::Category_Class,
		                               InVirtualPath, InScript->GetFName(), FText(),
		                               MakeShared<FContentBrowserLuaScriptFileItemDataPayload>(InScriptPath, InScript));
	}
}


bool ULuaContentBrowserDataSource::OnValidateItemName(const FContentBrowserItemData& InItem,
	const FString& InProposedName, FText* OutErrorMsg)
{
	return CanRenameItem(InItem, &InProposedName, OutErrorMsg);
}

FContentBrowserItemData ULuaContentBrowserDataSource::OnFinalizeCreateAsset(const FContentBrowserItemData& InItemData,
	const FString& InProposedName, FText* OutErrorMsg)
{
	checkf(InItemData.GetOwnerDataSource() == this, TEXT("ULuaContentBrowserDataSource::OnFinalizeCreateAsset: Item data is not owned by this data source."));
	checkf(EnumHasAllFlags(InItemData.GetItemFlags(), EContentBrowserItemFlags::Type_File | EContentBrowserItemFlags::Category_Class | EContentBrowserItemFlags::Temporary_Creation), TEXT("ULuaContentBrowserDataSource::OnFinalizeCreateAsset: Item data is not a temporary class creation item."));

	IContentBrowserItemDataPayload* IPayload = const_cast<IContentBrowserItemDataPayload*>(InItemData.GetPayload().Get());
	const FContentBrowserLuaScriptFileItemDataPayload* LuaScriptPayload = static_cast<FContentBrowserLuaScriptFileItemDataPayload*>(IPayload);

	LuaScriptHierarchy->ScriptCreate(LuaScriptPayload->GetScript()->EntryPath, InProposedName);

	return FContentBrowserItemData();
}

void ULuaContentBrowserDataSource::Initialize(const bool InAutoRegister)
{
	Super::Initialize(InAutoRegister);

	FileTypeAction = MakeShared<FAssetTypeActions_LuaFile>();

	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AddNewContextMenu"))
	{
		Menu->AddDynamicSection(*FString::Printf(TEXT("DynamicSection_DataSource_%s"), *GetName()),
		                        FNewToolMenuDelegate::CreateLambda(
			                        [WeakThis = TWeakObjectPtr<ULuaContentBrowserDataSource>(this)](UToolMenu* InMenu)
			                        {
				                        if (ULuaContentBrowserDataSource* This = WeakThis.Get())
				                        {
					                        This->PopulateAddNewContextMenu(InMenu);
				                        }
			                        }));
	}

	// 初始化LuaScriptHierarchy目录结构
	LuaScriptHierarchy = MakeShared<FLuaScriptHierarchy>();
	LuaScriptHierarchy->OnScriptHierarchyChanged.AddUObject(this, &ThisClass::OnScriptHierarchyChanged);
	BuildRootPathVirtualTree();
}

void ULuaContentBrowserDataSource::CompileFilter(const FName InPath, const FContentBrowserDataFilter& InFilter,
                                                 FContentBrowserDataCompiledFilter& OutCompiledFilter)
{
	const bool bIncludeFolders =
		EnumHasAnyFlags(InFilter.ItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFolders);
	const bool bIncludeFiles = EnumHasAnyFlags(InFilter.ItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFiles);
	const bool bIncludeCollections = EnumHasAnyFlags(InFilter.ItemCategoryFilter,
	                                                 EContentBrowserItemCategoryFilter::IncludeCollections);
	const bool bIncludeClasses = EnumHasAnyFlags(InFilter.ItemCategoryFilter,
	                                             EContentBrowserItemCategoryFilter::IncludeClasses);
	const bool bIncludeDeveloper = EnumHasAnyFlags(InFilter.ItemAttributeFilter,
	                                               EContentBrowserItemAttributeFilter::IncludeDeveloper);

	FContentBrowserDataFilterList& FilterList = OutCompiledFilter.CompiledFilters.FindOrAdd(this);
	FContentBrowserLuaScriptDataFilter& ScriptDataFilter = FilterList.FindOrAddFilter<
		FContentBrowserLuaScriptDataFilter>();

	if (!bIncludeClasses || (!bIncludeFolders && !bIncludeFiles))
	{
		return;
	}
	check(LuaScriptHierarchy);
	RefreshVirtualPathTreeIfNeeded();

	FName ConvertedPath;
	TryConvertVirtualPath(InPath, ConvertedPath);

	TSet<FName> InternalPaths;
	InternalPaths.Add(ConvertedPath);

	FLuaScriptHierarchyFilter Filter;
	Filter.InEntryPath = ConvertedPath;
	Filter.IncludeDeveloper = bIncludeDeveloper;
	Filter.IncludeCollections = bIncludeCollections;

	if (bIncludeFolders)
	{
		TArray<FString> ChildScriptFolders;
		LuaScriptHierarchy->GetMatchingFolders(Filter, ChildScriptFolders);

		for (const FString& ChildClassFolder : ChildScriptFolders)
		{
			ScriptDataFilter.ValidFolders.Add(*ChildClassFolder);
		}
	}

	if (bIncludeClasses)
	{
		TArray<TObjectPtr<ULuaFile>> ChildScripts;
		LuaScriptHierarchy->GetMatchingFiles(Filter, ChildScripts);

		for (ULuaFile* ChildScript : ChildScripts)
		{
			ScriptDataFilter.ValidScripts.Add(ChildScript);
		}
	}
}

void ULuaContentBrowserDataSource::BuildRootPathVirtualTree()
{
	Super::BuildRootPathVirtualTree();
	check(LuaScriptHierarchy.IsValid());

	TArray<FName> InternalRoots;
	LuaScriptHierarchy->GetScriptRoots(InternalRoots);
	for (const FName InternalRoot : InternalRoots)
	{
		RootPathAdded(FNameBuilder(InternalRoot));
	}
}

void ULuaContentBrowserDataSource::Tick(const float InDeltaTime)
{
	Super::Tick(InDeltaTime);
	check(LuaScriptHierarchy.IsValid());

	if (!LuaScriptHierarchy->NeedRefresh())
		return;

	TArray<FName> WaitToAsyncFolders;
	TArray<TObjectPtr<ULuaFile>> WaitToAsyncFiles;
	TArray<FContentBrowserItem> Items;

	LuaScriptHierarchy->RefreshHierarchy(WaitToAsyncFolders, WaitToAsyncFiles);

	for (const FName Folder : WaitToAsyncFolders)
	{
		QueueItemDataUpdate(FContentBrowserItemDataUpdate::MakeItemAddedUpdate(CreateScriptFolderItem(Folder)));
	}
	for (ULuaFile* File : WaitToAsyncFiles)
	{
		QueueItemDataUpdate(FContentBrowserItemDataUpdate::MakeItemAddedUpdate(CreateScriptFileItem(File)));
	}

	NotifyItemDataRefreshed();
}

void ULuaContentBrowserDataSource::EnumerateItemsMatchingFilter(const FContentBrowserDataCompiledFilter& InFilter,
                                                                TFunctionRef<bool(FContentBrowserItemData&&)>
                                                                InCallback)
{
	const FContentBrowserDataFilterList* FilterList = InFilter.CompiledFilters.Find(this);
	if (!FilterList)
	{
		return;
	}

	const FContentBrowserLuaScriptDataFilter* ScriptDataFilter = FilterList->FindFilter<
		FContentBrowserLuaScriptDataFilter>();
	if (!ScriptDataFilter)
	{
		return;
	}

	if (EnumHasAnyFlags(InFilter.ItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFiles))
	{
		for (const FName& ValidFolder : ScriptDataFilter->ValidFolders)
		{
			if (!InCallback(CreateScriptFolderItem(ValidFolder)))
			{
				return;
			}
		}
	}

	if (EnumHasAnyFlags(InFilter.ItemCategoryFilter, EContentBrowserItemCategoryFilter::IncludeClasses))
	{
		for (const TObjectPtr<ULuaFile>& ValidScript : ScriptDataFilter->ValidScripts)
		{
			if (!InCallback(CreateScriptFileItem(ValidScript)))
			{
				return;
			}
		}
	}
}

void ULuaContentBrowserDataSource::EnumerateItemsAtPath(const FName InPath,
                                                        const EContentBrowserItemTypeFilter InItemTypeFilter,
                                                        TFunctionRef<bool(FContentBrowserItemData&&)> InCallback)
{
	FName InternalPath;
	if (!TryConvertVirtualPathToInternal(InPath, InternalPath))
	{
		return;
	}

	check(LuaScriptHierarchy);

	bool bIncludeFolders = EnumHasAnyFlags(InItemTypeFilter, EContentBrowserItemTypeFilter::IncludeFolders);
	FLuaScriptHierarchyFilter Filter;
	Filter.InEntryPath = InternalPath;
}

bool ULuaContentBrowserDataSource::EnumerateItemsForObjects(const TArrayView<UObject*> InObjects,
                                                            TFunctionRef<bool(FContentBrowserItemData&&)> InCallback)
{
	return false;
}

bool ULuaContentBrowserDataSource::GetItemAttribute(const FContentBrowserItemData& InItem, const bool InIncludeMetaData,
                                                    const FName InAttributeKey,
                                                    FContentBrowserItemDataAttributeValue& OutAttributeValue)
{
	TSharedPtr<FAssetTypeActions_LuaFile, ESPMode::ThreadSafe> TypeAction;
	if (const auto& Payload = GetScriptFileItemPayload(InItem))
	{
		FString Suffix;
		Payload->GetScript()->EntryPath.Split(".", nullptr, &Suffix, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		TypeAction = FileTypeAction;
	}
	else
	{
		TypeAction = FileTypeAction;
	}
	return ContentBrowserLuaScriptData::GetItemAttribute(TypeAction.Get(), this, InItem, InIncludeMetaData,
	                                                     InAttributeKey, OutAttributeValue);
}

bool ULuaContentBrowserDataSource::GetItemAttributes(const FContentBrowserItemData& InItem,
                                                     const bool InIncludeMetaData,
                                                     FContentBrowserItemDataAttributeValues& OutAttributeValues)
{
	return ContentBrowserLuaScriptData::GetItemAttributes(this, InItem, InIncludeMetaData, OutAttributeValues);
}

bool ULuaContentBrowserDataSource::GetItemPhysicalPath(const FContentBrowserItemData& InItem, FString& OutDiskPath)
{
	return ContentBrowserLuaScriptData::GetItemPhysicalPath(this, InItem, OutDiskPath);
}

bool ULuaContentBrowserDataSource::UpdateThumbnail(const FContentBrowserItemData& InItem, FAssetThumbnail& InThumbnail)
{
	return ContentBrowserLuaScriptData::UpdateItemThumbnail(this, InItem, InThumbnail);
}

bool ULuaContentBrowserDataSource::CanEditItem(const FContentBrowserItemData& InItem, FText* OutErrorMsg)
{
	if (InItem.IsFile()) return true;
	return false;
}

bool ULuaContentBrowserDataSource::EditItem(const FContentBrowserItemData& InItem)
{
	TSharedPtr<FAssetTypeActions_LuaFile, ESPMode::ThreadSafe> TypeAction;
	if (const auto Payload = GetScriptFileItemPayload(InItem))
	{
		FString Suffix;
		Payload->GetScript()->EntryPath.Split(".", nullptr, &Suffix, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		TypeAction = FileTypeAction;
	}
	else
	{
		TypeAction = FileTypeAction;
	}
	return ContentBrowserLuaScriptData::EditItem(TypeAction.Get(), this, InItem);
}

bool ULuaContentBrowserDataSource::CanRenameItem(const FContentBrowserItemData& InItem, const FString* InNewName,
	FText* OutErrorMsg)
{
	return ContentBrowserLuaScriptData::CanRenameItem(this, InItem, InNewName, OutErrorMsg);
}

bool ULuaContentBrowserDataSource::RenameItem(const FContentBrowserItemData& InItem, const FString& InNewName,
	FContentBrowserItemData& OutNewItem)
{
	ContentBrowserLuaScriptData::RenameItem(this, InItem, InNewName);
	return false;
}

bool ULuaContentBrowserDataSource::CanMoveItem(const FContentBrowserItemData& InItem, const FName InDestPath,
	FText* OutErrorMsg)
{
	return true;
}

bool ULuaContentBrowserDataSource::MoveItem(const FContentBrowserItemData& InItem, const FName InDestPath)
{
	FName InDestInternalPath;
	TryConvertVirtualPath(InDestPath, InDestInternalPath);

	return ContentBrowserLuaScriptData::MoveItem(this, InItem, InDestInternalPath);
}

bool ULuaContentBrowserDataSource::CanDeleteItem(const FContentBrowserItemData& InItem, FText* OutErrorMsg)
{
	return true;
}

bool ULuaContentBrowserDataSource::DeleteItem(const FContentBrowserItemData& InItem)
{
	return ContentBrowserLuaScriptData::DeleteItems(this, MakeArrayView(&InItem, 1));
}

bool ULuaContentBrowserDataSource::BulkDeleteItems(TArrayView<const FContentBrowserItemData> InItems)
{
	return ContentBrowserLuaScriptData::DeleteItems(this, InItems);
}


void ULuaContentBrowserDataSource::PopulateAddNewContextMenu(UToolMenu* InMenu)
{
	const UContentBrowserDataMenuContext_AddNewMenu* MenuContext = InMenu->FindContext<
		UContentBrowserDataMenuContext_AddNewMenu>();
	checkf(MenuContext, TEXT("ULuaContentBrowserDataSource::PopulateAddNewContextMenu: Menu context is null"));

	TArray<FName> SelectedScriptPath;
	for (const FName& SelectedPath : MenuContext->SelectedPaths)
	{
		FName InternalPath;
		if (TryConvertVirtualPathToInternal(SelectedPath, InternalPath))
		{
			SelectedScriptPath.Add(InternalPath);
		}
	}

	FOnNewScriptRequested OnNewScriptRequested;
	if (SelectedScriptPath.Num() > 0)
	{
		if (MenuContext->OnBeginItemCreation.IsBound())
		{
			OnNewScriptRequested = FOnNewScriptRequested::CreateUObject(this, &ThisClass::OnNewAssetRequested,
			                                                            MenuContext->OnBeginItemCreation);
		}
	}
	MakeContextMenu(InMenu, SelectedScriptPath, OnNewScriptRequested);
}

void ULuaContentBrowserDataSource::MakeContextMenu(UToolMenu* Menu, const TArray<FName>& InSelectedScriptPaths,
                                                   FOnNewScriptRequested InOnNewScriptRequested)
{
	if (InSelectedScriptPaths.Num() == 0)
		return;

	const FName FirstSelectedPath = InSelectedScriptPaths[0];
	const bool bHasSinglePathSelected = InSelectedScriptPaths.Num() == 1;
	auto CanExecuteClassActions = [bHasSinglePathSelected]() -> bool
	{
		// We can execute class actions when we only have a single path selected
		return bHasSinglePathSelected;
	};
	const FCanExecuteAction CanExecuteClassActionsDelegate = FCanExecuteAction::CreateLambda(CanExecuteClassActions);
	if (InOnNewScriptRequested.IsBound())
	{
		FText NewScriptToolTip;
		if (bHasSinglePathSelected)
		{
			NewScriptToolTip = FText::Format(
				LOCTEXT("NewLuaScriptTooltip_CreateIn", "Create a new lua script in {0}."),
				FText::FromName(FirstSelectedPath));
		}
		else
		{
			NewScriptToolTip = LOCTEXT("NewLuaScriptTooltip_InvalidNumberOfPaths",
			                           "Can only create lua script when there is a single path selected.");
		}

		FToolMenuSection& Section = Menu->AddSection("ContentBrowserNewLuaScript",
		                                             LOCTEXT("LuaMenuHeading", "Lua Script"));
		Section.AddMenuEntry(
			"NewScript",
			LOCTEXT("NewLuaScriptLabel", "New Lua Script..."),
			NewScriptToolTip,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "MainFrame.AddCodeToProject"),
			FUIAction(
				FExecuteAction::CreateStatic(&ULuaContentBrowserDataSource::ExecuteNewClass, FirstSelectedPath,
				                             InOnNewScriptRequested),
				CanExecuteClassActionsDelegate
			)
		);
	}
}

void ULuaContentBrowserDataSource::OnNewAssetRequested(const FName InPath,
                                                       UContentBrowserDataMenuContext_AddNewMenu::FOnBeginItemCreation
                                                       InOnBeginItemCreation)
{
	if (ensure(!InPath.IsNone()) && ensure(InOnBeginItemCreation.IsBound()))
	{
		const FString AbsolutePath = LUA::ConvertInternalPathToAbsolutePath(InPath.ToString());
		TArray<FString> FoundedFiles;
		FPlatformFileManager::Get().GetPlatformFile().FindFiles(FoundedFiles, *AbsolutePath, TEXT(".lua"));

		FName DefaultAssetName = TEXT("NewLuaScript");
		bool IsSpecial = false;
		int Suffix = 1;
		while (!IsSpecial)
		{
			IsSpecial = true;
			for (const auto& Path : FoundedFiles)
			{
				if (Path.Contains(DefaultAssetName.ToString()))
				{
					IsSpecial = false;
					DefaultAssetName = FName(*FString::Printf(TEXT("NewLuaScript%d"), Suffix++));
					break;
				}
			}
		}
		FName VirtualPath;
		TryConvertInternalPathToVirtual(InPath, VirtualPath);
		VirtualPath = FName(*(VirtualPath.ToString() + TEXT("/")));
		ULuaFile* Asset = LuaScriptHierarchy->ScriptPreCreate(InPath.ToString(), DefaultAssetName);
		check(Asset);
		FName Temp = FName(*(InPath.ToString() / DefaultAssetName.ToString()));
		FContentBrowserItemData NewItemData(this, EContentBrowserItemFlags::Type_File | EContentBrowserItemFlags::Category_Class | EContentBrowserItemFlags::Temporary_Creation, VirtualPath, DefaultAssetName, FText::FromString(DefaultAssetName.ToString()), MakeShared<FContentBrowserLuaScriptFileItemDataPayload>(Temp, Asset));
		InOnBeginItemCreation.Execute(FContentBrowserItemDataTemporaryContext(MoveTemp(NewItemData), FContentBrowserItemDataTemporaryContext::FOnValidateItem::CreateUObject(this, &ThisClass::OnValidateItemName), FContentBrowserItemDataTemporaryContext::FOnFinalizeItem::CreateUObject(this, &ThisClass::OnFinalizeCreateAsset)));
	}
}

void ULuaContentBrowserDataSource::ExecuteNewClass(FName InPath, FOnNewScriptRequested InOnNewScriptRequested)
{
	InOnNewScriptRequested.ExecuteIfBound(InPath);
}


FContentBrowserItemData ULuaContentBrowserDataSource::CreateScriptFolderItem(const FName InFolderPath)
{
	FName VirtualizedPath;
	TryConvertInternalPathToVirtual(InFolderPath, VirtualizedPath);

	return ContentBrowserLuaScriptData::CreateScriptFolderItem(this, VirtualizedPath, InFolderPath);
}

FContentBrowserItemData ULuaContentBrowserDataSource::CreateScriptFileItem(ULuaFile* InFile)
{
	const FName ScriptPath = *InFile->EntryPath;
	FName VirtualizedPath;
	TryConvertInternalPathToVirtual(ScriptPath, VirtualizedPath);

	return ContentBrowserLuaScriptData::CreateScriptFileItem(this, VirtualizedPath, ScriptPath, InFile);
}

TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> ULuaContentBrowserDataSource::GetScriptFileItemPayload(
	const FContentBrowserItemData& InItem) const
{
	return ContentBrowserLuaScriptData::GetScriptFileItemDataPayload(this, InItem);
}

TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload>
ULuaContentBrowserDataSource::GetScriptFolderItemPayload(const FContentBrowserItemData& InItem) const
{
	return ContentBrowserLuaScriptData::GetScriptFolderItemPayload(this, InItem);
}

void ULuaContentBrowserDataSource::OnScriptHierarchyChanged()
{
	SetVirtualPathTreeNeedsRebuild();
	NotifyItemDataRefreshed();
}

#undef LOCTEXT_NAMESPACE
