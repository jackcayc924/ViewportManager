// Copyright jackcayc924 2025. All Rights Reserved.

#include "SVMLayoutDesignerWidget.h"
#include "VMLayoutPaneEditorObject.h"
#include "VMSplitLayoutAsset.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Styling/AppStyle.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Editor.h"
#include "AssetRegistry/AssetData.h"
#include "InputCoreTypes.h"
#include "VMLayoutEditorCommands.h"
#include "VMLayoutTemplates.h"
#include "Styles/VMLayoutEditorStyle.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SCheckBox.h"

static FLinearColor GetPaneColor(int32 LocalPlayerIndex, bool bSelected)
{
	const uint8 Hue = static_cast<uint8>((LocalPlayerIndex * 53) % 255);
	FLinearColor Base = FLinearColor::MakeFromHSV8(Hue, 200, 245);
	return bSelected ? Base.Desaturate(0.25f) : Base * 0.85f;
}

SVMLayoutDesignerWidget::~SVMLayoutDesignerWidget()
{
	if (LayoutPropertyChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(LayoutPropertyChangedHandle);
	}
}

void SVMLayoutDesignerWidget::Construct(const FArguments& InArgs)
{
	bAllowAssetPicker = InArgs._bShowAssetPicker;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsArgs;
	DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsArgs.bHideSelectionTip = true;
	DetailsArgs.bAllowSearch = true;
	DetailsArgs.bShowScrollBar = true;

	PaneDetailsView = PropertyModule.CreateDetailView(DetailsArgs);
	PaneDetailsView->SetObject(nullptr);
	PaneDetailsView->OnFinishedChangingProperties().AddLambda([this](const FPropertyChangedEvent&)
	{
		RefreshPreview();
	});

	// Initialize command list and bind shortcuts
	FVMLayoutEditorCommands::Register();
	BindCommands();

	SAssignNew(PreviewCanvas, SConstraintCanvas);

	TSharedRef<SHorizontalBox> Toolbar = SNew(SHorizontalBox);
	BuildToolbar(Toolbar);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.0f)
		[
			Toolbar
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(4.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)
			.ResizeMode(ESplitterResizeMode::Fill)
			+ SSplitter::Slot()
			.Value(0.6f)
			.MinSize(300.f)
			[
				SAssignNew(PreviewContainer, SBorder)
				.Padding(6.0f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					// Removed fixed size SBox to allow resizing
					SNew(SBox)
					.MinDesiredWidth(400.f)
					.MinDesiredHeight(300.f)
					[
						PreviewCanvas.ToSharedRef()
					]
				]
			]
			+ SSplitter::Slot()
			.Value(0.4f)
			.MinSize(250.f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(4.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4.f)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("VMLayoutDesigner", "PaneSettingsTitle", "Viewport Pane Settings"))
						.TextStyle(&FAppStyle::Get().GetWidgetStyle<FTextBlockStyle>("DetailsView.CategoryTextStyle"))
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						PaneDetailsView.ToSharedRef()
					]
				]
			]
		]
	];

	SetLayoutAsset(InArgs._InitialAsset);
}

void SVMLayoutDesignerWidget::SetLayoutAsset(UVMSplitLayoutAsset* NewAsset)
{
	if (LayoutAsset.Get() == NewAsset)
	{
		RefreshPreview();
		return;
	}

	if (LayoutPropertyChangedHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(LayoutPropertyChangedHandle);
		LayoutPropertyChangedHandle.Reset();
	}

	LayoutAsset = NewAsset;
	SelectedPaneIndex = INDEX_NONE;

	if (PaneEditorObject.IsValid())
	{
		PaneEditorObject.Reset();
	}

	if (PaneDetailsView.IsValid())
	{
		PaneDetailsView->SetObject(nullptr);
		PaneDetailsView->ForceRefresh();
	}

	if (LayoutAsset.IsValid())
	{
		LayoutPropertyChangedHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(this, &SVMLayoutDesignerWidget::OnLayoutPropertyChanged);
	}

	RefreshPreview();
}

void SVMLayoutDesignerWidget::BuildToolbar(const TSharedRef<SHorizontalBox>& Toolbar)
{
	Toolbar->AddSlot()
	.AutoWidth()
	.Padding(0.f, 0.f, 12.f, 0.f)
	.VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text_Lambda([this]() { return FText::Format(NSLOCTEXT("VMLayoutDesigner", "ActiveLayoutLabel", "Active Layout: {0}"), GetActiveLayoutName()); })
	];

	if (bAllowAssetPicker)
	{
		Toolbar->AddSlot()
		.AutoWidth()
		.Padding(0.f, 0.f, 12.f, 0.f)
		[
			SAssignNew(LayoutAssetPickerButton, SComboButton)
			.ToolTipText(NSLOCTEXT("VMLayoutDesigner", "SelectLayoutTooltip", "Select a VMSplitLayoutAsset to preview."))
			.ButtonContent()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return GetActiveLayoutName();
				})
			]
			.ContentPadding(FMargin(6.f, 2.f))
			.OnGetMenuContent(this, &SVMLayoutDesignerWidget::CreateLayoutAssetPickerMenu)
		];
	}

	// Add template picker button
	Toolbar->AddSlot()
	.AutoWidth()
	.Padding(4.f, 0.f, 12.f, 0.f)
	[
		SNew(SComboButton)
		.ButtonContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("VMLayoutDesigner", "Templates", "📋 Templates"))
			]
		]
		.OnGetMenuContent(this, &SVMLayoutDesignerWidget::CreateTemplatePickerMenu)
	];

	// Add grid toggle button
	Toolbar->AddSlot()
	.AutoWidth()
	.Padding(4.f, 0.f, 12.f, 0.f)
	[
		SNew(SCheckBox)
		.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
		.IsChecked_Lambda([this]() { return bShowGrid ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
		.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
		{
			bShowGrid = (NewState == ECheckBoxState::Checked);
			RefreshPreview();
		})
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("VMLayoutDesigner", "Grid", "# Grid"))
		]
	];

	// Add helpful tooltip about controls
	Toolbar->AddSlot()
	.AutoWidth()
	.Padding(8.f, 0.f, 0.f, 0.f)
	[
		SNew(STextBlock)
		.Text(NSLOCTEXT("VMLayoutDesigner", "ResizeTip", "💡 Tip: Drag pane edges to resize. Hold Ctrl to snap to viewport edges."))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)))
	];
}

TSharedRef<SWidget> SVMLayoutDesignerWidget::CreateLayoutAssetPickerMenu()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.SelectionMode = ESelectionMode::Single;
	AssetPickerConfig.Filter.ClassPaths.Add(UVMSplitLayoutAsset::StaticClass()->GetClassPathName());
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::Tile;
	AssetPickerConfig.bAllowNullSelection = false;
	AssetPickerConfig.bFocusSearchBoxWhenOpened = true;

	if (LayoutAsset.IsValid())
	{
		AssetPickerConfig.InitialAssetSelection = FAssetData(LayoutAsset.Get());
	}

	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SVMLayoutDesignerWidget::OnLayoutAssetPicked);
	AssetPickerConfig.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &SVMLayoutDesignerWidget::OnLayoutAssetPicked);
	AssetPickerConfig.OnAssetEnterPressed = FOnAssetEnterPressed::CreateSP(this, &SVMLayoutDesignerWidget::OnLayoutAssetPickedFromEnter);

	return ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig);
}

void SVMLayoutDesignerWidget::OnLayoutAssetPicked(const FAssetData& AssetData)
{
	HandleLayoutAssetChanged(AssetData);

	if (LayoutAssetPickerButton.IsValid())
	{
		LayoutAssetPickerButton->SetIsOpen(false);
	}
}

void SVMLayoutDesignerWidget::OnLayoutAssetPickedFromEnter(const TArray<FAssetData>& AssetList)
{
	if (AssetList.Num() > 0)
	{
		OnLayoutAssetPicked(AssetList[0]);
	}
}

void SVMLayoutDesignerWidget::HandleLayoutAssetChanged(const FAssetData& AssetData)
{
	SetLayoutAsset(Cast<UVMSplitLayoutAsset>(AssetData.GetAsset()));
}

void SVMLayoutDesignerWidget::OnLayoutPropertyChanged(UObject* Object, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (Object != LayoutAsset.Get())
	{
		return;
	}

	if (PaneEditorObject.IsValid() && LayoutAsset->Panes.IsValidIndex(SelectedPaneIndex))
	{
		PaneEditorObject->Initialize(LayoutAsset.Get(), SelectedPaneIndex);
		PaneDetailsView->SetObject(PaneEditorObject.Get());
	}
	else if (PaneEditorObject.IsValid())
	{
		PaneEditorObject.Reset();
		PaneDetailsView->SetObject(nullptr);
		SelectedPaneIndex = INDEX_NONE;
	}

	RefreshPreview();
}

void SVMLayoutDesignerWidget::SelectPane(int32 PaneIndex)
{
	if (!LayoutAsset.IsValid() || !LayoutAsset->Panes.IsValidIndex(PaneIndex))
	{
		SelectedPaneIndex = INDEX_NONE;
		PaneEditorObject.Reset();
		PaneDetailsView->SetObject(nullptr);
		RefreshPreview();
		return;
	}

	SelectedPaneIndex = PaneIndex;

	UVMLayoutPaneEditorObject* EditorObject = NewObject<UVMLayoutPaneEditorObject>();
	EditorObject->SetFlags(RF_Transactional);
	EditorObject->Initialize(LayoutAsset.Get(), PaneIndex);

	PaneEditorObject = TStrongObjectPtr<UVMLayoutPaneEditorObject>(EditorObject);
	PaneDetailsView->SetObject(EditorObject);
	PaneDetailsView->ForceRefresh();

	RefreshPreview();
}

bool SVMLayoutDesignerWidget::HasValidSelection() const
{
	return LayoutAsset.IsValid() && LayoutAsset->Panes.IsValidIndex(SelectedPaneIndex);
}

FText SVMLayoutDesignerWidget::GetActiveLayoutName() const
{
	return LayoutAsset.IsValid() ? FText::FromString(LayoutAsset->GetName()) : NSLOCTEXT("VMLayoutDesigner", "NoLayoutSelected", "None");
}

void SVMLayoutDesignerWidget::RefreshPreview()
{
	if (!PreviewCanvas.IsValid())
	{
		return;
	}

	PreviewCanvas->ClearChildren();

	if (!LayoutAsset.IsValid())
	{
		PreviewCanvas->AddSlot()
		.Anchors(FAnchors(0.f, 0.f, 1.f, 1.f))
		.Alignment(FVector2D(0.5f, 0.5f))
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("VMLayoutDesigner", "SelectLayoutPrompt", "Select a VMSplitLayoutAsset to begin designing."))
		];
		return;
	}

	// Create sorted indices array based on depth (lower depth renders first, higher depth on top)
	TArray<int32> SortedPaneIndices;
	SortedPaneIndices.Reserve(LayoutAsset->Panes.Num());
	for (int32 i = 0; i < LayoutAsset->Panes.Num(); ++i)
	{
		SortedPaneIndices.Add(i);
	}
	SortedPaneIndices.Sort([this](int32 A, int32 B)
	{
		return LayoutAsset->Panes[A].Depth < LayoutAsset->Panes[B].Depth;
	});

	// Render panes in depth order
	for (int32 PaneIdx : SortedPaneIndices)
	{
		const FVMSplitPane& Pane = LayoutAsset->Panes[PaneIdx];
		const FVMSplitRect& Rect = Pane.Rect;

		// Clamp values to prevent invalid anchors that cause freezing
		const float MinX = FMath::Clamp(Rect.Origin01.X, 0.f, 1.f);
		const float MinY = FMath::Clamp(Rect.Origin01.Y, 0.f, 1.f);
		const float MaxX = FMath::Clamp(Rect.Origin01.X + Rect.Size01.X, 0.f, 1.f);
		const float MaxY = FMath::Clamp(Rect.Origin01.Y + Rect.Size01.Y, 0.f, 1.f);

		// Skip invalid panes that would cause rendering issues
		if (MinX >= MaxX || MinY >= MaxY)
		{
			continue;
		}

		const bool bIsSelected = PaneIdx == SelectedPaneIndex;

		FLinearColor PaneColor = GetPaneColor(Pane.LocalPlayerIndex, bIsSelected);
		PaneColor.A = 0.7f;

		FText Label = FText::Format(
			NSLOCTEXT("VMLayoutDesigner", "PaneFmt", "LP{0}\n({1}, {2})\n{3} x {4}"),
			FText::AsNumber(Pane.LocalPlayerIndex),
			FText::AsNumber(FMath::RoundToInt(Rect.Origin01.X * 100.f)),
			FText::AsNumber(FMath::RoundToInt(Rect.Origin01.Y * 100.f)),
			FText::AsNumber(FMath::RoundToInt(Rect.Size01.X * 100.f)),
			FText::AsNumber(FMath::RoundToInt(Rect.Size01.Y * 100.f))
		);

		PreviewCanvas->AddSlot()
		.Anchors(FAnchors(MinX, MinY, MaxX, MaxY))
		.Offset(FMargin(0.f))
		.Alignment(FVector2D(0.f, 0.f))
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			.BorderBackgroundColor(PaneColor)
			.Padding(4.f)
			.OnMouseButtonDown(FPointerEventHandler::CreateSP(this, &SVMLayoutDesignerWidget::HandlePaneMouseDown, PaneIdx))
			[
				SNew(STextBlock)
				.Text(Label)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FLinearColor::White)
				.ShadowColorAndOpacity(FLinearColor::Black)
				.ShadowOffset(FVector2D(1.f, 1.f))
			]
		];

		// Add visual resize indicators for selected pane
		if (bIsSelected)
		{
			const FLinearColor HandleColor = FVMLayoutEditorStyle::HandleBlue;
			const float CornerSize = 10.f; // Pixels for corner indicators
			const float EdgeLength = 30.f; // Pixels for edge indicators
			const float EdgeThickness = 4.f; // Pixels

			// Helper to add a corner indicator (small square)
			// Alignment adjusts based on corner to keep indicator within canvas bounds
			auto AddCornerIndicator = [&](float AnchorX, float AnchorY, float AlignX, float AlignY)
			{
				PreviewCanvas->AddSlot()
				.Anchors(FAnchors(AnchorX, AnchorY))
				.Offset(FMargin(0.f, 0.f, CornerSize, CornerSize))
				.Alignment(FVector2D(AlignX, AlignY))
				[
					SNew(SBox)
					.WidthOverride(CornerSize)
					.HeightOverride(CornerSize)
					[
						SNew(SBorder)
						.Visibility(EVisibility::HitTestInvisible)
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor(HandleColor)
						.Padding(0.f)
					]
				];
			};

			// Helper to add an edge indicator (small line on edge)
			// Alignment adjusts based on edge position to keep indicator within canvas bounds
			auto AddEdgeIndicator = [&](float AnchorX, float AnchorY, float AlignX, float AlignY, bool bIsHorizontal)
			{
				PreviewCanvas->AddSlot()
				.Anchors(FAnchors(AnchorX, AnchorY))
				.Offset(bIsHorizontal
					? FMargin(0.f, 0.f, EdgeLength, EdgeThickness)
					: FMargin(0.f, 0.f, EdgeThickness, EdgeLength))
				.Alignment(FVector2D(AlignX, AlignY))
				[
					SNew(SBox)
					.WidthOverride(bIsHorizontal ? EdgeLength : EdgeThickness)
					.HeightOverride(bIsHorizontal ? EdgeThickness : EdgeLength)
					[
						SNew(SBorder)
						.Visibility(EVisibility::HitTestInvisible)
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor(HandleColor)
						.Padding(0.f)
					]
				];
			};

			// Add 4 corner indicators with alignment to keep within panel bounds
			AddCornerIndicator(MinX, MinY, 0.0f, 0.0f); // Top-left: grow down and right
			AddCornerIndicator(MaxX, MinY, 1.0f, 0.0f); // Top-right: grow down and left
			AddCornerIndicator(MinX, MaxY, 0.0f, 1.0f); // Bottom-left: grow up and right
			AddCornerIndicator(MaxX, MaxY, 1.0f, 1.0f); // Bottom-right: grow up and left

			// Add 4 edge indicators (centered on each edge)
			const float MidX = (MinX + MaxX) / 2.f;
			const float MidY = (MinY + MaxY) / 2.f;

			AddEdgeIndicator(MidX, MinY, 0.5f, 0.0f, true);  // Top edge: centered horizontally, grow down
			AddEdgeIndicator(MidX, MaxY, 0.5f, 1.0f, true);  // Bottom edge: centered horizontally, grow up
			AddEdgeIndicator(MinX, MidY, 0.0f, 0.5f, false); // Left edge: grow right, centered vertically
			AddEdgeIndicator(MaxX, MidY, 1.0f, 0.5f, false); // Right edge: grow left, centered vertically
		}
	}
}

FReply SVMLayoutDesignerWidget::HandlePaneMouseDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, int32 PaneIndex)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SelectPane(PaneIndex);

		if (LayoutAsset.IsValid() && LayoutAsset->Panes.IsValidIndex(PaneIndex))
		{
			// Check if we're near a resize handle
			const FVector2D LocalMousePos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
			EResizeHandle Handle = GetResizeHandleAtPosition(MyGeometry, LocalMousePos);

			// Start drag operation - either resize or move
			bIsDraggingPane = true;
			DraggingPaneIndex = PaneIndex;
			ActiveResizeHandle = Handle; // Will be None if clicking in middle (move mode)
			DragStartMousePos = MouseEvent.GetScreenSpacePosition();
			DragStartPaneRect = LayoutAsset->Panes[PaneIndex].Rect;

			return FReply::Handled().CaptureMouse(AsShared());
		}

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SVMLayoutDesignerWidget::HandleSnapButtonClicked(float Increment)
{
	if (!LayoutAsset.IsValid() || !LayoutAsset->Panes.IsValidIndex(SelectedPaneIndex))
	{
		return FReply::Handled();
	}

	FVMSplitPane& Pane = LayoutAsset->Panes[SelectedPaneIndex];

	auto SnapValue = [Increment](float Value)
	{
		return FMath::RoundToFloat(Value / Increment) * Increment;
	};

	Pane.Rect.Origin01.X = FMath::Clamp(SnapValue(Pane.Rect.Origin01.X), 0.f, 1.f);
	Pane.Rect.Origin01.Y = FMath::Clamp(SnapValue(Pane.Rect.Origin01.Y), 0.f, 1.f);
	Pane.Rect.Size01.X = FMath::Clamp(SnapValue(Pane.Rect.Size01.X), 0.05f, 1.f);
	Pane.Rect.Size01.Y = FMath::Clamp(SnapValue(Pane.Rect.Size01.Y), 0.05f, 1.f);

	// Ensure pane stays in bounds
	Pane.Rect.Size01.X = FMath::Min(Pane.Rect.Size01.X, 1.f - Pane.Rect.Origin01.X);
	Pane.Rect.Size01.Y = FMath::Min(Pane.Rect.Size01.Y, 1.f - Pane.Rect.Origin01.Y);

	FScopedTransaction Transaction(NSLOCTEXT("VMLayoutEditor", "SnapPane", "Snap Pane to Grid"));
	LayoutAsset->Modify();
	LayoutAsset->MarkPackageDirty();

	if (PaneEditorObject.IsValid())
	{
		PaneEditorObject->Pane = Pane;
		PaneEditorObject->ApplyToLayout(false);
		PaneDetailsView->SetObject(PaneEditorObject.Get());
		PaneDetailsView->ForceRefresh();
	}

	RefreshPreview();
	return FReply::Handled();
}

FReply SVMLayoutDesignerWidget::HandleSnapToEdgeClicked(int32 Edge)
{
	if (!LayoutAsset.IsValid() || !LayoutAsset->Panes.IsValidIndex(SelectedPaneIndex))
	{
		return FReply::Handled();
	}

	FVMSplitPane& Pane = LayoutAsset->Panes[SelectedPaneIndex];

	// 0=left, 1=right, 2=top, 3=bottom
	switch (Edge)
	{
	case 0: // Snap to left
		Pane.Rect.Origin01.X = 0.f;
		break;
	case 1: // Snap to right
		Pane.Rect.Origin01.X = 1.f - Pane.Rect.Size01.X;
		break;
	case 2: // Snap to top
		Pane.Rect.Origin01.Y = 0.f;
		break;
	case 3: // Snap to bottom
		Pane.Rect.Origin01.Y = 1.f - Pane.Rect.Size01.Y;
		break;
	}

	// Clamp to valid range
	Pane.Rect.Origin01.X = FMath::Clamp(Pane.Rect.Origin01.X, 0.f, 1.f - Pane.Rect.Size01.X);
	Pane.Rect.Origin01.Y = FMath::Clamp(Pane.Rect.Origin01.Y, 0.f, 1.f - Pane.Rect.Size01.Y);

	FScopedTransaction Transaction(NSLOCTEXT("VMLayoutEditor", "SnapToEdge", "Snap Pane to Edge"));
	LayoutAsset->Modify();
	LayoutAsset->MarkPackageDirty();

	if (PaneEditorObject.IsValid())
	{
		PaneEditorObject->Pane = Pane;
		PaneEditorObject->ApplyToLayout(false);
		PaneDetailsView->SetObject(PaneEditorObject.Get());
		PaneDetailsView->ForceRefresh();
	}

	RefreshPreview();
	return FReply::Handled();
}

SVMLayoutDesignerWidget::EResizeHandle SVMLayoutDesignerWidget::GetResizeHandleAtPosition(const FGeometry& PaneGeometry, const FVector2D& LocalMousePos) const
{
	const float HandleSize = 16.f; // Pixels (increased from 8 for larger hitbox)
	const FVector2D PaneSize = PaneGeometry.GetLocalSize();

	const bool bNearLeft = LocalMousePos.X <= HandleSize;
	const bool bNearRight = LocalMousePos.X >= PaneSize.X - HandleSize;
	const bool bNearTop = LocalMousePos.Y <= HandleSize;
	const bool bNearBottom = LocalMousePos.Y >= PaneSize.Y - HandleSize;

	// Check corners first (higher priority)
	if (bNearLeft && bNearTop) return EResizeHandle::TopLeft;
	if (bNearRight && bNearTop) return EResizeHandle::TopRight;
	if (bNearLeft && bNearBottom) return EResizeHandle::BottomLeft;
	if (bNearRight && bNearBottom) return EResizeHandle::BottomRight;

	// Check edges
	if (bNearLeft) return EResizeHandle::Left;
	if (bNearRight) return EResizeHandle::Right;
	if (bNearTop) return EResizeHandle::Top;
	if (bNearBottom) return EResizeHandle::Bottom;

	return EResizeHandle::None;
}

FCursorReply SVMLayoutDesignerWidget::GetCursorForHandle(EResizeHandle Handle) const
{
	switch (Handle)
	{
	case EResizeHandle::Left:
	case EResizeHandle::Right:
		return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
	case EResizeHandle::Top:
	case EResizeHandle::Bottom:
		return FCursorReply::Cursor(EMouseCursor::ResizeUpDown);
	case EResizeHandle::TopLeft:
	case EResizeHandle::BottomRight:
		return FCursorReply::Cursor(EMouseCursor::ResizeSouthEast);
	case EResizeHandle::TopRight:
	case EResizeHandle::BottomLeft:
		return FCursorReply::Cursor(EMouseCursor::ResizeSouthWest);
	default:
		return FCursorReply::Cursor(EMouseCursor::Default);
	}
}

FReply SVMLayoutDesignerWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bIsDraggingPane && LayoutAsset.IsValid() && LayoutAsset->Panes.IsValidIndex(DraggingPaneIndex))
	{
		FVMSplitPane& Pane = LayoutAsset->Panes[DraggingPaneIndex];
		const FVector2D CurrentMousePos = MouseEvent.GetScreenSpacePosition();
		const FVector2D MouseDelta = CurrentMousePos - DragStartMousePos;

		// Convert to normalized delta
		const FVector2D CanvasSize = PreviewCanvas->GetTickSpaceGeometry().GetLocalSize();
		const FVector2f NormalizedDelta(MouseDelta.X / CanvasSize.X, MouseDelta.Y / CanvasSize.Y);

		// Apply resize or move based on active handle
		FVMSplitRect NewRect = DragStartPaneRect;

		switch (ActiveResizeHandle)
		{
		case EResizeHandle::None:
			// Move the entire pane
			NewRect.Origin01.X = FMath::Clamp(DragStartPaneRect.Origin01.X + NormalizedDelta.X, 0.f, 1.f - DragStartPaneRect.Size01.X);
			NewRect.Origin01.Y = FMath::Clamp(DragStartPaneRect.Origin01.Y + NormalizedDelta.Y, 0.f, 1.f - DragStartPaneRect.Size01.Y);
			// Size remains unchanged
			break;
		case EResizeHandle::Left:
			NewRect.Origin01.X = FMath::Clamp(DragStartPaneRect.Origin01.X + NormalizedDelta.X, 0.f, DragStartPaneRect.Origin01.X + DragStartPaneRect.Size01.X - 0.05f);
			NewRect.Size01.X = DragStartPaneRect.Origin01.X + DragStartPaneRect.Size01.X - NewRect.Origin01.X;
			break;
		case EResizeHandle::Right:
			NewRect.Size01.X = FMath::Clamp(DragStartPaneRect.Size01.X + NormalizedDelta.X, 0.05f, 1.f - DragStartPaneRect.Origin01.X);
			break;
		case EResizeHandle::Top:
			NewRect.Origin01.Y = FMath::Clamp(DragStartPaneRect.Origin01.Y + NormalizedDelta.Y, 0.f, DragStartPaneRect.Origin01.Y + DragStartPaneRect.Size01.Y - 0.05f);
			NewRect.Size01.Y = DragStartPaneRect.Origin01.Y + DragStartPaneRect.Size01.Y - NewRect.Origin01.Y;
			break;
		case EResizeHandle::Bottom:
			NewRect.Size01.Y = FMath::Clamp(DragStartPaneRect.Size01.Y + NormalizedDelta.Y, 0.05f, 1.f - DragStartPaneRect.Origin01.Y);
			break;
		case EResizeHandle::TopLeft:
			NewRect.Origin01.X = FMath::Clamp(DragStartPaneRect.Origin01.X + NormalizedDelta.X, 0.f, DragStartPaneRect.Origin01.X + DragStartPaneRect.Size01.X - 0.05f);
			NewRect.Size01.X = DragStartPaneRect.Origin01.X + DragStartPaneRect.Size01.X - NewRect.Origin01.X;
			NewRect.Origin01.Y = FMath::Clamp(DragStartPaneRect.Origin01.Y + NormalizedDelta.Y, 0.f, DragStartPaneRect.Origin01.Y + DragStartPaneRect.Size01.Y - 0.05f);
			NewRect.Size01.Y = DragStartPaneRect.Origin01.Y + DragStartPaneRect.Size01.Y - NewRect.Origin01.Y;
			break;
		case EResizeHandle::TopRight:
			NewRect.Size01.X = FMath::Clamp(DragStartPaneRect.Size01.X + NormalizedDelta.X, 0.05f, 1.f - DragStartPaneRect.Origin01.X);
			NewRect.Origin01.Y = FMath::Clamp(DragStartPaneRect.Origin01.Y + NormalizedDelta.Y, 0.f, DragStartPaneRect.Origin01.Y + DragStartPaneRect.Size01.Y - 0.05f);
			NewRect.Size01.Y = DragStartPaneRect.Origin01.Y + DragStartPaneRect.Size01.Y - NewRect.Origin01.Y;
			break;
		case EResizeHandle::BottomLeft:
			NewRect.Origin01.X = FMath::Clamp(DragStartPaneRect.Origin01.X + NormalizedDelta.X, 0.f, DragStartPaneRect.Origin01.X + DragStartPaneRect.Size01.X - 0.05f);
			NewRect.Size01.X = DragStartPaneRect.Origin01.X + DragStartPaneRect.Size01.X - NewRect.Origin01.X;
			NewRect.Size01.Y = FMath::Clamp(DragStartPaneRect.Size01.Y + NormalizedDelta.Y, 0.05f, 1.f - DragStartPaneRect.Origin01.Y);
			break;
		case EResizeHandle::BottomRight:
			NewRect.Size01.X = FMath::Clamp(DragStartPaneRect.Size01.X + NormalizedDelta.X, 0.05f, 1.f - DragStartPaneRect.Origin01.X);
			NewRect.Size01.Y = FMath::Clamp(DragStartPaneRect.Size01.Y + NormalizedDelta.Y, 0.05f, 1.f - DragStartPaneRect.Origin01.Y);
			break;
		default:
			break;
		}

		// Determine which edges can snap based on active handle
		bool bCanSnapLeft = false, bCanSnapRight = false, bCanSnapTop = false, bCanSnapBottom = false;

		switch (ActiveResizeHandle)
		{
		case EResizeHandle::Left:
		case EResizeHandle::TopLeft:
		case EResizeHandle::BottomLeft:
			bCanSnapLeft = true;
			break;
		case EResizeHandle::Right:
		case EResizeHandle::TopRight:
		case EResizeHandle::BottomRight:
			bCanSnapRight = true;
			break;
		}

		switch (ActiveResizeHandle)
		{
		case EResizeHandle::Top:
		case EResizeHandle::TopLeft:
		case EResizeHandle::TopRight:
			bCanSnapTop = true;
			break;
		case EResizeHandle::Bottom:
		case EResizeHandle::BottomLeft:
		case EResizeHandle::BottomRight:
			bCanSnapBottom = true;
			break;
		}

		if (ActiveResizeHandle == EResizeHandle::None)
		{
			// Move mode - snap all edges
			bCanSnapLeft = bCanSnapRight = bCanSnapTop = bCanSnapBottom = true;
		}

		// Ctrl-based edge snapping (viewport edges and other panes)
		if (MouseEvent.IsControlDown())
		{
			const float SnapThreshold = 0.08f; // Snap when within 8% of edge (increased for easier snapping)

			// Save the opposite edges before snapping - needed to recalculate size when origin changes
			const float OriginalRightEdge = NewRect.Origin01.X + NewRect.Size01.X;
			const float OriginalBottomEdge = NewRect.Origin01.Y + NewRect.Size01.Y;

			// Calculate current pane edges
			float CurrentLeft = NewRect.Origin01.X;
			float CurrentRight = OriginalRightEdge;
			float CurrentTop = NewRect.Origin01.Y;
			float CurrentBottom = OriginalBottomEdge;

			// Track if we snapped and whether we need to recalculate size
			bool bSnappedX = false;
			bool bSnappedY = false;
			bool bRecalcSizeX = false; // True if we snapped left edge
			bool bRecalcSizeY = false; // True if we snapped top edge

			// First, try snapping to viewport edges (0.0 and 1.0)

			// Snap left edge to 0.0
			if (bCanSnapLeft && CurrentLeft <= SnapThreshold && CurrentLeft >= 0.f)
			{
				NewRect.Origin01.X = 0.f;
				bSnappedX = true;
				bRecalcSizeX = true;
			}

			// Snap right edge to 1.0
			if (bCanSnapRight && !bSnappedX && CurrentRight >= 1.f - SnapThreshold && CurrentRight <= 1.f)
			{
				NewRect.Size01.X = 1.f - NewRect.Origin01.X;
				bSnappedX = true;
			}

			// Snap top edge to 0.0
			if (bCanSnapTop && CurrentTop <= SnapThreshold && CurrentTop >= 0.f)
			{
				NewRect.Origin01.Y = 0.f;
				bSnappedY = true;
				bRecalcSizeY = true;
			}

			// Snap bottom edge to 1.0
			if (bCanSnapBottom && !bSnappedY && CurrentBottom >= 1.f - SnapThreshold && CurrentBottom <= 1.f)
			{
				NewRect.Size01.Y = 1.f - NewRect.Origin01.Y;
				bSnappedY = true;
			}

			// Then, try snapping to other panes' edges
			if (LayoutAsset.IsValid())
			{
				for (int32 OtherIdx = 0; OtherIdx < LayoutAsset->Panes.Num(); ++OtherIdx)
				{
					// Skip the pane we're currently dragging
					if (OtherIdx == DraggingPaneIndex)
						continue;

					const FVMSplitPane& OtherPane = LayoutAsset->Panes[OtherIdx];
					const float OtherLeft = OtherPane.Rect.Origin01.X;
					const float OtherRight = OtherPane.Rect.Origin01.X + OtherPane.Rect.Size01.X;
					const float OtherTop = OtherPane.Rect.Origin01.Y;
					const float OtherBottom = OtherPane.Rect.Origin01.Y + OtherPane.Rect.Size01.Y;

					// Recalculate current edges in case they were modified by viewport snapping
					CurrentLeft = NewRect.Origin01.X;
					CurrentRight = NewRect.Origin01.X + NewRect.Size01.X;
					CurrentTop = NewRect.Origin01.Y;
					CurrentBottom = NewRect.Origin01.Y + NewRect.Size01.Y;

					// Try snapping horizontal edges
					if (!bSnappedX)
					{
						// Snap current left to other's left
						if (bCanSnapLeft && FMath::Abs(CurrentLeft - OtherLeft) <= SnapThreshold)
						{
							NewRect.Origin01.X = OtherLeft;
							bSnappedX = true;
							bRecalcSizeX = true;
						}
						// Snap current left to other's right
						else if (bCanSnapLeft && FMath::Abs(CurrentLeft - OtherRight) <= SnapThreshold)
						{
							NewRect.Origin01.X = OtherRight;
							bSnappedX = true;
							bRecalcSizeX = true;
						}
						// Snap current right to other's left
						else if (bCanSnapRight && FMath::Abs(CurrentRight - OtherLeft) <= SnapThreshold)
						{
							NewRect.Size01.X = OtherLeft - NewRect.Origin01.X;
							bSnappedX = true;
						}
						// Snap current right to other's right
						else if (bCanSnapRight && FMath::Abs(CurrentRight - OtherRight) <= SnapThreshold)
						{
							NewRect.Size01.X = OtherRight - NewRect.Origin01.X;
							bSnappedX = true;
						}
					}

					// Try snapping vertical edges
					if (!bSnappedY)
					{
						// Snap current top to other's top
						if (bCanSnapTop && FMath::Abs(CurrentTop - OtherTop) <= SnapThreshold)
						{
							NewRect.Origin01.Y = OtherTop;
							bSnappedY = true;
							bRecalcSizeY = true;
						}
						// Snap current top to other's bottom
						else if (bCanSnapTop && FMath::Abs(CurrentTop - OtherBottom) <= SnapThreshold)
						{
							NewRect.Origin01.Y = OtherBottom;
							bSnappedY = true;
							bRecalcSizeY = true;
						}
						// Snap current bottom to other's top
						else if (bCanSnapBottom && FMath::Abs(CurrentBottom - OtherTop) <= SnapThreshold)
						{
							NewRect.Size01.Y = OtherTop - NewRect.Origin01.Y;
							bSnappedY = true;
						}
						// Snap current bottom to other's bottom
						else if (bCanSnapBottom && FMath::Abs(CurrentBottom - OtherBottom) <= SnapThreshold)
						{
							NewRect.Size01.Y = OtherBottom - NewRect.Origin01.Y;
							bSnappedY = true;
						}
					}

					// If we've snapped both X and Y, no need to check more panes
					if (bSnappedX && bSnappedY)
						break;
				}
			}

			// Recalculate sizes if we snapped origin (left or top edge) during RESIZE
			// This maintains the opposite edge position
			// Skip this for move operations (ActiveResizeHandle == None) where both edges move together
			if (ActiveResizeHandle != EResizeHandle::None)
			{
				if (bRecalcSizeX)
				{
					NewRect.Size01.X = OriginalRightEdge - NewRect.Origin01.X;
					NewRect.Size01.X = FMath::Max(NewRect.Size01.X, 0.05f); // Ensure minimum size
				}
				if (bRecalcSizeY)
				{
					NewRect.Size01.Y = OriginalBottomEdge - NewRect.Origin01.Y;
					NewRect.Size01.Y = FMath::Max(NewRect.Size01.Y, 0.05f); // Ensure minimum size
				}
			}
		}

		Pane.Rect = NewRect;

		// Only update preview during drag - don't refresh details panel (causes lag)
		// Details panel will be updated on mouse release
		RefreshPreview();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SVMLayoutDesignerWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bIsDraggingPane && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDraggingPane = false;

		if (LayoutAsset.IsValid())
		{
			FScopedTransaction Transaction(NSLOCTEXT("VMLayoutEditor", "ResizeMovePane", "Resize/Move Pane"));
			LayoutAsset->Modify();
			LayoutAsset->MarkPackageDirty();

			// Final update to editor object and details panel after drag completes
			if (PaneEditorObject.IsValid() && LayoutAsset->Panes.IsValidIndex(DraggingPaneIndex))
			{
				PaneEditorObject->Pane = LayoutAsset->Panes[DraggingPaneIndex];
				PaneEditorObject->ApplyToLayout(false);

				// Update details panel now that drag is complete
				PaneDetailsView->SetObject(PaneEditorObject.Get());
				PaneDetailsView->ForceRefresh();
			}
		}

		DraggingPaneIndex = INDEX_NONE;
		ActiveResizeHandle = EResizeHandle::None;

		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

void SVMLayoutDesignerWidget::BindCommands()
{
	CommandList = MakeShared<FUICommandList>();

	CommandList->MapAction(
		FVMLayoutEditorCommands::Get().Duplicate,
		FExecuteAction::CreateSP(this, &SVMLayoutDesignerWidget::DuplicateSelectedPane),
		FCanExecuteAction::CreateSP(this, &SVMLayoutDesignerWidget::HasValidSelection)
	);

	CommandList->MapAction(
		FVMLayoutEditorCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &SVMLayoutDesignerWidget::DeleteSelectedPane),
		FCanExecuteAction::CreateSP(this, &SVMLayoutDesignerWidget::HasValidSelection)
	);

	CommandList->MapAction(
		FVMLayoutEditorCommands::Get().BringToFront,
		FExecuteAction::CreateSP(this, &SVMLayoutDesignerWidget::BringToFront),
		FCanExecuteAction::CreateSP(this, &SVMLayoutDesignerWidget::HasValidSelection)
	);

	CommandList->MapAction(
		FVMLayoutEditorCommands::Get().SendToBack,
		FExecuteAction::CreateSP(this, &SVMLayoutDesignerWidget::SendToBack),
		FCanExecuteAction::CreateSP(this, &SVMLayoutDesignerWidget::HasValidSelection)
	);

	CommandList->MapAction(
		FVMLayoutEditorCommands::Get().ToggleGrid,
		FExecuteAction::CreateLambda([this]() { bShowGrid = !bShowGrid; RefreshPreview(); })
	);
}

FReply SVMLayoutDesignerWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SVMLayoutDesignerWidget::DuplicateSelectedPane()
{
	if (!HasValidSelection()) return;

	FScopedTransaction Transaction(NSLOCTEXT("VMLayoutEditor", "DuplicatePane", "Duplicate Pane"));
	LayoutAsset->Modify();

	FVMSplitPane NewPane = LayoutAsset->Panes[SelectedPaneIndex];

	// Find next available player index
	int32 MaxIndex = 0;
	for (const auto& Pane : LayoutAsset->Panes)
		MaxIndex = FMath::Max(MaxIndex, Pane.LocalPlayerIndex);

	NewPane.LocalPlayerIndex = MaxIndex + 1;

	// Offset position slightly
	NewPane.Rect.Origin01.X = FMath::Clamp(NewPane.Rect.Origin01.X + 0.05f, 0.f, 1.f - NewPane.Rect.Size01.X);
	NewPane.Rect.Origin01.Y = FMath::Clamp(NewPane.Rect.Origin01.Y + 0.05f, 0.f, 1.f - NewPane.Rect.Size01.Y);

	LayoutAsset->Panes.Add(NewPane);
	SelectPane(LayoutAsset->Panes.Num() - 1);

	RefreshPreview();
}

void SVMLayoutDesignerWidget::DeleteSelectedPane()
{
	if (!HasValidSelection()) return;

	FScopedTransaction Transaction(NSLOCTEXT("VMLayoutEditor", "DeletePane", "Delete Pane"));
	LayoutAsset->Modify();

	LayoutAsset->Panes.RemoveAt(SelectedPaneIndex);
	SelectedPaneIndex = INDEX_NONE;
	PaneEditorObject.Reset();
	PaneDetailsView->SetObject(nullptr);

	RefreshPreview();
}

void SVMLayoutDesignerWidget::BringToFront()
{
	if (!HasValidSelection()) return;

	FScopedTransaction Transaction(NSLOCTEXT("VMLayoutEditor", "BringToFront", "Bring to Front"));
	LayoutAsset->Modify();

	int32 MaxDepth = 0;
	for (const auto& Pane : LayoutAsset->Panes)
		MaxDepth = FMath::Max(MaxDepth, Pane.Depth);

	LayoutAsset->Panes[SelectedPaneIndex].Depth = MaxDepth + 1;
	RefreshPreview();
}

void SVMLayoutDesignerWidget::SendToBack()
{
	if (!HasValidSelection()) return;

	FScopedTransaction Transaction(NSLOCTEXT("VMLayoutEditor", "SendToBack", "Send to Back"));
	LayoutAsset->Modify();

	int32 MinDepth = 0;
	for (const auto& Pane : LayoutAsset->Panes)
		MinDepth = FMath::Min(MinDepth, Pane.Depth);

	LayoutAsset->Panes[SelectedPaneIndex].Depth = MinDepth - 1;
	RefreshPreview();
}

TSharedRef<SWidget> SVMLayoutDesignerWidget::CreateTemplatePickerMenu()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	TArray<FVMLayoutTemplate> Templates = FVMLayoutTemplateLibrary::GetAllTemplates();

	for (const FVMLayoutTemplate& Template : Templates)
	{
		MenuBuilder.AddMenuEntry(
			Template.Name,
			Template.Description,
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([this, Template]()
				{
					if (!LayoutAsset.IsValid()) return;

					FScopedTransaction Transaction(
						FText::Format(NSLOCTEXT("VMLayoutEditor", "ApplyTemplate", "Apply Template: {0}"), Template.Name)
					);

					LayoutAsset->Modify();
					LayoutAsset->Panes = Template.Panes;

					SelectedPaneIndex = INDEX_NONE;
					PaneDetailsView->SetObject(nullptr);
					RefreshPreview();
				})
			)
		);
	}

	return MenuBuilder.MakeWidget();
}
