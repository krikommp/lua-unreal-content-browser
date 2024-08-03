#pragma once

#include "LuaCodeProjectItem.generated.h"

/** Types of project items. Note that the enum ordering determines the tree sorting */
UENUM()
namespace ELuaCodeProjectItemType
{
	enum Type : int
	{
		Project,
		Folder,
		File
	};
}

UCLASS()
class LUACODEEDITOR_API ULuaCodeProjectItem : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	void RescanChildren();

	void HandleDirectoryScanned(const FString& InPathName, ELuaCodeProjectItemType::Type InType);

	/** Handle directory changing */
	void HandleDirectoryChanged(const TArray<struct FFileChangeData>& FileChanges);

public:
	UPROPERTY(Transient)
	TEnumAsByte<ELuaCodeProjectItemType::Type> Type;

	UPROPERTY(Transient)
	FString Name;

	UPROPERTY(Transient)
	FString Extension;

	UPROPERTY(Transient)
	FString Path;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ULuaCodeProjectItem>> Children;

	/** Delegate handle for directory watcher */
	FDelegateHandle OnDirectoryChangedHandle;
};
