#include "LuaCodeProjectItem.h"
#include "UObject/Package.h"
#include "IDirectoryWatcher.h"
#include "DirectoryScanner.h"

ULuaCodeProjectItem::ULuaCodeProjectItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULuaCodeProjectItem::RescanChildren()
{
	if(Path.Len() > 0)
	{
		FDirectoryScanner::AddDirectory(Path, FOnDirectoryScanned::CreateUObject(this, &ULuaCodeProjectItem::HandleDirectoryScanned));
	}
}

void ULuaCodeProjectItem::HandleDirectoryScanned(const FString& InPathName, ELuaCodeProjectItemType::Type InType)
{
	// check for a child that already exists
	bool bCreateNew = true;
	for(const auto& Child : Children)
	{
		if(Child->Type == InType && Child->Path == InPathName)
		{
			bCreateNew = false;
			break;
		}
	}

	// create children now & kick off their scan
	if(bCreateNew)
	{
		ULuaCodeProjectItem* NewItem = NewObject<ULuaCodeProjectItem>(GetOutermost(), ULuaCodeProjectItem::StaticClass());
		NewItem->Type = InType;
		NewItem->Path = InPathName;
		NewItem->Name = FPaths::GetCleanFilename(InPathName);
		if(InType != ELuaCodeProjectItemType::Folder)
		{
			NewItem->Extension = FPaths::GetExtension(InPathName);
		}

		Children.Add(NewItem);

		Children.Sort(
			[](const ULuaCodeProjectItem& ItemA, const ULuaCodeProjectItem& ItemB) -> bool
			{
				if(ItemA.Type != ItemB.Type)
				{
					return ItemA.Type < ItemB.Type;
				}

				return ItemA.Name.Compare(ItemB.Name) < 0;
			}
		);

		if(InType == ELuaCodeProjectItemType::Folder)
		{
			// kick off another scan for subdirectories
			FDirectoryScanner::AddDirectory(InPathName, FOnDirectoryScanned::CreateUObject(NewItem, &ULuaCodeProjectItem::HandleDirectoryScanned));

			// @TODO: now register for any changes to this directory if needed
		//	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::Get().LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		//	DirectoryWatcherModule.Get()->RegisterDirectoryChangedCallback_Handle(InPathName, IDirectoryWatcher::FDirectoryChanged::CreateUObject(NewItem, &ULuaCodeProjectItem::HandleDirectoryChanged), OnDirectoryChangedHandle);
		}
	}
}

void ULuaCodeProjectItem::HandleDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	// @TODO: dynamical update directory watchers so we can update the view in real-time
	for(const auto& Change : FileChanges)
	{
		switch(Change.Action)
		{
		default:
		case FFileChangeData::FCA_Unknown:
			break;
		case FFileChangeData::FCA_Added:
			{

			}
			break;
		case FFileChangeData::FCA_Modified:
			{

			}
			break;
		case FFileChangeData::FCA_Removed:
			{

			}
			break;
		}
	}
}
