#include "ContentBrowserLuaScriptData.h"

#include "ContentBrowserDataSource.h"
#include "ContentBrowserLuaScriptDataPayload.h"
#include "IAssetTypeActions.h"
#include "IPropertyAccessEditor.h"
#include "LuaContentBrowserUtil.h"
#include "LuaFile.h"

#define LOCTEXT_NAMESPACE "ContentBrowserLuaScriptData"

struct FScriptTagDefinition
{
	UObject::FAssetRegistryTag::ETagType TagType = UObject::FAssetRegistryTag::TT_Hidden;
	uint32 DisplayFlags = UObject::FAssetRegistryTag::TD_None;
	FText DisplayName;
};
using FClassTagDefinitionMap = TSortedMap<FName, FScriptTagDefinition, FDefaultAllocator, FNameFastLess>;

const FClassTagDefinitionMap& GetAvailableClassTags()
{
	static const FClassTagDefinitionMap ClassTags = []()
	{
		FClassTagDefinitionMap ClassTagsTmp;

		TArray<UObject::FAssetRegistryTag> CDOClassTags;
		GetDefault<UClass>()->GetAssetRegistryTags(CDOClassTags);

		for (const UObject::FAssetRegistryTag& CDOClassTag : CDOClassTags)
		{
			FScriptTagDefinition& ClassTag = ClassTagsTmp.Add(CDOClassTag.Name);
			ClassTag.TagType = CDOClassTag.Type;
			ClassTag.DisplayFlags = CDOClassTag.DisplayFlags;
			ClassTag.DisplayName = FText::AsCultureInvariant(FName::NameToDisplayString(CDOClassTag.Name.ToString(), /*bIsBool*/false));
		}

		return ClassTagsTmp;
	}();

	return ClassTags;
}

static void GetScriptItemAttribute(const bool InIncludeMetaData, FContentBrowserItemDataAttributeValue& OutAttributeValue)
{
	OutAttributeValue.SetValue(NAME_Class);
	if (InIncludeMetaData)
	{
		static const FText ScriptDisplayName = LOCTEXT("AttributeDisplayName_LuaScript", "Lua Script");

		FContentBrowserItemDataAttributeMetaData AttributeMetaData;
		AttributeMetaData.AttributeType = UObject::FAssetRegistryTag::TT_Hidden;
		AttributeMetaData.DisplayName = ScriptDisplayName;
		OutAttributeValue.SetMetaData(MoveTemp(AttributeMetaData));
	}
}

static void GetGenericItemAttribute(const FName InTagKey, const FString& InTagValue, const bool InIncludeMetaData, FContentBrowserItemDataAttributeValue& OutAttributeValue)
{
	check(!InTagKey.IsNone());

	if (FTextStringHelper::IsComplexText(*InTagValue))
	{
		FText TmpText;
		if (FTextStringHelper::ReadFromBuffer(*InTagValue, TmpText))
		{
			OutAttributeValue.SetValue(TmpText);
		}
	}
	if (!OutAttributeValue.IsValid())
	{
		OutAttributeValue.SetValue(InTagValue);
	}
	if (InIncludeMetaData)
	{
		FContentBrowserItemDataAttributeMetaData AttributeMetaData;
		if (const FScriptTagDefinition* ScriptTagCache = GetAvailableClassTags().Find(InTagKey))
		{
			AttributeMetaData.AttributeType = ScriptTagCache->TagType;
			AttributeMetaData.DisplayName = ScriptTagCache->DisplayName;
			AttributeMetaData.DisplayFlags = ScriptTagCache->DisplayFlags;
		}else
		{
			AttributeMetaData.DisplayName = FText::AsCultureInvariant(FName::NameToDisplayString(InTagKey.ToString(), false));
		}
		OutAttributeValue.SetMetaData(MoveTemp(AttributeMetaData));
	}
}

static bool GetScriptFileItemAttributes(const FContentBrowserLuaScriptFileItemDataPayload& InScriptPayload, const bool InIncludeMetaData, FContentBrowserItemDataAttributeValues& OutAttributeValues)
{
	FContentBrowserItemDataAttributeValue& ScriptAttributeValue = OutAttributeValues.FindOrAdd(NAME_Class);
	GetScriptItemAttribute(InIncludeMetaData, ScriptAttributeValue);

	const FAssetData& AssetData = InScriptPayload.GetAssetData();
	OutAttributeValues.Reserve(OutAttributeValues.Num() + AssetData.TagsAndValues.Num());
	for (const auto& TagAndValue : AssetData.TagsAndValues)
	{
		FContentBrowserItemDataAttributeValue& AttributeValue = OutAttributeValues.Add(TagAndValue.Key);
		GetGenericItemAttribute(TagAndValue.Key, TagAndValue.Value.AsString(), InIncludeMetaData, AttributeValue);
	}

	return true;
}

static bool GetScriptFolderItemPhysicalPath(const FContentBrowserLuaScriptFolderItemDataPayload& InFolderPayload, FString& OutDiskPath)
{
	const FString& FolderFileName = InFolderPayload.GetFilename();
	if (!FolderFileName.IsEmpty())
	{
		OutDiskPath = FolderFileName;
		return true;
	}
	return false;
}

static bool GetScriptFileItemPhysicalPath(const FContentBrowserLuaScriptFileItemDataPayload& InScriptPayload, FString& OutDiskPath)
{
	const FString& ScriptFileName = InScriptPayload.GetFilename();
	if (!ScriptFileName.IsEmpty())
	{
		OutDiskPath = ScriptFileName;
		return true;
	}
	return false;
}

static bool UpdateScriptFileItemThumbnail(const FContentBrowserLuaScriptFileItemDataPayload& InScriptPayload, FAssetThumbnail& InThumbnail)
{
	InScriptPayload.UpdateThumbnail(InThumbnail);
	return true;
}

static bool CanRenameFileItem(const FContentBrowserLuaScriptFileItemDataPayload& Payload, const FString* InNewName, bool IsTemporary, FText* OutErrorMsg)
{
	// 查重判断
	bool IsNewNameRepeat = false;
		
	const FString& EntryPath = Payload.GetInternalPath().ToString();
	const FString AbsolutePath = LUA::ConvertInternalPathToAbsolutePath(EntryPath);
	const SIZE_T SearchEnd = AbsolutePath.Find("/", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString OriName = AbsolutePath.RightChop(SearchEnd + 1);
	const FString Dir = AbsolutePath.LeftChop(AbsolutePath.Len()-SearchEnd);
	
	TArray<FString> FoundFiles;
	FPlatformFileManager::Get().GetPlatformFile().FindFiles(FoundFiles, *Dir, TEXT(".cs"));
	for (auto Path : FoundFiles)
	{
		const SIZE_T TempSearchEnd = Path.Find("/", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		const FString FileName = Path.RightChop(TempSearchEnd + 1).LeftChop(3);
		if (!FileName.Equals(OriName) && FileName.Equals(*InNewName))
		{
			IsNewNameRepeat =  true;
			break;
		}
	}
	
	if (!IsNewNameRepeat) return true;

	*OutErrorMsg = LOCTEXT("LUARenameError", "The name is used by another file, please change it...");
	return false;
}

static bool CanRenameFolderItem(const FContentBrowserLuaScriptFolderItemDataPayload& Payload, const FString* InNewName, FText* OutErrorMsg)
{
	bool IsNewNameRepeat = false;
		
	const FString& EntryPath = Payload.GetInternalPath().ToString();
	const FString AbsolutePath = LUA::ConvertInternalPathToAbsolutePath(EntryPath);
	const SIZE_T SearchEnd = AbsolutePath.Find("/", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString OriName = AbsolutePath.RightChop(SearchEnd + 1);
	const FString Dir = AbsolutePath.LeftChop(AbsolutePath.Len()-SearchEnd);
	
	LUA::FDirectoryEnumerator DirectoryVisitor;
	FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*Dir, DirectoryVisitor);
	for (auto Path : DirectoryVisitor.Directories)
	{
		const SIZE_T TempSearchEnd = Path.Find("/", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		const FString FileName = Path.RightChop(TempSearchEnd + 1);
		if (!FileName.Equals(OriName) && FileName.Equals(*InNewName))
		{
			IsNewNameRepeat =  true;
			break;
		}
	}
	
	if (!IsNewNameRepeat) return true;
	*OutErrorMsg = LOCTEXT("LuaFolderRenameError", "The name is used by another folder, please change it...");
	return false;
}

void CopyFilesRecursive(FString OriPath, FString DestPath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		
	if (!PlatformFile.DirectoryExists(*DestPath)) PlatformFile.CreateDirectory(*DestPath);

	LUA::FDirectoryEnumerator DirectoryVisitor;

	PlatformFile.IterateDirectory(*OriPath, DirectoryVisitor);

	for(auto File : DirectoryVisitor.Files)
	{
		FString NewFile = DestPath + File.RightChop(OriPath.Len());

		if (!IFileManager::Get().Move(*NewFile, *File))
			UE_LOG(LogTemp, Error, TEXT("Move File Failed"));
	}
		
	for (auto Directory : DirectoryVisitor.Directories)
	{
		FString NewDirectory = DestPath + Directory.RightChop(OriPath.Len());
		if (!IFileManager::Get().MakeDirectory(*NewDirectory))
			UE_LOG(LogTemp, Error, TEXT("Make Directory Failed"));

		CopyFilesRecursive(Directory, NewDirectory);
	}
}

static bool RenameFolderItem(const FContentBrowserLuaScriptFolderItemDataPayload& Payload, const FString& InItemsNewName)
{
	FString OriginPath, DirName;
	const FString AbsolutePath = LUA::ConvertInternalPathToAbsolutePath(
		Payload.GetInternalPath().ToString());

	AbsolutePath.Split("/", &OriginPath, &DirName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString NewPath = AbsolutePath.Replace(*DirName, *InItemsNewName);

	CopyFilesRecursive(AbsolutePath, NewPath);
	if (!IFileManager::Get().DeleteDirectory(*AbsolutePath, false, true))
		UE_LOG(LogTemp, Error, TEXT("Delete Directory Failed"));
	return true;
}

static bool RenameFileItem(const FContentBrowserLuaScriptFileItemDataPayload& Payload, const FString& InItemsNewName)
{
	FString OriginPath, FileName;
	const FString AbsolutePath = LUA::ConvertInternalPathToAbsolutePath(
		Payload.GetInternalPath().ToString());

	AbsolutePath.Split("/", &OriginPath, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString NewPath = AbsolutePath.Replace(*FileName.LeftChop(4), *InItemsNewName);
		
	if (!IFileManager::Get().Move(*NewPath, *AbsolutePath))
		UE_LOG(LogTemp, Error, TEXT("Rename File Failed"));
		
	return true;
}

static bool MoveFileItem(const FContentBrowserLuaScriptFileItemDataPayload& Payload, const FString& InDestPath)
{
	FString OriginPath, FileName;
	const FString AbsolutePath = LUA::ConvertInternalPathToAbsolutePath(
		Payload.GetInternalPath().ToString());

	AbsolutePath.Split("/", &OriginPath, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString NewPath = InDestPath / FileName;
		
	if (!IFileManager::Get().Move(*NewPath, *AbsolutePath))
		UE_LOG(LogTemp, Error, TEXT("Move File Failed"));
		
	return true;
}

static bool MoveFolderItem(const FContentBrowserLuaScriptFolderItemDataPayload& Payload, const FString& InDestPath)
{
	FString OriginPath, FileName;
	const FString AbsolutePath = LUA::ConvertInternalPathToAbsolutePath(
		Payload.GetInternalPath().ToString());

	AbsolutePath.Split("/", &OriginPath, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString NewPath = InDestPath / FileName;

	CopyFilesRecursive(AbsolutePath, NewPath);
	if (!IFileManager::Get().DeleteDirectory(*AbsolutePath, false, true))
		UE_LOG(LogTemp, Error, TEXT("Delete Directory Failed"));
	return true;
}

void EnumerateScriptFileItemPayload(const UContentBrowserDataSource* InOwnerDataSource, TArrayView<const FContentBrowserItemData> InItems, TFunctionRef<bool(const TSharedRef<const FContentBrowserLuaScriptFileItemDataPayload>&)> InScriptPayloadCallback)
{
	for (const FContentBrowserItemData& Item : InItems)
	{
		if (TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> ClassPayload = ContentBrowserLuaScriptData::GetScriptFileItemDataPayload(InOwnerDataSource, Item))
		{
			if (!InScriptPayloadCallback(ClassPayload.ToSharedRef()))
			{
				break;
			}
		}
	}
}

void EnumerateScriptFolderItemPayload(const UContentBrowserDataSource* InOwnerDataSource,
										TArrayView<const FContentBrowserItemData> InItems,
										TFunctionRef<bool(const TSharedRef<const FContentBrowserLuaScriptFolderItemDataPayload>&)>
										InFolderPayloadCallback)
{
	for (const FContentBrowserItemData& Item : InItems)
	{
		if (TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload> ClassPayload = ContentBrowserLuaScriptData::GetScriptFolderItemPayload(InOwnerDataSource, Item))
		{
			if (!InFolderPayloadCallback(ClassPayload.ToSharedRef()))
			{
				break;
			}
		}
	}
}

namespace ContentBrowserLuaScriptData
{
	TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload> GetScriptFolderItemPayload(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem)
	{
		if (InItem.GetOwnerDataSource() == InOwnerDataSource && InItem.IsFolder())
		{
			return StaticCastSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload>(InItem.GetPayload());
		}
		return nullptr;
	}

	TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> GetScriptFileItemDataPayload(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem)
	{
		if (InItem.GetOwnerDataSource() == InOwnerDataSource && InItem.IsFile())
		{
			return StaticCastSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload>(InItem.GetPayload());
		}
		return nullptr;
	}

	bool EditItem(IAssetTypeActions* InClassTypeActions, const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem)
	{
		if (TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> ClassPayload = GetScriptFileItemDataPayload(InOwnerDataSource, InItem))
		{
			return EditScriptFileItem(InClassTypeActions, ClassPayload.ToSharedRef());
		}
		
		return false;
	}

	bool EditScriptFileItem(IAssetTypeActions* InScriptTypeActions, TSharedRef<const FContentBrowserLuaScriptFileItemDataPayload> InScriptPayload)
	{
		TArray<UObject*> ScriptList;
		if (ULuaFile* ScriptPtr = InScriptPayload->GetScript())
		{
			ScriptList.Add(ScriptPtr);
		}

		if (ScriptList.Num() > 0)
		{
			InScriptTypeActions->OpenAssetEditor(ScriptList);
			return true;
		}

		return false;
	}

	bool GetItemAttribute(IAssetTypeActions* InScriptTypeActions, const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue)
	{
		if (TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload> FolderPayload = GetScriptFolderItemPayload(InOwnerDataSource, InItem))
		{
			return GetScriptFolderItemAttribute(*FolderPayload, InIncludeMetaData, InAttributeKey, OutAttributeValue);
		}

		if (TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> ScriptPayload = GetScriptFileItemDataPayload(InOwnerDataSource, InItem))
		{
			return GetScriptFileItemAttribute(InScriptTypeActions, *ScriptPayload, InIncludeMetaData, InAttributeKey, OutAttributeValue);
		}

		return false;
	}

	bool GetItemAttributes(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const bool InIncludeMetaData, FContentBrowserItemDataAttributeValues& OutAttributeValues)
	{
		if (TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> ClassPayload = GetScriptFileItemDataPayload(InOwnerDataSource, InItem))
		{
			return GetScriptFileItemAttributes(*ClassPayload, InIncludeMetaData, OutAttributeValues);
		}

		return false;
	}

	bool GetScriptFolderItemAttribute(const FContentBrowserLuaScriptFolderItemDataPayload& InFolderPayload, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue)
	{
		return false;
	}

	bool GetScriptFileItemAttribute(IAssetTypeActions* InScriptTypeActions, const FContentBrowserLuaScriptFileItemDataPayload& InScriptPayload, const bool InIncludeMetaData, const FName InAttributeKey, FContentBrowserItemDataAttributeValue& OutAttributeValue)
	{
		static const FName NAME_Type = "Type";
		if (InAttributeKey == ContentBrowserItemAttributes::ItemTypeName || InAttributeKey == NAME_Class || InAttributeKey == NAME_Type)
		{
			GetScriptItemAttribute(InIncludeMetaData, OutAttributeValue);
			return true;
		}
		if (InAttributeKey == ContentBrowserItemAttributes::ItemTypeDisplayName)
		{
			OutAttributeValue.SetValue(InScriptTypeActions->GetName());
			return true;
		}
		if (InAttributeKey == ContentBrowserItemAttributes::ItemDescription)
		{
			const FText AssetDescription = InScriptTypeActions->GetAssetDescription(InScriptPayload.GetAssetData());
			if (!AssetDescription.IsEmpty())
			{
				OutAttributeValue.SetValue(AssetDescription);
				return true;
			}
			return false;
		}
		OutAttributeValue.SetValue(true);

		if (InAttributeKey == ContentBrowserItemAttributes::ItemColor)
		{
			const FLinearColor AssetColor = InScriptTypeActions->GetTypeColor();
			OutAttributeValue.SetValue(AssetColor.ToString());
			return true;
		}

		const FAssetData& AssetData = InScriptPayload.GetAssetData();
		const FName FoundAttributeKey = InAttributeKey ;
		const FAssetDataTagMapSharedView::FFindTagResult FoundValue = AssetData.TagsAndValues.FindTag(FoundAttributeKey);
		if (FoundValue.IsSet())
		{
			GetGenericItemAttribute(FoundAttributeKey, FoundValue.GetValue(), InIncludeMetaData, OutAttributeValue);
			return true;
		}
		return false;
	}

	bool GetItemPhysicalPath(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FString& OutDiskPath)
	{
		if (TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload> FolderPayload = GetScriptFolderItemPayload(InOwnerDataSource, InItem))
		{
			return GetScriptFolderItemPhysicalPath(*FolderPayload, OutDiskPath);
		}
		if (TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> ScriptPayload = GetScriptFileItemDataPayload(InOwnerDataSource, InItem))
		{
			return GetScriptFileItemPhysicalPath(*ScriptPayload, OutDiskPath);
		}
		return false;
	}

	bool UpdateItemThumbnail(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, FAssetThumbnail& InThumbnail)
	{
		if (TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> ScriptPayload = GetScriptFileItemDataPayload(InOwnerDataSource, InItem))
		{
			return UpdateScriptFileItemThumbnail(*ScriptPayload, InThumbnail);
		}
		return false;
	}

	bool CanRenameItem(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const FString* InNewName, FText* OutErrorMsg)
	{
		if (!InNewName) return true;
		
		if (const TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload> FolderPayload = GetScriptFolderItemPayload(InOwnerDataSource, InItem))
		{
			return CanRenameFolderItem(*FolderPayload, InNewName, OutErrorMsg);
		}

		if (TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> AssetPayload = GetScriptFileItemDataPayload(InOwnerDataSource, InItem))
		{
			return CanRenameFileItem(*AssetPayload, InNewName, InItem.IsTemporary(), OutErrorMsg);
		}

		return false;
	}

	bool RenameItem(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const FString& InItemsNewName)
	{
		if (TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload> FolderPayload = GetScriptFolderItemPayload(InOwnerDataSource, InItem))
		{
			return RenameFolderItem(*FolderPayload, InItemsNewName);
		}

		if (TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> FilePayload = GetScriptFileItemDataPayload(InOwnerDataSource, InItem))
		{
			if (CanRenameFileItem(*FilePayload, &InItemsNewName, InItem.IsTemporary(), nullptr))
			{
				return RenameFileItem(*FilePayload, InItemsNewName);
			}
		}

		return false;
	}

	bool MoveItem(const UContentBrowserDataSource* InOwnerDataSource, const FContentBrowserItemData& InItem, const FName InDestPath)
	{
		const FString DestAbsolutePath = LUA::ConvertInternalPathToAbsolutePath(InDestPath.ToString());
		if (const TSharedPtr<const FContentBrowserLuaScriptFileItemDataPayload> ScriptPayload = GetScriptFileItemDataPayload(InOwnerDataSource, InItem))
		{
			return MoveFileItem(*ScriptPayload, DestAbsolutePath);
		}
		if (const TSharedPtr<const FContentBrowserLuaScriptFolderItemDataPayload> FolderPayload = GetScriptFolderItemPayload(InOwnerDataSource, InItem))
		{
			return MoveFolderItem(*FolderPayload, DestAbsolutePath);
		}
		return false;
	}

	bool DeleteItems(const UContentBrowserDataSource* InOwnerDataSource, TArrayView<const FContentBrowserItemData> InItems)
	{
		TArray<TSharedRef<const FContentBrowserLuaScriptFileItemDataPayload>, TInlineAllocator<16>> ClassPayloads;
		EnumerateScriptFileItemPayload(InOwnerDataSource, InItems, [&ClassPayloads](const TSharedRef<const FContentBrowserLuaScriptFileItemDataPayload>& InClassPayload)
		{
			ClassPayloads.Add(InClassPayload);
			return true;
		});
		TArray<FString> FilesToDelete;
		for (const TSharedRef<const FContentBrowserLuaScriptFileItemDataPayload>& ClassPayload : ClassPayloads)
		{
			FilesToDelete.Add(LUA::ConvertInternalPathToAbsolutePath(ClassPayload->GetScript()->Path));
		}

		TArray<TSharedRef<const FContentBrowserLuaScriptFolderItemDataPayload>, TInlineAllocator<16>> FolderPayloads;
		EnumerateScriptFolderItemPayload(InOwnerDataSource, InItems, [&FolderPayloads](const TSharedRef<const FContentBrowserLuaScriptFolderItemDataPayload>& InFolderPayload)
		{
			FolderPayloads.Add(InFolderPayload);
			return true;
		});
		TArray<FString> DirectoriesToDelete;
		for (const TSharedRef<const FContentBrowserLuaScriptFolderItemDataPayload>& FolderPayload : FolderPayloads)
		{
			DirectoriesToDelete.Add(LUA::ConvertInternalPathToAbsolutePath(FolderPayload->GetInternalPath().ToString()));
		}

		if (!FilesToDelete.IsEmpty())
		{
			for (const FString& File : FilesToDelete)
				IFileManager::Get().Delete(*File);
		}

		if (!DirectoriesToDelete.IsEmpty())
		{
			for (const FString& Dir : DirectoriesToDelete)
				IFileManager::Get().DeleteDirectory(*Dir, false, true);
		}
		
		return !FilesToDelete.IsEmpty() || !DirectoriesToDelete.IsEmpty();
	}
}

#undef LOCTEXT_NAMESPACE