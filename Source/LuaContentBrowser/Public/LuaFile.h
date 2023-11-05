// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LuaFile.generated.h"

UCLASS()
class LUACONTENTBROWSER_API ULuaFile : public UObject
{
	GENERATED_BODY()
	
public:
	FString EntryPath;
	FString Suffix;
};
