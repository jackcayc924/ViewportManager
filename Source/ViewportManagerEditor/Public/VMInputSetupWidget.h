// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "VMInputSetupWidget.generated.h"

/**
 * Editor Utility Widget for setting up Enhanced Input assets with one click
 */
UCLASS()
class VIEWPORTMANAGEREDITOR_API UVMInputSetupWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	/**
	 * Call this to automatically create all InputAction assets
	 * This is exposed to Blueprint so it can be called from a button
	 */
	UFUNCTION(BlueprintCallable, Category = "Setup")
	void CreateInputAssets();
};
