// Copyright jackcayc924 2025. All Rights Reserved.

#include "VMLayoutAssetEditorToolkit.h"

#include "SVMLayoutDesignerWidget.h"
#include "VMSplitLayoutAsset.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"

const FName FVMLayoutAssetEditorToolkit::DesignerTabId(TEXT("ViewportManager_LayoutDesignerTab"));

void FVMLayoutAssetEditorToolkit::Initialize(UVMSplitLayoutAsset* InAsset, const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	EditingAsset = InAsset;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsArgs;
	DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsArgs.bAllowSearch = true;
	DetailsArgs.bHideSelectionTip = true;

	AssetDetailsView = PropertyModule.CreateDetailView(DetailsArgs);
	AssetDetailsView->SetObject(InAsset);

	DesignerWidget = SNew(SVMLayoutDesignerWidget)
		.InitialAsset(InAsset)
		.bShowAssetPicker(false);

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Standalone_VMLayoutAssetEditor_Layout_v1")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation(Orient_Horizontal)
		->Split
		(
			FTabManager::NewStack()
			->AddTab(DesignerTabId, ETabState::OpenedTab)
			->SetHideTabWell(true)
		)
	);

	InitAssetEditor(Mode, InitToolkitHost, TEXT("ViewportManagerLayoutAssetEditor"), Layout, true, true, { InAsset });

	RegenerateMenusAndToolbars();
}

void FVMLayoutAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(DesignerTabId, FOnSpawnTab::CreateSP(this, &FVMLayoutAssetEditorToolkit::SpawnDesignerTab))
		.SetDisplayName(NSLOCTEXT("ViewportManager", "LayoutDesignerTab", "Layout Designer"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewport"));
}

void FVMLayoutAssetEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(DesignerTabId);
}

TSharedRef<SDockTab> FVMLayoutAssetEditorToolkit::SpawnDesignerTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == DesignerTabId);

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
		.Label(NSLOCTEXT("ViewportManager", "LayoutDesignerTabTitle", "Layout Designer"))
		[
			BuildDesignerContent()
		];

	SpawnedTab->SetTabIcon(FAppStyle::Get().GetBrush("LevelEditor.Tabs.Viewport"));

	return SpawnedTab;
}

TSharedRef<SWidget> FVMLayoutAssetEditorToolkit::BuildDesignerContent()
{
	// Simply return the designer widget - it already has the layout preview on left
	// and pane details panel on right
	return DesignerWidget.ToSharedRef();
}

FName FVMLayoutAssetEditorToolkit::GetToolkitFName() const
{
	return FName("ViewportManagerLayoutAssetEditor");
}

FText FVMLayoutAssetEditorToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("ViewportManager", "LayoutAssetEditorAppLabel", "Viewport Layout Asset");
}

FString FVMLayoutAssetEditorToolkit::GetWorldCentricTabPrefix() const
{
	return TEXT("Viewport Layout");
}

FLinearColor FVMLayoutAssetEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.1f, 0.35f, 0.6f, 0.5f);
}

FString FVMLayoutAssetEditorToolkit::GetDocumentationLink() const
{
	return TEXT("https://docs.unrealengine.com/");
}






