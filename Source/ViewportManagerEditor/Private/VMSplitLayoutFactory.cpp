#include "VMSplitLayoutFactory.h"
#include "AssetToolsModule.h"
#include "VMSplitLayoutAsset.h"
#include "VMLayoutAssetEditorToolkit.h"

#define LOCTEXT_NAMESPACE "VMSplitLayoutFactory"

UVMSplitLayoutFactory::UVMSplitLayoutFactory()
{
	SupportedClass = UVMSplitLayoutAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UVMSplitLayoutFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UVMSplitLayoutAsset* NewAsset = NewObject<UVMSplitLayoutAsset>(InParent, Class, Name, Flags);
	
	// Set up a default 3-pane layout as specified in the requirements
	if (NewAsset)
	{
		// Clear any existing panes
		NewAsset->Panes.Empty();

		// LP0 - "Weapon View" (3D)
		FVMSplitPane WeaponPane;
		WeaponPane.LocalPlayerIndex = 0;
		WeaponPane.Rect.Origin01 = FVector2f(0.33f, 0.05f);
		WeaponPane.Rect.Size01 = FVector2f(0.32f, 0.45f);
		WeaponPane.bReceivesKeyboardMouse = true;
		NewAsset->Panes.Add(WeaponPane);

		// LP1 - "Trajectory View" (3D)
		FVMSplitPane TrajectoryPane;
		TrajectoryPane.LocalPlayerIndex = 1;
		TrajectoryPane.Rect.Origin01 = FVector2f(0.66f, 0.05f);
		TrajectoryPane.Rect.Size01 = FVector2f(0.32f, 0.45f);
		TrajectoryPane.bReceivesKeyboardMouse = true;
		NewAsset->Panes.Add(TrajectoryPane);

		// LP2 - "Control Panel" (UMG-only)
		FVMSplitPane ControlPane;
		ControlPane.LocalPlayerIndex = 2;
		ControlPane.Rect.Origin01 = FVector2f(0.02f, 0.05f);
		ControlPane.Rect.Size01 = FVector2f(0.28f, 0.45f);
		ControlPane.bReceivesKeyboardMouse = false; // Control panel doesn't need camera input
		NewAsset->Panes.Add(ControlPane);

		// Set default properties
		NewAsset->bAutoSpawnPlayers = true;
	}

	return NewAsset;
}

bool UVMSplitLayoutFactory::ShouldShowInNewMenu() const
{
	return true;
}

// Asset Type Actions Implementation
FText FAssetTypeActions_VMSplitLayout::GetName() const
{
	return LOCTEXT("AssetTypeActions_VMSplitLayout", "VM Split Layout");
}

FColor FAssetTypeActions_VMSplitLayout::GetTypeColor() const
{
	return FColor(129, 196, 115); // Green color for viewport layouts
}

UClass* FAssetTypeActions_VMSplitLayout::GetSupportedClass() const
{
	return UVMSplitLayoutAsset::StaticClass();
}

uint32 FAssetTypeActions_VMSplitLayout::GetCategories()
{
	return EAssetTypeCategories::UI;
}

void FAssetTypeActions_VMSplitLayout::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		if (UVMSplitLayoutAsset* LayoutAsset = Cast<UVMSplitLayoutAsset>(Object))
		{
			TSharedRef<FVMLayoutAssetEditorToolkit> Toolkit = MakeShared<FVMLayoutAssetEditorToolkit>();
			Toolkit->Initialize(LayoutAsset, Mode, EditWithinLevelEditor);
		}
	}
}

FText FAssetTypeActions_VMSplitLayout::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetTypeActions_VMSplitLayoutDesc", "Defines viewport layout configuration with multiple local players");
}

#undef LOCTEXT_NAMESPACE


