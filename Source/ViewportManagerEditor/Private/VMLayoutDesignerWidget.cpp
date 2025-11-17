#include "VMLayoutDesignerWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/SpinBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Engine/Engine.h"
#include "EditorUtilitySubsystem.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "VMLog.h"

UVMLayoutDesignerWidget::UVMLayoutDesignerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrentLayoutAsset = nullptr;
	SelectedPaneIndex = -1;
}

void UVMLayoutDesignerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Create design canvas if not bound in Blueprint
	if (!DesignCanvas)
	{
		DesignCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DesignCanvas"));
		if (WidgetTree->RootWidget)
		{
			if (UPanelWidget* RootPanel = Cast<UPanelWidget>(WidgetTree->RootWidget))
			{
				RootPanel->AddChild(DesignCanvas);
			}
		}
		else
		{
			WidgetTree->RootWidget = DesignCanvas;
		}
	}
	
	// Bind button events
	BindButtonEvents();
	
	// Update status
	UpdateStatusText();
	
	UE_LOG(LogViewportManager, Log, TEXT("UVMLayoutDesignerWidget::NativeConstruct - Layout designer initialized"));
}

void UVMLayoutDesignerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	// Update visuals if needed (could add dirty flag optimization)
}

void UVMLayoutDesignerWidget::SetLayoutAsset(UVMSplitLayoutAsset* InLayoutAsset)
{
	CurrentLayoutAsset = InLayoutAsset;
	SelectedPaneIndex = -1;
	
	RefreshLayout();
	UpdateStatusText();
	
	UE_LOG(LogViewportManager, Log, TEXT("UVMLayoutDesignerWidget::SetLayoutAsset - Set layout asset: %s"), 
		InLayoutAsset ? *InLayoutAsset->GetName() : TEXT("None"));
}

void UVMLayoutDesignerWidget::RefreshLayout()
{
	if (!DesignCanvas)
		return;
	
	// Clear existing visuals
	for (UBorder* Visual : PaneVisuals)
	{
		if (Visual)
		{
			DesignCanvas->RemoveChild(Visual);
		}
	}
	PaneVisuals.Empty();
	
	// Create new visuals if we have a layout asset
	if (CurrentLayoutAsset)
	{
		CreatePaneVisuals();
	}
}

void UVMLayoutDesignerWidget::AddNewPane()
{
	if (!CurrentLayoutAsset)
		return;
	
	// Find next available local player index
	TSet<int32> UsedIndices;
	for (const FVMSplitPane& Pane : CurrentLayoutAsset->Panes)
	{
		UsedIndices.Add(Pane.LocalPlayerIndex);
	}
	
	int32 NewIndex = 0;
	while (UsedIndices.Contains(NewIndex))
	{
		++NewIndex;
	}
	
	// Create new pane with default settings
	FVMSplitPane NewPane;
	NewPane.LocalPlayerIndex = NewIndex;
	const int32 ExistingPaneCount = CurrentLayoutAsset->Panes.Num();
	const int32 ProposedPaneCount = ExistingPaneCount + 1;
	const int32 Columns = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(ProposedPaneCount))));
	const int32 Rows = FMath::Max(1, FMath::CeilToInt(static_cast<float>(ProposedPaneCount) / Columns));
	const int32 GridIndex = ExistingPaneCount;
	const int32 ColumnIndex = GridIndex % Columns;
	const int32 RowIndex = GridIndex / Columns;
	const FVector2f CellSize(1.f / Columns, 1.f / Rows);
	const FVector2f Origin(CellSize.X * ColumnIndex, CellSize.Y * RowIndex);
	NewPane.Rect.Origin01 = Origin;
	NewPane.Rect.Size01 = CellSize;
	NewPane.ViewportHUDClass = nullptr;
	NewPane.PawnClass = nullptr;
	NewPane.bUseCustomCameraTransform = false;
	NewPane.bUseCustomFocusPoint = false;
	NewPane.FocusPoint = FVector::ZeroVector;
	NewPane.OrbitDistance = 500.0f;
	NewPane.CameraTransform = FTransform::Identity;
	
	CurrentLayoutAsset->Panes.Add(NewPane);
	CurrentLayoutAsset->MarkPackageDirty();
	
	RefreshLayout();
	UpdateStatusText();
	
	UE_LOG(LogViewportManager, Log, TEXT("UVMLayoutDesignerWidget::AddNewPane - Added new pane for LocalPlayer %d"), NewIndex);
}

void UVMLayoutDesignerWidget::RemoveSelectedPane()
{
	if (!CurrentLayoutAsset || SelectedPaneIndex < 0 || SelectedPaneIndex >= CurrentLayoutAsset->Panes.Num())
		return;
	
	CurrentLayoutAsset->Panes.RemoveAt(SelectedPaneIndex);
	CurrentLayoutAsset->MarkPackageDirty();
	
	SelectedPaneIndex = -1;
	RefreshLayout();
	UpdateStatusText();
	
	UE_LOG(LogViewportManager, Log, TEXT("UVMLayoutDesignerWidget::RemoveSelectedPane - Removed pane at index %d"), SelectedPaneIndex);
}

void UVMLayoutDesignerWidget::ApplyTwoPlayerHorizontal()
{
	if (!CurrentLayoutAsset)
		return;
	
	CurrentLayoutAsset->Panes.Empty();
	
	// Left pane
	FVMSplitPane LeftPane;
	LeftPane.LocalPlayerIndex = 0;
	LeftPane.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	LeftPane.Rect.Size01 = FVector2f(0.5f, 1.0f);
	CurrentLayoutAsset->Panes.Add(LeftPane);
	
	// Right pane
	FVMSplitPane RightPane;
	RightPane.LocalPlayerIndex = 1;
	RightPane.Rect.Origin01 = FVector2f(0.5f, 0.0f);
	RightPane.Rect.Size01 = FVector2f(0.5f, 1.0f);
	CurrentLayoutAsset->Panes.Add(RightPane);
	
	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UpdateStatusText();
	
	UE_LOG(LogViewportManager, Log, TEXT("UVMLayoutDesignerWidget::ApplyTwoPlayerHorizontal - Applied horizontal split layout"));
}

void UVMLayoutDesignerWidget::ApplyTwoPlayerVertical()
{
	if (!CurrentLayoutAsset)
		return;
	
	CurrentLayoutAsset->Panes.Empty();
	
	// Top pane
	FVMSplitPane TopPane;
	TopPane.LocalPlayerIndex = 0;
	TopPane.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	TopPane.Rect.Size01 = FVector2f(1.0f, 0.5f);
	CurrentLayoutAsset->Panes.Add(TopPane);
	
	// Bottom pane
	FVMSplitPane BottomPane;
	BottomPane.LocalPlayerIndex = 1;
	BottomPane.Rect.Origin01 = FVector2f(0.0f, 0.5f);
	BottomPane.Rect.Size01 = FVector2f(1.0f, 0.5f);
	CurrentLayoutAsset->Panes.Add(BottomPane);
	
	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UpdateStatusText();
	
	UE_LOG(LogViewportManager, Log, TEXT("UVMLayoutDesignerWidget::ApplyTwoPlayerVertical - Applied vertical split layout"));
}

void UVMLayoutDesignerWidget::ApplyFourPlayerGrid()
{
	if (!CurrentLayoutAsset)
		return;
	
	CurrentLayoutAsset->Panes.Empty();
	
	// Top-left pane
	FVMSplitPane TopLeftPane;
	TopLeftPane.LocalPlayerIndex = 0;
	TopLeftPane.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	TopLeftPane.Rect.Size01 = FVector2f(0.5f, 0.5f);
	CurrentLayoutAsset->Panes.Add(TopLeftPane);
	
	// Top-right pane
	FVMSplitPane TopRightPane;
	TopRightPane.LocalPlayerIndex = 1;
	TopRightPane.Rect.Origin01 = FVector2f(0.5f, 0.0f);
	TopRightPane.Rect.Size01 = FVector2f(0.5f, 0.5f);
	CurrentLayoutAsset->Panes.Add(TopRightPane);
	
	// Bottom-left pane
	FVMSplitPane BottomLeftPane;
	BottomLeftPane.LocalPlayerIndex = 2;
	BottomLeftPane.Rect.Origin01 = FVector2f(0.0f, 0.5f);
	BottomLeftPane.Rect.Size01 = FVector2f(0.5f, 0.5f);
	CurrentLayoutAsset->Panes.Add(BottomLeftPane);
	
	// Bottom-right pane
	FVMSplitPane BottomRightPane;
	BottomRightPane.LocalPlayerIndex = 3;
	BottomRightPane.Rect.Origin01 = FVector2f(0.5f, 0.5f);
	BottomRightPane.Rect.Size01 = FVector2f(0.5f, 0.5f);
	CurrentLayoutAsset->Panes.Add(BottomRightPane);
	
	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UpdateStatusText();
	
	UE_LOG(LogViewportManager, Log, TEXT("UVMLayoutDesignerWidget::ApplyFourPlayerGrid - Applied 2x2 grid layout"));
}

void UVMLayoutDesignerWidget::CreatePaneVisuals()
{
	if (!CurrentLayoutAsset || !DesignCanvas)
		return;
	
	for (int32 i = 0; i < CurrentLayoutAsset->Panes.Num(); ++i)
	{
		const FVMSplitPane& Pane = CurrentLayoutAsset->Panes[i];
		UBorder* PaneVisual = CreatePaneVisual(i, Pane);
		if (PaneVisual)
		{
			PaneVisuals.Add(PaneVisual);
		}
	}
}

UBorder* UVMLayoutDesignerWidget::CreatePaneVisual(int32 PaneIndex, const FVMSplitPane& Pane)
{
	if (!DesignCanvas)
		return nullptr;
	
	// Create border for visual representation
	UBorder* PaneBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), 
		*FString::Printf(TEXT("PaneVisual_%d"), PaneIndex));
	
	// Set border appearance
	FLinearColor PaneColor = GetPaneColor(Pane.LocalPlayerIndex);
	PaneBorder->SetBrushColor(PaneColor);
	PaneBorder->SetBrushFromMaterial(nullptr); // Use default border material
	
	// Create text label for the pane
	UTextBlock* PaneLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), 
		*FString::Printf(TEXT("PaneLabel_%d"), PaneIndex));
	
	FString LabelText = FString::Printf(TEXT("Player %d\n(%.2f, %.2f)\n%.2fx%.2f"), 
		Pane.LocalPlayerIndex,
		Pane.Rect.Origin01.X, Pane.Rect.Origin01.Y,
		Pane.Rect.Size01.X, Pane.Rect.Size01.Y);
	
	if (Pane.ViewportHUDClass)
	{
		LabelText += FString::Printf(TEXT("\nHUD: %s"), *Pane.ViewportHUDClass->GetName());
	}
	
	PaneLabel->SetText(FText::FromString(LabelText));
	PaneLabel->SetColorAndOpacity(FLinearColor::White);
	PaneLabel->SetJustification(ETextJustify::Center);
	
	// Add label to border
	PaneBorder->AddChild(PaneLabel);
	
	// Add border to canvas
	UCanvasPanelSlot* PaneSlot = DesignCanvas->AddChildToCanvas(PaneBorder);
	if (PaneSlot)
	{
		// Position based on pane rect (assuming 400x300 design canvas)
		const float CanvasWidth = 400.0f;
		const float CanvasHeight = 300.0f;
		
		FVector2f Position = FVector2f(
			Pane.Rect.Origin01.X * CanvasWidth,
			Pane.Rect.Origin01.Y * CanvasHeight
		);
		
		FVector2f Size = FVector2f(
			Pane.Rect.Size01.X * CanvasWidth,
			Pane.Rect.Size01.Y * CanvasHeight
		);
		
		PaneSlot->SetPosition(FVector2D(Position));
		PaneSlot->SetSize(FVector2D(Size));
		PaneSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		
		// Add selection highlight if this is the selected pane
		if (PaneIndex == SelectedPaneIndex)
		{
			PaneBorder->SetBrushColor(PaneColor * 1.5f); // Brighten selected pane
		}
	}
	
	return PaneBorder;
}

void UVMLayoutDesignerWidget::UpdatePaneVisual(int32 PaneIndex)
{
	if (!CurrentLayoutAsset || PaneIndex < 0 || PaneIndex >= PaneVisuals.Num())
		return;
	
	const FVMSplitPane& Pane = CurrentLayoutAsset->Panes[PaneIndex];
	UBorder* PaneVisual = PaneVisuals[PaneIndex];
	
	if (!PaneVisual)
		return;
	
	// Update color
	FLinearColor PaneColor = GetPaneColor(Pane.LocalPlayerIndex);
	if (PaneIndex == SelectedPaneIndex)
	{
		PaneColor *= 1.5f; // Brighten selected pane
	}
	PaneVisual->SetBrushColor(PaneColor);
	
	// Update position and size
	if (UCanvasPanelSlot* PaneSlot = Cast<UCanvasPanelSlot>(PaneVisual->Slot))
	{
		const float CanvasWidth = 400.0f;
		const float CanvasHeight = 300.0f;
		
		FVector2f Position = FVector2f(
			Pane.Rect.Origin01.X * CanvasWidth,
			Pane.Rect.Origin01.Y * CanvasHeight
		);
		
		FVector2f Size = FVector2f(
			Pane.Rect.Size01.X * CanvasWidth,
			Pane.Rect.Size01.Y * CanvasHeight
		);
		
		PaneSlot->SetPosition(FVector2D(Position));
		PaneSlot->SetSize(FVector2D(Size));
	}
	
	// Update label text
	if (UTextBlock* Label = Cast<UTextBlock>(PaneVisual->GetChildAt(0)))
	{
		FString LabelText = FString::Printf(TEXT("Player %d\n(%.2f, %.2f)\n%.2fx%.2f"), 
			Pane.LocalPlayerIndex,
			Pane.Rect.Origin01.X, Pane.Rect.Origin01.Y,
			Pane.Rect.Size01.X, Pane.Rect.Size01.Y);
		
		if (Pane.ViewportHUDClass)
		{
			LabelText += FString::Printf(TEXT("\nHUD: %s"), *Pane.ViewportHUDClass->GetName());
		}
		
		Label->SetText(FText::FromString(LabelText));
	}
}

void UVMLayoutDesignerWidget::OnPaneClicked(int32 PaneIndex)
{
	SelectedPaneIndex = PaneIndex;
	
	// Update all visuals to show selection
	for (int32 i = 0; i < PaneVisuals.Num(); ++i)
	{
		UpdatePaneVisual(i);
	}
	
	UpdateStatusText();
	
	UE_LOG(LogViewportManager, Log, TEXT("UVMLayoutDesignerWidget::OnPaneClicked - Selected pane %d"), PaneIndex);
}

void UVMLayoutDesignerWidget::UpdateStatusText()
{
	if (!StatusText)
		return;
	
	FString Status;
	
	if (!CurrentLayoutAsset)
	{
		Status = TEXT("No layout asset selected");
	}
	else
	{
		Status = FString::Printf(TEXT("Layout: %s | Panes: %d"), 
			*CurrentLayoutAsset->GetName(), 
			CurrentLayoutAsset->Panes.Num());
		
		if (SelectedPaneIndex >= 0 && SelectedPaneIndex < CurrentLayoutAsset->Panes.Num())
		{
			const FVMSplitPane& SelectedPane = CurrentLayoutAsset->Panes[SelectedPaneIndex];
			Status += FString::Printf(TEXT(" | Selected: Player %d"), SelectedPane.LocalPlayerIndex);
		}
	}
	
	StatusText->SetText(FText::FromString(Status));
}

void UVMLayoutDesignerWidget::BindButtonEvents()
{
	if (AddPaneButton)
	{
		AddPaneButton->OnClicked.AddDynamic(this, &UVMLayoutDesignerWidget::AddNewPane);
	}
	
	if (RemovePaneButton)
	{
		RemovePaneButton->OnClicked.AddDynamic(this, &UVMLayoutDesignerWidget::RemoveSelectedPane);
	}
	
	if (TwoPlayerHorizontalButton)
	{
		TwoPlayerHorizontalButton->OnClicked.AddDynamic(this, &UVMLayoutDesignerWidget::ApplyTwoPlayerHorizontal);
	}
	
	if (TwoPlayerVerticalButton)
	{
		TwoPlayerVerticalButton->OnClicked.AddDynamic(this, &UVMLayoutDesignerWidget::ApplyTwoPlayerVertical);
	}
	
	if (FourPlayerGridButton)
	{
		FourPlayerGridButton->OnClicked.AddDynamic(this, &UVMLayoutDesignerWidget::ApplyFourPlayerGrid);
	}
}

FLinearColor UVMLayoutDesignerWidget::GetPaneColor(int32 LocalPlayerIndex) const
{
	const uint8 Hue = static_cast<uint8>((LocalPlayerIndex * 53) % 255);
	FLinearColor Color = FLinearColor::MakeFromHSV8(Hue, 200, 255);
	return Color * 0.75f;
}

// ==================== Snap-to-Grid Implementation ====================

FVector2f UVMLayoutDesignerWidget::SnapToGrid(const FVector2f& Value) const
{
	if (!bSnapToGrid || GridSize <= 0.0f)
	{
		return Value;
	}

	return FVector2f(
		FMath::RoundToFloat(Value.X / GridSize) * GridSize,
		FMath::RoundToFloat(Value.Y / GridSize) * GridSize
	);
}

void UVMLayoutDesignerWidget::ToggleSnapToGrid()
{
	bSnapToGrid = !bSnapToGrid;
	UpdateStatusText();
	UE_LOG(LogViewportManager, Log, TEXT("Snap to grid: %s"), bSnapToGrid ? TEXT("Enabled") : TEXT("Disabled"));
}

void UVMLayoutDesignerWidget::SetGridSize(float NewGridSize)
{
	GridSize = FMath::Clamp(NewGridSize, 0.01f, 0.5f);
	UE_LOG(LogViewportManager, Log, TEXT("Grid size set to: %.3f"), GridSize);
}

// ==================== Alignment Tools Implementation ====================

void UVMLayoutDesignerWidget::AlignSelectedPaneLeft()
{
	if (!CurrentLayoutAsset || SelectedPaneIndex < 0 || SelectedPaneIndex >= CurrentLayoutAsset->Panes.Num())
		return;

	PushUndoState();
	CurrentLayoutAsset->Panes[SelectedPaneIndex].Rect.Origin01.X = 0.0f;
	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UE_LOG(LogViewportManager, Log, TEXT("Aligned pane %d to left"), SelectedPaneIndex);
}

void UVMLayoutDesignerWidget::AlignSelectedPaneRight()
{
	if (!CurrentLayoutAsset || SelectedPaneIndex < 0 || SelectedPaneIndex >= CurrentLayoutAsset->Panes.Num())
		return;

	PushUndoState();
	FVMSplitPane& Pane = CurrentLayoutAsset->Panes[SelectedPaneIndex];
	Pane.Rect.Origin01.X = 1.0f - Pane.Rect.Size01.X;
	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UE_LOG(LogViewportManager, Log, TEXT("Aligned pane %d to right"), SelectedPaneIndex);
}

void UVMLayoutDesignerWidget::AlignSelectedPaneTop()
{
	if (!CurrentLayoutAsset || SelectedPaneIndex < 0 || SelectedPaneIndex >= CurrentLayoutAsset->Panes.Num())
		return;

	PushUndoState();
	CurrentLayoutAsset->Panes[SelectedPaneIndex].Rect.Origin01.Y = 0.0f;
	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UE_LOG(LogViewportManager, Log, TEXT("Aligned pane %d to top"), SelectedPaneIndex);
}

void UVMLayoutDesignerWidget::AlignSelectedPaneBottom()
{
	if (!CurrentLayoutAsset || SelectedPaneIndex < 0 || SelectedPaneIndex >= CurrentLayoutAsset->Panes.Num())
		return;

	PushUndoState();
	FVMSplitPane& Pane = CurrentLayoutAsset->Panes[SelectedPaneIndex];
	Pane.Rect.Origin01.Y = 1.0f - Pane.Rect.Size01.Y;
	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UE_LOG(LogViewportManager, Log, TEXT("Aligned pane %d to bottom"), SelectedPaneIndex);
}

void UVMLayoutDesignerWidget::AlignSelectedPaneCenter()
{
	if (!CurrentLayoutAsset || SelectedPaneIndex < 0 || SelectedPaneIndex >= CurrentLayoutAsset->Panes.Num())
		return;

	PushUndoState();
	FVMSplitPane& Pane = CurrentLayoutAsset->Panes[SelectedPaneIndex];
	Pane.Rect.Origin01.X = (1.0f - Pane.Rect.Size01.X) * 0.5f;
	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UE_LOG(LogViewportManager, Log, TEXT("Aligned pane %d to center"), SelectedPaneIndex);
}

void UVMLayoutDesignerWidget::AlignSelectedPaneMiddle()
{
	if (!CurrentLayoutAsset || SelectedPaneIndex < 0 || SelectedPaneIndex >= CurrentLayoutAsset->Panes.Num())
		return;

	PushUndoState();
	FVMSplitPane& Pane = CurrentLayoutAsset->Panes[SelectedPaneIndex];
	Pane.Rect.Origin01.Y = (1.0f - Pane.Rect.Size01.Y) * 0.5f;
	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UE_LOG(LogViewportManager, Log, TEXT("Aligned pane %d to middle"), SelectedPaneIndex);
}

void UVMLayoutDesignerWidget::DistributePanesHorizontally()
{
	if (!CurrentLayoutAsset || CurrentLayoutAsset->Panes.Num() < 2)
		return;

	PushUndoState();

	// Sort panes by X position
	TArray<int32> PaneIndices;
	for (int32 i = 0; i < CurrentLayoutAsset->Panes.Num(); ++i)
	{
		PaneIndices.Add(i);
	}

	PaneIndices.Sort([this](int32 A, int32 B) {
		return CurrentLayoutAsset->Panes[A].Rect.Origin01.X < CurrentLayoutAsset->Panes[B].Rect.Origin01.X;
	});

	// Calculate total width needed
	float TotalWidth = 0.0f;
	for (int32 i = 0; i < PaneIndices.Num(); ++i)
	{
		TotalWidth += CurrentLayoutAsset->Panes[PaneIndices[i]].Rect.Size01.X;
	}

	// Distribute evenly
	float Spacing = (1.0f - TotalWidth) / (PaneIndices.Num() - 1);
	float CurrentX = 0.0f;

	for (int32 i = 0; i < PaneIndices.Num(); ++i)
	{
		FVMSplitPane& Pane = CurrentLayoutAsset->Panes[PaneIndices[i]];
		Pane.Rect.Origin01.X = CurrentX;
		CurrentX += Pane.Rect.Size01.X + Spacing;
	}

	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UE_LOG(LogViewportManager, Log, TEXT("Distributed %d panes horizontally"), PaneIndices.Num());
}

void UVMLayoutDesignerWidget::DistributePanesVertically()
{
	if (!CurrentLayoutAsset || CurrentLayoutAsset->Panes.Num() < 2)
		return;

	PushUndoState();

	// Sort panes by Y position
	TArray<int32> PaneIndices;
	for (int32 i = 0; i < CurrentLayoutAsset->Panes.Num(); ++i)
	{
		PaneIndices.Add(i);
	}

	PaneIndices.Sort([this](int32 A, int32 B) {
		return CurrentLayoutAsset->Panes[A].Rect.Origin01.Y < CurrentLayoutAsset->Panes[B].Rect.Origin01.Y;
	});

	// Calculate total height needed
	float TotalHeight = 0.0f;
	for (int32 i = 0; i < PaneIndices.Num(); ++i)
	{
		TotalHeight += CurrentLayoutAsset->Panes[PaneIndices[i]].Rect.Size01.Y;
	}

	// Distribute evenly
	float Spacing = (1.0f - TotalHeight) / (PaneIndices.Num() - 1);
	float CurrentY = 0.0f;

	for (int32 i = 0; i < PaneIndices.Num(); ++i)
	{
		FVMSplitPane& Pane = CurrentLayoutAsset->Panes[PaneIndices[i]];
		Pane.Rect.Origin01.Y = CurrentY;
		CurrentY += Pane.Rect.Size01.Y + Spacing;
	}

	CurrentLayoutAsset->MarkPackageDirty();
	RefreshLayout();
	UE_LOG(LogViewportManager, Log, TEXT("Distributed %d panes vertically"), PaneIndices.Num());
}

// ==================== Undo/Redo Implementation ====================

void UVMLayoutDesignerWidget::PushUndoState()
{
	if (!CurrentLayoutAsset)
		return;

	// Create a snapshot of current state
	UVMSplitLayoutAsset* Snapshot = DuplicateLayoutAsset(CurrentLayoutAsset);
	if (Snapshot)
	{
		UndoStack.Add(Snapshot);

		// Limit undo stack size to 20 operations
		if (UndoStack.Num() > 20)
		{
			UndoStack.RemoveAt(0);
		}

		// Clear redo stack when a new action is performed
		RedoStack.Empty();

		UE_LOG(LogViewportManager, Verbose, TEXT("Pushed undo state (stack size: %d)"), UndoStack.Num());
	}
}

void UVMLayoutDesignerWidget::Undo()
{
	if (!CanUndo())
		return;

	// Save current state to redo stack
	UVMSplitLayoutAsset* CurrentState = DuplicateLayoutAsset(CurrentLayoutAsset);
	if (CurrentState)
	{
		RedoStack.Add(CurrentState);
	}

	// Restore previous state
	UVMSplitLayoutAsset* PreviousState = UndoStack.Pop();
	if (PreviousState)
	{
		CurrentLayoutAsset->Panes = PreviousState->Panes;
		CurrentLayoutAsset->bAutoSpawnPlayers = PreviousState->bAutoSpawnPlayers;
		CurrentLayoutAsset->MarkPackageDirty();
		RefreshLayout();
		UpdateStatusText();

		UE_LOG(LogViewportManager, Log, TEXT("Undo performed (undo stack: %d, redo stack: %d)"),
			UndoStack.Num(), RedoStack.Num());
	}
}

void UVMLayoutDesignerWidget::Redo()
{
	if (!CanRedo())
		return;

	// Save current state to undo stack
	UVMSplitLayoutAsset* CurrentState = DuplicateLayoutAsset(CurrentLayoutAsset);
	if (CurrentState)
	{
		UndoStack.Add(CurrentState);
	}

	// Restore next state
	UVMSplitLayoutAsset* NextState = RedoStack.Pop();
	if (NextState)
	{
		CurrentLayoutAsset->Panes = NextState->Panes;
		CurrentLayoutAsset->bAutoSpawnPlayers = NextState->bAutoSpawnPlayers;
		CurrentLayoutAsset->MarkPackageDirty();
		RefreshLayout();
		UpdateStatusText();

		UE_LOG(LogViewportManager, Log, TEXT("Redo performed (undo stack: %d, redo stack: %d)"),
			UndoStack.Num(), RedoStack.Num());
	}
}

bool UVMLayoutDesignerWidget::CanUndo() const
{
	return UndoStack.Num() > 0;
}

bool UVMLayoutDesignerWidget::CanRedo() const
{
	return RedoStack.Num() > 0;
}

void UVMLayoutDesignerWidget::ClearUndoHistory()
{
	UndoStack.Empty();
	RedoStack.Empty();
	UE_LOG(LogViewportManager, Log, TEXT("Cleared undo/redo history"));
}

UVMSplitLayoutAsset* UVMLayoutDesignerWidget::DuplicateLayoutAsset(UVMSplitLayoutAsset* Source)
{
	if (!Source)
		return nullptr;

	UVMSplitLayoutAsset* Duplicate = NewObject<UVMSplitLayoutAsset>(GetTransientPackage());
	Duplicate->Panes = Source->Panes;
	Duplicate->bAutoSpawnPlayers = Source->bAutoSpawnPlayers;
	return Duplicate;
}

// ==================== Drag Handling Implementation ====================

FReply UVMLayoutDesignerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && DesignCanvas)
	{
		const FVector2D LocalMousePos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

		// Check if we're clicking on a pane
		for (int32 i = 0; i < PaneVisuals.Num(); ++i)
		{
			if (UBorder* PaneBorder = PaneVisuals[i])
			{
				if (UCanvasPanelSlot* PaneSlot = Cast<UCanvasPanelSlot>(PaneBorder->Slot))
				{
					const FVector2D PanePos = PaneSlot->GetPosition();
					const FVector2D PaneSize = PaneSlot->GetSize();

					if (LocalMousePos.X >= PanePos.X && LocalMousePos.X <= PanePos.X + PaneSize.X &&
						LocalMousePos.Y >= PanePos.Y && LocalMousePos.Y <= PanePos.Y + PaneSize.Y)
					{
						// Start dragging
						bIsDraggingPane = true;
						DraggingPaneIndex = i;
						DragStartPosition = LocalMousePos;
						OnPaneClicked(i);

						return FReply::Handled().CaptureMouse(this->TakeWidget());
					}
				}
			}
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UVMLayoutDesignerWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggingPane && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDraggingPane = false;
		DraggingPaneIndex = -1;

		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UVMLayoutDesignerWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggingPane && DraggingPaneIndex >= 0 && CurrentLayoutAsset &&
		DraggingPaneIndex < CurrentLayoutAsset->Panes.Num())
	{
		const FVector2D LocalMousePos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		const FVector2D Delta = LocalMousePos - DragStartPosition;

		// Convert to normalized coordinates (0-1)
		const float CanvasWidth = 400.0f;
		const float CanvasHeight = 300.0f;
		const FVector2f NormalizedDelta = FVector2f(Delta.X / CanvasWidth, Delta.Y / CanvasHeight);

		// Update pane position
		FVMSplitPane& Pane = CurrentLayoutAsset->Panes[DraggingPaneIndex];
		FVector2f NewOrigin = Pane.Rect.Origin01 + NormalizedDelta;

		// Apply snap-to-grid
		if (bSnapToGrid)
		{
			NewOrigin = SnapToGrid(NewOrigin);
		}

		// Clamp to valid range
		NewOrigin.X = FMath::Clamp(NewOrigin.X, 0.0f, 1.0f - Pane.Rect.Size01.X);
		NewOrigin.Y = FMath::Clamp(NewOrigin.Y, 0.0f, 1.0f - Pane.Rect.Size01.Y);

		Pane.Rect.Origin01 = NewOrigin;
		CurrentLayoutAsset->MarkPackageDirty();

		// Update visual
		UpdatePaneVisual(DraggingPaneIndex);
		DragStartPosition = LocalMousePos;

		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}



