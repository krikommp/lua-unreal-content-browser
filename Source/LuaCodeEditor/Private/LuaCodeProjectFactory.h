#pragma once

#include "Factories/Factory.h"
#include "LuaCodeProjectFactory.generated.h"

UCLASS()
class LUACODEEDITOR_API ULuaCodeProjectFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
