#pragma once

#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "VMSplitLayoutAsset.h"
#include "VMLayoutPaneEditorObject.generated.h"

class UVMSplitLayoutAsset;

/**
 * Transient UObject wrapper that exposes an FVMSplitPane to the details panel.
 */
UCLASS()
class UVMLayoutPaneEditorObject : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UVMSplitLayoutAsset* InLayoutAsset, int32 InPaneIndex);

	// Pane data that can be edited in the details panel.
	UPROPERTY(EditAnywhere, Category = "Viewport Manager")
	FVMSplitPane Pane;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	void ApplyToLayout(bool bMarkDirty = true);

private:
	UPROPERTY()
	TObjectPtr<UVMSplitLayoutAsset> LayoutAsset = nullptr;

	int32 PaneIndex = INDEX_NONE;
};
