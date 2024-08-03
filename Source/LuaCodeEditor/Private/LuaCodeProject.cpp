#include "LuaCodeProject.h"
#include "Misc/Paths.h"

ULuaCodeProject::ULuaCodeProject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Path = FPaths::ProjectContentDir();
}
