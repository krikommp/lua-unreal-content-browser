#include "LuaScriptHierarchy.h"

#include "DirectoryWatcherModule.h"
#include "LuaContentBrowserUtil.h"
#include "LuaFile.h"

static UPackage* Root = nullptr;
static TMap<FString, TObjectPtr<UPackage>> EntryToPackageMap;
TSharedPtr<FLuaScriptHierarchyNode> FLuaScriptHierarchyNode::MakeNodeEntry(const FName InEntryName,
                                                                           const FString& InEntryPath,
                                                                           const ELuaScriptHierarchyNodeType InType,
                                                                           const FString& InSuffix)
{
	if (Root == nullptr)
	{
		Root = NewObject<UPackage>(GetTransientPackage(), "LuaScriptHierarchyRoot");
		Root->AddToRoot();
	}
	const FString ParentPath = InEntryPath.LeftChop(InEntryName.ToString().Len() + 1) + (
		InSuffix == "" ? "" : "." + InSuffix);
	UPackage* Package;
	if (ParentPath.IsEmpty())
	{
		Package = NewObject<UPackage>(Root, *InEntryPath);
		EntryToPackageMap.Add(ParentPath, Package);
	}
	else
	{
		Package = EntryToPackageMap.FindOrAdd(*ParentPath);
		if (Package == nullptr)
		{
			Package = NewObject<UPackage>(Root, *ParentPath);
			EntryToPackageMap.Add(ParentPath, Package);
		}
		if (InType == ELuaScriptHierarchyNodeType::Folder)
		{
			UPackage* TempPackage = NewObject<UPackage>(Package, *InEntryPath);
			TempPackage->AddToRoot();
			EntryToPackageMap.Add(InEntryPath, TempPackage);
		}
	}
	Package->AddToRoot();

	TSharedPtr<FLuaScriptHierarchyNode> NewEntry = MakeShared<FLuaScriptHierarchyNode>();
	NewEntry->EntryName = InEntryName;
	NewEntry->EntryPath = InEntryPath;
	NewEntry->NodeType = ELuaScriptHierarchyNodeType::Folder;
	NewEntry->Script = nullptr;

	if (InType != ELuaScriptHierarchyNodeType::Folder)
	{
		NewEntry->NodeType = InType;
		switch (InType)
		{
		case ELuaScriptHierarchyNodeType::Script:
			NewEntry->Script = NewObject<ULuaFile>(Package, InEntryName);
			break;
		default:
			break;
		}
		NewEntry->Script->Path = InEntryPath + "." + InSuffix;
		NewEntry->Script->Extension = InSuffix;
		NewEntry->Script->Name = InEntryName.ToString();
		NewEntry->Script->AddToRoot();
	}

	return NewEntry;
}

void FLuaScriptHierarchyNode::AddChild(TSharedPtr<FLuaScriptHierarchyNode> ChildEntry)
{
	check(NodeType == ELuaScriptHierarchyNodeType::Folder);
	Children.Add(FLuaScriptHierarchyNodeKey(ChildEntry->EntryName, ChildEntry->NodeType), ChildEntry);
}

FLuaScriptHierarchy::FLuaScriptHierarchy()
{
	FModuleManager::Get().OnModulesChanged().AddRaw(this, &ThisType::OnModulesChanged);

	CachedFolders.Empty();
	WaitToSyncFiles.Empty();
	WaitToSyncFolders.Empty();

	PopulateHierarchy();
	StartupWatcher();
}

FLuaScriptHierarchy::~FLuaScriptHierarchy()
{
	FModuleManager::Get().OnModulesChanged().RemoveAll(this);
	RootNodes.Empty();
	Root->RemoveFromRoot();
	CleanupWatcher();
}

void FLuaScriptHierarchy::GetScriptRoots(TArray<FName>& OutScriptRoots)
{
	for (const auto& Node : RootNodes)
	{
		FString RootPath = Node.Value->EntryPath;
		OutScriptRoots.Add(*RootPath);
	}
}

bool FLuaScriptHierarchy::NeedRefresh()
{
	bool NeedRefresh = false;
	if (CachedFolders.IsEmpty())
	{
		PopulateHierarchy();
		return NeedRefresh;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TArray<FString> WaitToRemoveFolders;
	for (auto& Folder : CachedFolders)
	{
		FString FolderPath = Folder.Value.AbsolutePath;
		if (!PlatformFile.DirectoryExists(*FolderPath))
		{
			WaitToRemoveFolders.Add(Folder.Key);
			continue;
		}

		FDateTime NowTime = PlatformFile.GetTimeStamp(*FolderPath);
		if (NowTime != Folder.Value.LastWriteTime)
		{
			Folder.Value.LastWriteTime = NowTime;
			ModifiedFolders.Add(Folder);
			NeedRefresh = true;
		}
	}

	for (const auto& Folder : WaitToRemoveFolders)
	{
		CachedFolders.Remove(Folder);
	}

	return NeedRefresh;
}

TSharedPtr<FLuaScriptHierarchyNode> FLuaScriptHierarchy::GetNode(const FString& InternalPath)
{
	TArray<FString> HierarchyPathParts;
	InternalPath.ParseIntoArray(HierarchyPathParts, TEXT("/"), true);
	TSharedPtr<FLuaScriptHierarchyNode> CurrentNode;

	for (decltype(HierarchyPathParts)::SizeType i = 0; i < HierarchyPathParts.Num(); ++i)
	{
		const FString& HierarchyPathPart = HierarchyPathParts[i];
		const FName HierarchyPathPartName = FName(*HierarchyPathPart);

		if (i == 0)
		{
			TSharedPtr<FLuaScriptHierarchyNode>& RootNode = RootNodes.FindOrAdd(HierarchyPathPartName);
			if (!RootNode.IsValid()) return RootNode;

			CurrentNode = RootNode;
			continue;;
		}

		TSharedPtr<FLuaScriptHierarchyNode>& ChildNode = CurrentNode->Children.FindOrAdd(
			FLuaScriptHierarchyNodeKey(HierarchyPathPartName, ELuaScriptHierarchyNodeType::Folder));
		if (!ChildNode.IsValid()) return ChildNode;
		CurrentNode = ChildNode;
	}

	return CurrentNode;
}


void FLuaScriptHierarchy::OnModulesChanged(FName ModuleName, EModuleChangeReason Reason)
{
}

void FLuaScriptHierarchy::StartupWatcher()
{
	CleanupWatcher();
}

void FLuaScriptHierarchy::CleanupWatcher()
{
}

void FLuaScriptHierarchy::PopulateHierarchy()
{
	RootNodes.Empty();
	CachedFolders.Empty();

	const FString FullProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	for (const auto& [PluginName, RelativePath] : LUA::GetPlugins())
	{
		BuildNode(RelativePath, PluginName);
	}

	OnScriptHierarchyChanged.Broadcast();
}

void FLuaScriptHierarchy::BuildNode(const FString& RelativePath, const FName PluginName)
{
	const FString FolderPath = FPaths::ConvertRelativePathToFull(RelativePath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	LUA::FDirectoryEnumerator DirectoryEnumerator;
	PlatformFile.IterateDirectory(*FolderPath, DirectoryEnumerator);

	auto ConvertAbsolutePathToInternalPathAndReturnRootNodeName = [
		](const FString& AbsolutePath, const FName PluginName, FString& InternalPath)
	{
		FName RootNodeName = NAME_None;
		if (const auto& SearchEndIdx = AbsolutePath.Find("/Content"); SearchEndIdx != INDEX_NONE)
		{
			RootNodeName = PluginName;
			InternalPath = AbsolutePath.RightChop(SearchEndIdx + 8);
		}
		return RootNodeName;
	};

	for (const auto& Dir : DirectoryEnumerator.Directories)
	{
		FString InternalPath;
		const FName RootNodeName =
			ConvertAbsolutePathToInternalPathAndReturnRootNodeName(Dir, PluginName, InternalPath);
		if (RootNodeName == NAME_None) continue;

		TSharedPtr<FLuaScriptHierarchyNode>& RootNode = RootNodes.FindOrAdd(RootNodeName);
		if (!RootNode.IsValid())
		{
			RootNode = FLuaScriptHierarchyNode::MakeNodeEntry(RootNodeName, TEXT("/") + RootNodeName.ToString(),
			                                                  ELuaScriptHierarchyNodeType::Folder);
			BuildNodeRecursive(Dir, RootNode);
		}
	}
}

static void GetFileNameAndSuffix(const FString& FilePath, FName& FileName, FString& FileSuffix)
{
	FString TempFileName;
	FilePath.Split("/", nullptr, &TempFileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	TempFileName.Split(".", &TempFileName, &FileSuffix, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	FileName = FName(*TempFileName);
}

static FName GetFileName(const FString FilePath)
{
	FString FileName;
	FilePath.Split("/", nullptr, &FileName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	FileName.Split(".", &FileName, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	return FName(*FileName);
}

void FLuaScriptHierarchy::BuildNodeRecursive(const FString& FolderPath,
                                             const TSharedPtr<FLuaScriptHierarchyNode> RootNode, bool bIsRefresh)
{
	const FString InternalPath = RootNode->EntryPath;

	LUA::FDirectoryEnumerator DirectoryEnumerator;
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.IterateDirectory(*FolderPath, DirectoryEnumerator);

	FLuaScriptHierarchyFolderInfo FolderInfo;
	FolderInfo.FolderName = RootNode->EntryName;
	FolderInfo.AbsolutePath = FolderPath;
	FolderInfo.LastWriteTime = PlatformFile.GetTimeStamp(*FolderPath);

	CachedFolders.Add(InternalPath, FolderInfo);

	for (const auto& File : DirectoryEnumerator.Files)
	{
		FName FileName;
		FString FileSuffix;
		GetFileNameAndSuffix(File, FileName, FileSuffix);
		if (!FileSuffix.Equals("lua") && !FileSuffix.Equals("luac"))
			continue;
		ELuaScriptHierarchyNodeType NodeType = ELuaScriptHierarchyNodeType::Script;
		const auto Node = FLuaScriptHierarchyNode::MakeNodeEntry(FileName, InternalPath / FileName.ToString(), NodeType,
		                                                         FileSuffix);
		if (bIsRefresh)
			WaitToSyncFiles.Add(Node->Script);
		RootNode->AddChild(Node);
	}

	for (const auto& Dir : DirectoryEnumerator.Directories)
	{
		FName DirName = GetFileName(Dir);
		FString EntryPath = InternalPath / DirName.ToString();
		RootNode->AddChild(
			FLuaScriptHierarchyNode::MakeNodeEntry(DirName, EntryPath, ELuaScriptHierarchyNodeType::Folder));
		if (bIsRefresh)
			WaitToSyncFolders.Add(*EntryPath);
		BuildNodeRecursive(
			Dir, *RootNode->Children.Find(FLuaScriptHierarchyNodeKey(DirName, ELuaScriptHierarchyNodeType::Folder)));
	}
}

void FLuaScriptHierarchy::RefreshHierarchy(TArray<FName>& OutWaitToAsyncFolders, TArray<TObjectPtr<ULuaFile>>& OutWaitToAsyncFiles)
{
	WaitToSyncFiles.Empty();
	WaitToSyncFolders.Empty();

	for (const auto& ModifiedFolder : ModifiedFolders)
	{
		const TSharedPtr<FLuaScriptHierarchyNode> FolderNode = GetNode(ModifiedFolder.Key);
		if (!FolderNode.IsValid()) continue;
		LUA::FDirectoryEnumerator DirectoryEnumerator;
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.IterateDirectory(*ModifiedFolder.Value.AbsolutePath, DirectoryEnumerator);

		TArray<FLuaScriptHierarchyNodeKey> WaitToDeleteNodes;
		for (const auto& Node : FolderNode->Children)
		{
			if (Node.Value->NodeType != ELuaScriptHierarchyNodeType::Folder)
			{
				FString Path = LUA::ConvertInternalPathToAbsolutePath(Node.Value->Script->Path);
				if (PlatformFile.FileExists(*Path)) continue;

				WaitToDeleteNodes.Add(Node.Key);
			}
			else
			{
				FString Path = LUA::ConvertInternalPathToAbsolutePath(Node.Value->EntryPath);
				if (PlatformFile.DirectoryExists(*Path)) continue;

				WaitToDeleteNodes.Add(Node.Key);
				WaitToSyncFolders.Add(*Node.Value->EntryPath);
			}
		}

		if (!WaitToDeleteNodes.IsEmpty())
		{
			for (auto& NodeKey : WaitToDeleteNodes)
			{
				FolderNode->Children.Remove(NodeKey);
			}
		}

		for (const auto& File : DirectoryEnumerator.Files)
		{
			FName FileName;
			FString FileSuffix;
			GetFileNameAndSuffix(File, FileName, FileSuffix);
			if (!FileSuffix.Equals("lua") && !FileSuffix.Equals("luac"))
				continue;
			ELuaScriptHierarchyNodeType NodeType = ELuaScriptHierarchyNodeType::Script;
			if (!FolderNode->Children.Contains(FLuaScriptHierarchyNodeKey(FileName, NodeType)))
			{
				const auto Node = FLuaScriptHierarchyNode::MakeNodeEntry(FileName,
				                                                         FolderNode->EntryPath / FileName.ToString(),
				                                                         NodeType, FileSuffix);
				WaitToSyncFiles.Add(Node->Script);
				FolderNode->AddChild(Node);
			}
		}

		for (const auto& Dir : DirectoryEnumerator.Directories)
		{
			const FName DirName = GetFileName(Dir);
			if (!FolderNode->Children.Contains(FLuaScriptHierarchyNodeKey(DirName, ELuaScriptHierarchyNodeType::Folder)))
			{
				FString EntryPath = FolderNode->EntryPath / DirName.ToString();
				FolderNode->AddChild(FLuaScriptHierarchyNode::MakeNodeEntry(DirName, EntryPath, ELuaScriptHierarchyNodeType::Folder));
				BuildNodeRecursive(Dir, *FolderNode->Children.Find(FLuaScriptHierarchyNodeKey(DirName, ELuaScriptHierarchyNodeType::Folder)), false);
				WaitToSyncFolders.Add(*EntryPath);
			}
		}
	}

	OutWaitToAsyncFolders = WaitToSyncFolders;
	OutWaitToAsyncFiles = WaitToSyncFiles;
	ModifiedFolders.Empty();
}

void FLuaScriptHierarchy::GetMatchingFolders(const FLuaScriptHierarchyFilter& Filter, TArray<FString>& OutFolders)
{
	const FName& EntryPath = Filter.InEntryPath;
	if (EntryPath == "/")
	{
		TQueue<TSharedPtr<FLuaScriptHierarchyNode>> RootNodeQueue;
		for (auto& RootNode : RootNodes)
		{
			RootNodeQueue.Enqueue(RootNode.Value);
		}
		TSharedPtr<FLuaScriptHierarchyNode> CurrentNode;
		while (RootNodeQueue.Dequeue(CurrentNode))
		{
			if (CurrentNode == nullptr || !CurrentNode.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("CurrentNode is nullptr"));
				continue;
			}
			for (auto& ChildNode : CurrentNode->Children)
			{
				if (ChildNode.Value->EntryPath.Contains("Developers") && !Filter.IncludeDeveloper) continue;
				if (ChildNode.Value->EntryPath.Contains("Collections") && !Filter.IncludeCollections) continue;

				RootNodeQueue.Enqueue(ChildNode.Value);
				if (ChildNode.Value->NodeType == ELuaScriptHierarchyNodeType::Folder)
					OutFolders.Add(ChildNode.Value->EntryPath);
			}
		}
		return;
	}

	const TSharedPtr<FLuaScriptHierarchyNode>* CurrentNode = nullptr;
	TArray<FString> HierarchyPathParts;
	EntryPath.ToString().ParseIntoArray(HierarchyPathParts, TEXT("/"), true);

	for (decltype(HierarchyPathParts)::SizeType i = 0; i < HierarchyPathParts.Num(); ++i)
	{
		const FString& HierarchyPathPart = HierarchyPathParts[i];
		const FName HierarchyPathPartName = FName(*HierarchyPathPart);

		if (i == 0)
		{
			CurrentNode = RootNodes.Find(HierarchyPathPartName);
			if (CurrentNode == nullptr || !CurrentNode->IsValid()) return;
			continue;
		}

		const TSharedPtr<FLuaScriptHierarchyNode>* ChildNode = CurrentNode->Get()->Children.Find(FLuaScriptHierarchyNodeKey(HierarchyPathPartName, ELuaScriptHierarchyNodeType::Folder));
		if (ChildNode == nullptr || !ChildNode->IsValid()) return;
		CurrentNode = ChildNode;
	}

	check(CurrentNode != nullptr);

	for (const auto& Child : CurrentNode->Get()->Children)
	{
		if (Child.Value->EntryPath.Contains(TEXT("Developers"))) continue;
		if (Child.Value->EntryPath.Contains(TEXT("Collections"))) continue;

		if (Child.Value->NodeType == ELuaScriptHierarchyNodeType::Folder)
		{
			OutFolders.Add(Child.Value->EntryPath);
		}
	}
}

void FLuaScriptHierarchy::GetMatchingFiles(const FLuaScriptHierarchyFilter& Filter,
	TArray<TObjectPtr<ULuaFile>>& OutFiles)
{
	const FName& EntryPath = Filter.InEntryPath;
	if (EntryPath == "/")
	{
		TQueue<TSharedPtr<FLuaScriptHierarchyNode>> RootNodeQueue = TQueue<TSharedPtr<FLuaScriptHierarchyNode>>();
		for (auto& RootNode : RootNodes)
		{
			if (RootNode.Value == nullptr || !RootNode.Value.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("%s is null or invalid."), *RootNode.Key.ToString());
			}

			RootNodeQueue.Enqueue(RootNode.Value);
		}
		{
			TSharedPtr<FLuaScriptHierarchyNode> CurrentNode;
			while (RootNodeQueue.Dequeue(CurrentNode))
			{
				if (CurrentNode == nullptr || !CurrentNode.IsValid())
				{
					UE_LOG(LogTemp, Error, TEXT("CurrentNode is nullptr"));
					continue;
				}
				for (auto& Children : CurrentNode->Children)
				{
					RootNodeQueue.Enqueue(Children.Value);
					if (Children.Value->NodeType != ELuaScriptHierarchyNodeType::Folder)
					{
						OutFiles.Add(Children.Value->Script);
					}
				}
			}
		}
		return;
	}

	const TSharedPtr<FLuaScriptHierarchyNode>* CurrentNode = nullptr;
	TArray<FString> HierarchyPathParts;
	EntryPath.ToString().ParseIntoArray(HierarchyPathParts, TEXT("/"), true);

	for (int i = 0; i < HierarchyPathParts.Num(); i++)
	{
		const FString& HierarchyPathPart = HierarchyPathParts[i];
		const FName HierarchyPathPartName = *HierarchyPathPart;

		if (i == 0)
		{
			CurrentNode = RootNodes.Find(HierarchyPathPartName);
			if (CurrentNode == nullptr || !CurrentNode->IsValid()) return;
			continue;
		}

		const TSharedPtr<FLuaScriptHierarchyNode>* ChildNode = CurrentNode->Get()->Children.Find(FLuaScriptHierarchyNodeKey(HierarchyPathPartName, ELuaScriptHierarchyNodeType::Folder));
		if (ChildNode == nullptr || !ChildNode->IsValid()) return;
		CurrentNode = ChildNode;
 	}

	for (const auto& Child : CurrentNode->Get()->Children)
	{
		if (Child.Value->NodeType != ELuaScriptHierarchyNodeType::Folder)
		{
			OutFiles.Add(Child.Value->Script);
		}
	}
}

ULuaFile* FLuaScriptHierarchy::ScriptPreCreate(const FString& InEntryPath, const FName InEntryName)
{
	if (EntryToPackageMap.Contains(InEntryPath))
	{
		UPackage* Package = EntryToPackageMap.FindChecked(*InEntryPath);
		ULuaFile* Script = NewObject<ULuaFile>(Package, InEntryName);
		Script->Path = InEntryPath / InEntryName.ToString() + ".lua";
		Script->Name = InEntryName.ToString();
		Script->Extension = ".lua";
		Script->AddToRoot();
		return Script;
	}
	return nullptr;
}

void FLuaScriptHierarchy::ScriptCreate(const FString& EntryPath, const FString& InProposedName)
{
	const FName OldFileName = LUA::GetFileName(EntryPath);;
	const FString SavePath = EntryPath.Replace(*OldFileName.ToString(), *InProposedName);
	const FString AbsolutePath = LUA::ConvertInternalPathToAbsolutePath(EntryPath);
	static FString LuaTemplate = "";
	FFileHelper::SaveStringToFile(LuaTemplate, *AbsolutePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(),
								  FILEWRITE_Append);
}
