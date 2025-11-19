// Copyright jackcayc924 2025. All Rights Reserved.

#include "VMLayoutEditorCommands.h"

#define LOCTEXT_NAMESPACE "VMLayoutEditorCommands"

FVMLayoutEditorCommands::FVMLayoutEditorCommands()
	: TCommands<FVMLayoutEditorCommands>(
		TEXT("VMLayoutEditor"),
		NSLOCTEXT("Contexts", "VMLayoutEditor", "VM Layout Editor"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FVMLayoutEditorCommands::RegisterCommands()
{
	UI_COMMAND(Duplicate, "Duplicate", "Duplicate selected pane", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control, EKeys::D));
	UI_COMMAND(Delete, "Delete", "Delete selected pane", EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
	UI_COMMAND(BringToFront, "Bring to Front", "Move pane to top layer", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(SendToBack, "Send to Back", "Move pane to bottom layer", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ToggleGrid, "Toggle Grid", "Show/hide alignment grid", EUserInterfaceActionType::ToggleButton, FInputChord(EModifierKey::Control, EKeys::G));
}

#undef LOCTEXT_NAMESPACE
