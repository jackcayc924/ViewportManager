#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

/**
 * Modern Figma-inspired style set for the VM Layout Editor
 * Provides clean, minimal color palette and brushes
 */
class FVMLayoutEditorStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static const ISlateStyle& Get();
	static FName GetStyleSetName();

	// Modern color palette
	static const FLinearColor PrimaryBlue;
	static const FLinearColor AccentGreen;
	static const FLinearColor BackgroundDark;
	static const FLinearColor SurfaceGray;
	static const FLinearColor SurfaceLight;
	static const FLinearColor TextPrimary;
	static const FLinearColor TextSecondary;
	static const FLinearColor HandleBlue;
	static const FLinearColor SnapGuideYellow;
	static const FLinearColor ErrorRed;
	static const FLinearColor SuccessGreen;
	static const FLinearColor GridLine;

private:
	static TSharedPtr<FSlateStyleSet> StyleInstance;
	static TSharedRef<FSlateStyleSet> Create();
};
