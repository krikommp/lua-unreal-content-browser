#include "LuaCodeProjectFactory.h"
#include "LuaCodeProject.h"


#define LOCTEXT_NAMESPACE "LuaCodeEditor"


ULuaCodeProjectFactory::ULuaCodeProjectFactory(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = ULuaCodeProject::StaticClass();
}


UObject* ULuaCodeProjectFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	ULuaCodeProject* NewCodeProject = NewObject<ULuaCodeProject>(InParent, Class, Name, Flags);
	return NewCodeProject;
}


#undef LOCTEXT_NAMESPACE
