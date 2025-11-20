// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "Toolkits/AssetEditorToolkit.h"

class UVMSplitLayoutAsset;
class SVMLayoutDesignerWidget;
class IDetailsView;

class FVMLayoutAssetEditorToolkit : public FAssetEditorToolkit
{
public:
	void Initialize(UVMSplitLayoutAsset* InAsset, const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost);

	// FAssetEditorToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetDocumentationLink() const override;

	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;

protected:
	TSharedRef<SDockTab> SpawnDesignerTab(const FSpawnTabArgs& Args);
	TSharedRef<SWidget> BuildDesignerContent();

private:
	static const FName DesignerTabId;

	TObjectPtr<UVMSplitLayoutAsset> EditingAsset = nullptr;
	TSharedPtr<SVMLayoutDesignerWidget> DesignerWidget;
	TSharedPtr<IDetailsView> AssetDetailsView;
};

