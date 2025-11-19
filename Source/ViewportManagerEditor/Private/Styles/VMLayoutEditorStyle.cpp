// Copyright jackcayc924 2025. All Rights Reserved.

#include "Styles/VMLayoutEditorStyle.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FVMLayoutEditorStyle::StyleInstance = nullptr;

// Modern Figma-inspired color palette
const FLinearColor FVMLayoutEditorStyle::PrimaryBlue(0.2f, 0.6f, 1.0f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::AccentGreen(0.3f, 0.85f, 0.4f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::BackgroundDark(0.08f, 0.08f, 0.09f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::SurfaceGray(0.15f, 0.15f, 0.16f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::SurfaceLight(0.22f, 0.22f, 0.23f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::TextPrimary(0.95f, 0.95f, 0.95f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::TextSecondary(0.7f, 0.7f, 0.7f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::HandleBlue(0.3f, 0.7f, 1.0f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::SnapGuideYellow(1.0f, 0.9f, 0.2f, 0.9f);
const FLinearColor FVMLayoutEditorStyle::ErrorRed(0.9f, 0.25f, 0.25f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::SuccessGreen(0.3f, 0.9f, 0.4f, 1.0f);
const FLinearColor FVMLayoutEditorStyle::GridLine(0.3f, 0.3f, 0.35f, 0.15f);

void FVMLayoutEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FVMLayoutEditorStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

const ISlateStyle& FVMLayoutEditorStyle::Get()
{
	check(StyleInstance.IsValid());
	return *StyleInstance;
}

FName FVMLayoutEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("VMLayoutEditorStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FVMLayoutEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShared<FSlateStyleSet>(GetStyleSetName());

	// Set content root to plugin resources directory
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("ViewportManager");
	if (Plugin.IsValid())
	{
		Style->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
	}

	// Define modern button styles
	FButtonStyle ModernButton = FButtonStyle()
		.SetNormal(FSlateRoundedBoxBrush(SurfaceGray, 4.0f, FLinearColor(0.2f, 0.2f, 0.25f, 1.0f), 1.0f))
		.SetHovered(FSlateRoundedBoxBrush(SurfaceLight, 4.0f, PrimaryBlue * 0.3f, 1.0f))
		.SetPressed(FSlateRoundedBoxBrush(SurfaceGray, 4.0f, PrimaryBlue * 0.5f, 1.0f))
		.SetNormalPadding(FMargin(8.f, 4.f))
		.SetPressedPadding(FMargin(8.f, 5.f, 8.f, 3.f));

	Style->Set("VMLayoutEditor.ModernButton", ModernButton);

	// Define pane border style (rounded corners)
	FSlateRoundedBoxBrush PaneBorder(SurfaceGray, 4.0f, FLinearColor(0.3f, 0.3f, 0.35f, 0.8f), 1.0f);
	Style->Set("VMLayoutEditor.PaneBorder", new FSlateRoundedBoxBrush(PaneBorder));

	// Selected pane border (brighter blue outline)
	FSlateRoundedBoxBrush SelectedPaneBorder(SurfaceGray, 4.0f, PrimaryBlue, 2.0f);
	Style->Set("VMLayoutEditor.SelectedPaneBorder", new FSlateRoundedBoxBrush(SelectedPaneBorder));

	// Handle style (small circular resize handles)
	FSlateBrush HandleBrush;
	HandleBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
	HandleBrush.TintColor = FSlateColor(HandleBlue);
	HandleBrush.OutlineSettings.Color = FLinearColor::White;
	HandleBrush.OutlineSettings.Width = 1.5f;
	HandleBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
	HandleBrush.OutlineSettings.CornerRadii = FVector4(6.0f, 6.0f, 6.0f, 6.0f);
	Style->Set("VMLayoutEditor.ResizeHandle", new FSlateBrush(HandleBrush));

	// Snap guide line
	Style->Set("VMLayoutEditor.SnapGuide", new FSlateColorBrush(SnapGuideYellow));

	// Grid line
	Style->Set("VMLayoutEditor.GridLine", new FSlateColorBrush(GridLine));

	// Player label background
	FSlateRoundedBoxBrush PlayerLabelBG(PrimaryBlue * 0.8f, 3.0f, FLinearColor::Transparent, 0.0f);
	Style->Set("VMLayoutEditor.PlayerLabel", new FSlateRoundedBoxBrush(PlayerLabelBG));

	// Text styles
	FTextBlockStyle NormalText = FTextBlockStyle()
		.SetColorAndOpacity(TextPrimary)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 9));

	Style->Set("VMLayoutEditor.NormalText", NormalText);

	FTextBlockStyle BoldText = FTextBlockStyle()
		.SetColorAndOpacity(TextPrimary)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10));

	Style->Set("VMLayoutEditor.BoldText", BoldText);

	FTextBlockStyle PlayerLabelText = FTextBlockStyle()
		.SetColorAndOpacity(FLinearColor::White)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		.SetShadowOffset(FVector2D(1.0f, 1.0f))
		.SetShadowColorAndOpacity(FLinearColor::Black * 0.8f);

	Style->Set("VMLayoutEditor.PlayerLabelText", PlayerLabelText);

	return Style;
}
