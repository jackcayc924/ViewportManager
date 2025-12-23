// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FVMLayoutEditorCommands : public TCommands<FVMLayoutEditorCommands>
{
public:
	FVMLayoutEditorCommands();

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> Duplicate;
	TSharedPtr<FUICommandInfo> Delete;
	TSharedPtr<FUICommandInfo> ToggleGrid;
};
