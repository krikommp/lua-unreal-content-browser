#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LuaCodeEditorCustomization.generated.h"

USTRUCT()
struct FLuaCodeEditorTextCustomization
{
	GENERATED_USTRUCT_BODY()

	FLuaCodeEditorTextCustomization()
		: Font("")
		, Color(0.0f, 0.0f, 0.0f, 1.0f)
	{
	}

	UPROPERTY(EditAnywhere, Category=Text)
	FString Font;

	UPROPERTY(EditAnywhere, Category=Text)
	FLinearColor Color;
};

USTRUCT()
struct FLuaCodeEditorControlCustomization
{
	GENERATED_USTRUCT_BODY()

	FLuaCodeEditorControlCustomization()
		: Color(0.0f, 0.0f, 0.0f, 1.0f)
	{
	}

	UPROPERTY(EditAnywhere, Category=Controls)
	FLinearColor Color;
};

UCLASS(Config=Editor)
class ULuaCodeEditorCustomization : public UObject
{
	GENERATED_UCLASS_BODY()

	static const FLuaCodeEditorControlCustomization& GetControl(const FName& ControlCustomizationName);

	static const FLuaCodeEditorTextCustomization& GetText(const FName& TextCustomizationName);

private:
	UPROPERTY(EditAnywhere, EditFixedSize, Category=Controls)
	TArray<FLuaCodeEditorControlCustomization> Controls;

	UPROPERTY(EditAnywhere, EditFixedSize, Category=Text)
	TArray<FLuaCodeEditorTextCustomization> Text;
};
