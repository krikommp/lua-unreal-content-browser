#pragma once

#include "Framework/Commands/Commands.h"

class FLuaCodeProjectEditorCommands : public TCommands<FLuaCodeProjectEditorCommands>
{
public:
	FLuaCodeProjectEditorCommands();

	TSharedPtr<FUICommandInfo> Save;
	TSharedPtr<FUICommandInfo> SaveAll;

	/** Initialize commands */
	virtual void RegisterCommands() override;
};
