#pragma once

class ULuaFile;

enum class ELuaScriptHierarchyNodeType
{
	Folder,
	Script,
};

using FLuaScriptHierarchyNodeKey = TPair<FName, ELuaScriptHierarchyNodeType>;
struct FLuaScriptHierarchyNode
{
	FName EntryName;
	FString EntryPath;
	ELuaScriptHierarchyNodeType NodeType;
	TObjectPtr<ULuaFile> Script;
	TMap<FLuaScriptHierarchyNodeKey, TSharedPtr<FLuaScriptHierarchyNode>> Children;

	static TSharedPtr<FLuaScriptHierarchyNode> MakeNodeEntry(const FName InEntryName, const FString& InEntryPath, const ELuaScriptHierarchyNodeType InType, const FString& InSuffix = "");
	void AddChild(TSharedPtr<FLuaScriptHierarchyNode> ChildEntry);
};

struct FLuaScriptHierarchyFolderInfo
{
	FName FolderName;
	FString AbsolutePath;
	FDateTime LastWriteTime;
};

struct FLuaScriptHierarchyFilter
{
	FName InEntryPath;
	bool IncludeDeveloper;
	bool IncludeCollections;
};

class FLuaScriptHierarchy
{
	using ThisType = FLuaScriptHierarchy;
	
	void OnModulesChanged(FName ModuleName, EModuleChangeReason Reason);
	// 刷新脚本目录
	void PopulateHierarchy();
	void BuildNode(const FString& RelativePath, const FName PluginName);
	void BuildNodeRecursive(const FString& FolderPath, const TSharedPtr<FLuaScriptHierarchyNode> RootNode, bool bIsRefresh = false);

	void StartupWatcher();
	void CleanupWatcher();

	TSharedPtr<FLuaScriptHierarchyNode> GetNode(const FString& InternalPath);
public:
	FLuaScriptHierarchy();
	~FLuaScriptHierarchy();

private:
	TMap<FName, TSharedPtr<FLuaScriptHierarchyNode>> RootNodes;
	TMap<FString, FLuaScriptHierarchyFolderInfo> CachedFolders;
	TArray<TObjectPtr<ULuaFile>> WaitToSyncFiles;
	TArray<FName> WaitToSyncFolders;
	TMap<FString, FLuaScriptHierarchyFolderInfo> ModifiedFolders;

public:
	void GetScriptRoots(TArray<FName>& OutScriptRoots);
	bool NeedRefresh();
	void RefreshHierarchy(TArray<FName>& OutWaitToAsyncFolders, TArray<TObjectPtr<ULuaFile>>& OutWaitToAsyncFiles);
	void GetMatchingFolders(const FLuaScriptHierarchyFilter& Filter, TArray<FString>& OutFolders);
	void GetMatchingFiles(const FLuaScriptHierarchyFilter& Filter, TArray<TObjectPtr<ULuaFile>>& OutFiles);
	ULuaFile* ScriptPreCreate(const FString& InEntryPath, const FName InEntryName);
	void ScriptCreate(const FString& EntryPath, const FString& InProposedName);
	FSimpleMulticastDelegate OnScriptHierarchyChanged;
};