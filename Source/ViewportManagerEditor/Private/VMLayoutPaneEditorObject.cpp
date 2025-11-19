// Copyright jackcayc924 2025. All Rights Reserved.

#include "VMLayoutPaneEditorObject.h"
#include "VMSplitLayoutAsset.h"

void UVMLayoutPaneEditorObject::Initialize(UVMSplitLayoutAsset* InLayoutAsset, int32 InPaneIndex)
{
	LayoutAsset = InLayoutAsset;
	PaneIndex = InPaneIndex;

	if (LayoutAsset && LayoutAsset->Panes.IsValidIndex(PaneIndex))
	{
		Pane = LayoutAsset->Panes[PaneIndex];
	}
}

void UVMLayoutPaneEditorObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Validate and clamp rect values to prevent freezing/invalid states
	Pane.Rect.Origin01.X = FMath::Clamp(Pane.Rect.Origin01.X, 0.f, 1.f);
	Pane.Rect.Origin01.Y = FMath::Clamp(Pane.Rect.Origin01.Y, 0.f, 1.f);
	Pane.Rect.Size01.X = FMath::Clamp(Pane.Rect.Size01.X, 0.01f, 1.f);
	Pane.Rect.Size01.Y = FMath::Clamp(Pane.Rect.Size01.Y, 0.01f, 1.f);

	// Ensure pane doesn't extend beyond canvas bounds
	if (Pane.Rect.Origin01.X + Pane.Rect.Size01.X > 1.f)
	{
		Pane.Rect.Size01.X = 1.f - Pane.Rect.Origin01.X;
	}
	if (Pane.Rect.Origin01.Y + Pane.Rect.Size01.Y > 1.f)
	{
		Pane.Rect.Size01.Y = 1.f - Pane.Rect.Origin01.Y;
	}

	ApplyToLayout();
}

void UVMLayoutPaneEditorObject::ApplyToLayout(bool bMarkDirty)
{
	if (!LayoutAsset || !LayoutAsset->Panes.IsValidIndex(PaneIndex))
	{
		return;
	}

	LayoutAsset->Modify();
	LayoutAsset->Panes[PaneIndex] = Pane;

	if (bMarkDirty && LayoutAsset->GetPackage())
	{
		LayoutAsset->MarkPackageDirty();
	}
}
