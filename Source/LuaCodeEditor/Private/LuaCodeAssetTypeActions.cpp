#include "LuaCodeAssetTypeActions.h"
#include "LuaCodeProject.h"
#include "LuaCodeProjectEditor.h"


#define LOCTEXT_NAMESPACE "LuaCodeAssetTypeActions"


FText FLuaCodeAssetTypeActions::GetName() const
{
	return LOCTEXT("LuaCodeProjectActionsName", "Lua Code Project");
}

FColor FLuaCodeAssetTypeActions::GetTypeColor() const
{
	return FColor(255, 255, 0);
}

UClass* FLuaCodeAssetTypeActions::GetSupportedClass() const
{
	return ULuaCodeProject::StaticClass();
}

void FLuaCodeAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (ULuaCodeProject* CodeProject = Cast<ULuaCodeProject>(*ObjIt))
		{
			TSharedRef<FLuaCodeProjectEditor> NewCodeProjectEditor(new FLuaCodeProjectEditor());
			NewCodeProjectEditor->InitCodeEditor(Mode, EditWithinLevelEditor, CodeProject);
		}
	}
}

uint32 FLuaCodeAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

#undef LOCTEXT_NAMESPACE
