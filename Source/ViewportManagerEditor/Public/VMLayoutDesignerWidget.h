// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "VMSplitLayoutAsset.h"
#include "VMLayoutDesignerWidget.generated.h"

class UCanvasPanel;
class UBorder;
class UTextBlock;
class UButton;
/**
 * Visual layout designer widget for creating and editing VMSplitLayoutAsset configurations
 * Provides drag-and-drop interface for designing viewport layouts
 */
UCLASS(BlueprintType)
class VIEWPORTMANAGEREDITOR_API UVMLayoutDesignerWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	UVMLayoutDesignerWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer")
	void SetLayoutAsset(UVMSplitLayoutAsset* InLayoutAsset);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VM Layout Designer")
	UVMSplitLayoutAsset* GetLayoutAsset() const { return CurrentLayoutAsset; }

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer")
	void RefreshLayout();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer")
	void AddNewPane();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer")
	void RemoveSelectedPane();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer")
	void ApplyTwoPlayerHorizontal();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer")
	void ApplyTwoPlayerVertical();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer")
	void ApplyFourPlayerGrid();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "VM Layout Designer")
	TObjectPtr<UVMSplitLayoutAsset> CurrentLayoutAsset;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Layout Designer")
	TObjectPtr<UCanvasPanel> DesignCanvas;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Layout Designer")
	TObjectPtr<UButton> AddPaneButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Layout Designer")
	TObjectPtr<UButton> RemovePaneButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Layout Designer")
	TObjectPtr<UButton> TwoPlayerHorizontalButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Layout Designer")
	TObjectPtr<UButton> TwoPlayerVerticalButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Layout Designer")
	TObjectPtr<UButton> FourPlayerGridButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Layout Designer")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBorder>> PaneVisuals;

	UPROPERTY(BlueprintReadOnly, Category = "VM Layout Designer")
	int32 SelectedPaneIndex = -1;

	UPROPERTY(BlueprintReadWrite, Category = "VM Layout Designer|Grid")
	bool bSnapToGrid = true;

	UPROPERTY(BlueprintReadWrite, Category = "VM Layout Designer|Grid")
	float GridSize = 0.05f;

	UPROPERTY(BlueprintReadWrite, Category = "VM Layout Designer|Grid")
	bool bShowGrid = true;

	UPROPERTY(Transient)
	TArray<UVMSplitLayoutAsset*> UndoStack;

	UPROPERTY(Transient)
	TArray<UVMSplitLayoutAsset*> RedoStack;

	UPROPERTY(Transient)
	bool bIsDraggingPane = false;

	UPROPERTY(Transient)
	int32 DraggingPaneIndex = -1;

	UPROPERTY(Transient)
	FVector2D DragStartPosition;

	void CreatePaneVisuals();

	UBorder* CreatePaneVisual(int32 PaneIndex, const FVMSplitPane& Pane);

	void UpdatePaneVisual(int32 PaneIndex);

	UFUNCTION()
	void OnPaneClicked(int32 PaneIndex);

	void UpdateStatusText();

	void BindButtonEvents();

	FLinearColor GetPaneColor(int32 LocalPlayerIndex) const;

public:
	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Grid")
	FVector2f SnapToGrid(const FVector2f& Value) const;

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Grid")
	void ToggleSnapToGrid();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Grid")
	void SetGridSize(float NewGridSize);

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Alignment")
	void AlignSelectedPaneLeft();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Alignment")
	void AlignSelectedPaneRight();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Alignment")
	void AlignSelectedPaneTop();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Alignment")
	void AlignSelectedPaneBottom();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Alignment")
	void AlignSelectedPaneCenter();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Alignment")
	void AlignSelectedPaneMiddle();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Alignment")
	void DistributePanesHorizontally();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Alignment")
	void DistributePanesVertically();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Undo")
	void Undo();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Undo")
	void Redo();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Undo")
	bool CanUndo() const;

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Undo")
	bool CanRedo() const;

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Undo")
	void PushUndoState();

	UFUNCTION(BlueprintCallable, Category = "VM Layout Designer|Undo")
	void ClearUndoHistory();

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UVMSplitLayoutAsset* DuplicateLayoutAsset(UVMSplitLayoutAsset* Source);
};


