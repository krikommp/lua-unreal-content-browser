// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LuaProjectItem.generated.h"

UENUM()
namespace ELuaProjectItemType
{
	enum Type : int
	{
		Project,
		Folder,
		File
	};
}


UCLASS()
class LUACONTENTBROWSER_API ULuaProjectItem : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	void RescanChildren();

	void HandleDirectoryScanned(const FString& InPathName, ELuaProjectItemType::Type InType);

	/** Handle directory changing */
	void HandleDirectoryChanged(const TArray<struct FFileChangeData>& FileChanges);

public:
	UPROPERTY(Transient)
	TEnumAsByte<ELuaProjectItemType::Type> Type;

	UPROPERTY(Transient)
	FString Name;

	UPROPERTY(Transient)
	FString Extension;

	UPROPERTY(Transient)
	FString Path;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ULuaProjectItem>> Children;

	/** Delegate handle for directory watcher */
	FDelegateHandle OnDirectoryChangedHandle;
};
