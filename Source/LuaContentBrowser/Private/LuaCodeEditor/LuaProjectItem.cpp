// Fill out your copyright notice in the Description page of Project Settings.


#include "LuaProjectItem.h"
#include "UObject/Package.h"
#include "IDirectoryWatcher.h"
#include "DirectoryScanner.h"

ULuaProjectItem::ULuaProjectItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULuaProjectItem::RescanChildren()
{
	if(Path.Len() > 0)
	{
		FDirectoryScanner::AddDirectory(Path, FOnDirectoryScanned::CreateUObject(this, &ULuaProjectItem::HandleDirectoryScanned));
	}
}

void ULuaProjectItem::HandleDirectoryScanned(const FString& InPathName, ELuaProjectItemType::Type InType)
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
		ULuaProjectItem* NewItem = NewObject<ULuaProjectItem>(GetOutermost(), ULuaProjectItem::StaticClass());
		NewItem->Type = InType;
		NewItem->Path = InPathName;
		NewItem->Name = FPaths::GetCleanFilename(InPathName);
		if(InType != ELuaProjectItemType::Folder)
		{
			NewItem->Extension = FPaths::GetExtension(InPathName);
		}	

		Children.Add(NewItem);

		Children.Sort(
			[](const ULuaProjectItem& ItemA, const ULuaProjectItem& ItemB) -> bool
			{
				if(ItemA.Type != ItemB.Type)
				{
					return ItemA.Type < ItemB.Type;
				}

				return ItemA.Name.Compare(ItemB.Name) < 0;
			}
		);

		if(InType == ELuaProjectItemType::Folder)
		{
			// kick off another scan for subdirectories
			FDirectoryScanner::AddDirectory(InPathName, FOnDirectoryScanned::CreateUObject(NewItem, &ULuaProjectItem::HandleDirectoryScanned));

			// @TODO: now register for any changes to this directory if needed
		//	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::Get().LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		//	DirectoryWatcherModule.Get()->RegisterDirectoryChangedCallback_Handle(InPathName, IDirectoryWatcher::FDirectoryChanged::CreateUObject(NewItem, &ULuaProjectItem::HandleDirectoryChanged), OnDirectoryChangedHandle);
		}
	}
}

void ULuaProjectItem::HandleDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
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

