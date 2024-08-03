#pragma once

namespace LUA
{
	class FDirectoryEnumerator : public IPlatformFile::FDirectoryVisitor
	{
	public:
		TArray<FString> Directories;
		TArray<FString> Files;

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			const FString FileName = FString(FilenameOrDirectory);
			if (bIsDirectory)
			{
				if (FileName.Contains(".") || FileName.Contains("_")) return true;
				Directories.Add(FilenameOrDirectory);
			}
			else
			{
				if (FileName.EndsWith(".lua") || FileName.EndsWith(".luac"))
				{
					Files.Add(FilenameOrDirectory);
				}
			}
			return true;
		}
	};
	
	FString ConvertInternalPathToAbsolutePath(const FString InternalPath);
	TMap<FName, FString> GetPlugins();
	FName GetFileName(const FString& FilePath);
}