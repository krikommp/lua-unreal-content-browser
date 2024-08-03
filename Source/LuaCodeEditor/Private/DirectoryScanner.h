#pragma once

#include "LuaCodeProjectItem.h"

DECLARE_DELEGATE_TwoParams(FOnDirectoryScanned, const FString& /*InPathName*/, ELuaCodeProjectItemType::Type /*InType*/);

class FDirectoryScanner
{
public:
	static bool Tick();

	static void AddDirectory(const FString& PathName, const FOnDirectoryScanned& OnDirectoryScanned);

	static bool IsScanning() ;

public:
	static TArray<struct FDirectoryScannerCommand*> CommandQueue;

	static bool bDataDirty;
};
