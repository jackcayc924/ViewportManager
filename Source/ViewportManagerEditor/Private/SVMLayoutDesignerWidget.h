#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Templates/SharedPointer.h"
#include "VMLayoutPaneEditorObject.h"

class UVMSplitLayoutAsset;
class IDetailsView;
class SBorder;
class SComboButton;
class SConstraintCanvas;
struct FAssetData;

/**
 * Layout designer window body. Provides a live preview canvas and a details panel
 * for editing individual panes.
 */
class SVMLayoutDesignerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVMLayoutDesignerWidget)
		: _InitialAsset(nullptr)
		, _bShowAssetPicker(true)
	{}
		SLATE_ARGUMENT(UVMSplitLayoutAsset*, InitialAsset)
		SLATE_ARGUMENT(bool, bShowAssetPicker)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	~SVMLayoutDesignerWidget();

private:
	void SetLayoutAsset(UVMSplitLayoutAsset* NewAsset);
	void HandleLayoutAssetChanged(const FAssetData& AssetData);
	void SelectPane(int32 PaneIndex);
	void RefreshPreview();
	FReply HandlePaneMouseDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, int32 PaneIndex);
	FReply HandleSnapButtonClicked(float Increment);
	FReply HandleSnapToEdgeClicked(int32 Edge); // 0=left, 1=right, 2=top, 3=bottom
	void BuildToolbar(const TSharedRef<SHorizontalBox>& Toolbar);
	TSharedRef<SWidget> CreateLayoutAssetPickerMenu();
	void OnLayoutAssetPicked(const FAssetData& AssetData);
	void OnLayoutAssetPickedFromEnter(const TArray<FAssetData>& AssetList);
	void OnLayoutPropertyChanged(UObject* Object, struct FPropertyChangedEvent& PropertyChangedEvent);
	bool HasValidSelection() const;
	FText GetActiveLayoutName() const;

	// Command system
	void BindCommands();
	void DuplicateSelectedPane();
	void DeleteSelectedPane();
	void BringToFront();
	void SendToBack();
	TSharedRef<SWidget> CreateTemplatePickerMenu();
	void BuildContextMenu(FMenuBuilder& MenuBuilder);

	// Keyboard support
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	// Performance optimization
	void MarkDirty(bool bFullRefresh = false);
	void UpdatePaneSlotGeometry(int32 PaneIndex);

	// Resize handle detection
	enum class EResizeHandle
	{
		None,
		Left,
		Right,
		Top,
		Bottom,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

	EResizeHandle GetResizeHandleAtPosition(const FGeometry& PaneGeometry, const FVector2D& LocalMousePos) const;
	FCursorReply GetCursorForHandle(EResizeHandle Handle) const;

	// Drag handling
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	bool bIsDraggingPane = false;
	int32 DraggingPaneIndex = INDEX_NONE;
	EResizeHandle ActiveResizeHandle = EResizeHandle::None;
	FVector2D DragStartMousePos;
	FVMSplitRect DragStartPaneRect;

	TWeakObjectPtr<UVMSplitLayoutAsset> LayoutAsset;
	TSharedPtr<IDetailsView> PaneDetailsView;
	TStrongObjectPtr<UVMLayoutPaneEditorObject> PaneEditorObject;
	int32 SelectedPaneIndex = INDEX_NONE;

	TSharedPtr<SBorder> PreviewContainer;
	TSharedPtr<SConstraintCanvas> PreviewCanvas;
	TSharedPtr<SComboButton> LayoutAssetPickerButton;
	TSharedPtr<class SOverlay> SnapGuideOverlay;

	FDelegateHandle LayoutPropertyChangedHandle;
	bool bAllowAssetPicker = true;

	// Command list for keyboard shortcuts
	TSharedPtr<class FUICommandList> CommandList;

	// Performance optimization
	bool bNeedsFullRefresh = true;
	TSet<int32> bNeedsPaneUpdate;

	// Grid overlay
	bool bShowGrid = false;

	// Snap guides
	TArray<TSharedPtr<class SImage>> ActiveSnapGuides;
	void ShowSnapGuide(bool bVertical, float Position);
	void ClearSnapGuides();
};
