#include "LuaContentBrowserUtil.h"
#include "Interfaces/IPluginManager.h"

static TMap<FName, FString> GPluginNameToRelativePathMap;

static FName GetRootNodeName(const FString InternalPath)
{
	const SIZE_T SearchEnd = InternalPath.Find("/", ESearchCase::IgnoreCase, ESearchDir::FromStart, 1);
	return *InternalPath.Mid(1, SearchEnd == -1 ? InternalPath.Len() : SearchEnd - 1);
}

TMap<FName, FString> LUA::GetPlugins()
{
	GPluginNameToRelativePathMap.Empty();
	GPluginNameToRelativePathMap.Add(FName("Game"), FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()).LeftChop(1));
	IPluginManager& PluginManager = IPluginManager::Get();
	TArray<TSharedRef<IPlugin>> Plugins = PluginManager.GetEnabledPlugins();
	for(const TSharedRef<IPlugin>& Plugin: Plugins)
	{
		GPluginNameToRelativePathMap.Add(FName(*Plugin->GetName()), Plugin->GetBaseDir());
	}

	return GPluginNameToRelativePathMap;
}

FName LUA::GetFileName(const FString& FilePath)
{
	FString FileName;
	FilePath.Split("/", nullptr, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	FileName.Split(".", &FileName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	return FName(*FileName);
}


FString LUA::ConvertInternalPathToAbsolutePath(const FString InternalPath)
{
	const FName RootNodeName = GetRootNodeName(InternalPath);
	FString Dir;
	if (RootNodeName.ToString().StartsWith("Classes")) Dir = "Source";
	else Dir = "Content";

	if (GPluginNameToRelativePathMap.IsEmpty())
		GetPlugins();

	const FString AbsolutePath = GPluginNameToRelativePathMap[RootNodeName] / Dir;

	return InternalPath.RightChop(1).Replace(*RootNodeName.ToString(), *AbsolutePath);
}
