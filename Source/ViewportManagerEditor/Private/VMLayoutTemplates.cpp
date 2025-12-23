// Copyright jackcayc924 2025. All Rights Reserved.

#include "VMLayoutTemplates.h"

TArray<FVMLayoutTemplate> FVMLayoutTemplateLibrary::GetAllTemplates()
{
	TArray<FVMLayoutTemplate> Templates;

	Templates.Add(CreateSingleFullscreen());
	Templates.Add(CreateTwoPlayerHorizontal());
	Templates.Add(CreateTwoPlayerVertical());
	Templates.Add(CreateFourPlayerGrid());
	Templates.Add(CreatePictureInPicture());
	Templates.Add(CreateThreePlayerAsymmetric());
	Templates.Add(CreateSixPlayerGrid());

	return Templates;
}

FVMLayoutTemplate FVMLayoutTemplateLibrary::CreateSingleFullscreen()
{
	FVMLayoutTemplate Template(
		FText::FromString(TEXT("Single Fullscreen")),
		FText::FromString(TEXT("One viewport taking up the entire screen"))
	);

	FVMSplitPane Pane;
	Pane.LocalPlayerIndex = 0;
	Pane.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	Pane.Rect.Size01 = FVector2f(1.0f, 1.0f);

	Template.Panes.Add(Pane);

	return Template;
}

FVMLayoutTemplate FVMLayoutTemplateLibrary::CreateTwoPlayerHorizontal()
{
	FVMLayoutTemplate Template(
		FText::FromString(TEXT("Two Player - Horizontal Split")),
		FText::FromString(TEXT("Two viewports split horizontally (top/bottom)"))
	);

	// Top pane
	FVMSplitPane TopPane;
	TopPane.LocalPlayerIndex = 0;
	TopPane.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	TopPane.Rect.Size01 = FVector2f(1.0f, 0.5f);
	Template.Panes.Add(TopPane);

	// Bottom pane
	FVMSplitPane BottomPane;
	BottomPane.LocalPlayerIndex = 1;
	BottomPane.Rect.Origin01 = FVector2f(0.0f, 0.5f);
	BottomPane.Rect.Size01 = FVector2f(1.0f, 0.5f);
	Template.Panes.Add(BottomPane);

	return Template;
}

FVMLayoutTemplate FVMLayoutTemplateLibrary::CreateTwoPlayerVertical()
{
	FVMLayoutTemplate Template(
		FText::FromString(TEXT("Two Player - Vertical Split")),
		FText::FromString(TEXT("Two viewports split vertically (left/right)"))
	);

	// Left pane
	FVMSplitPane LeftPane;
	LeftPane.LocalPlayerIndex = 0;
	LeftPane.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	LeftPane.Rect.Size01 = FVector2f(0.5f, 1.0f);
	Template.Panes.Add(LeftPane);

	// Right pane
	FVMSplitPane RightPane;
	RightPane.LocalPlayerIndex = 1;
	RightPane.Rect.Origin01 = FVector2f(0.5f, 0.0f);
	RightPane.Rect.Size01 = FVector2f(0.5f, 1.0f);
	Template.Panes.Add(RightPane);

	return Template;
}

FVMLayoutTemplate FVMLayoutTemplateLibrary::CreateFourPlayerGrid()
{
	FVMLayoutTemplate Template(
		FText::FromString(TEXT("Four Player - Grid")),
		FText::FromString(TEXT("Four equal viewports in a 2x2 grid"))
	);

	// Top-left pane
	FVMSplitPane TopLeft;
	TopLeft.LocalPlayerIndex = 0;
	TopLeft.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	TopLeft.Rect.Size01 = FVector2f(0.5f, 0.5f);
	Template.Panes.Add(TopLeft);

	// Top-right pane
	FVMSplitPane TopRight;
	TopRight.LocalPlayerIndex = 1;
	TopRight.Rect.Origin01 = FVector2f(0.5f, 0.0f);
	TopRight.Rect.Size01 = FVector2f(0.5f, 0.5f);
	Template.Panes.Add(TopRight);

	// Bottom-left pane
	FVMSplitPane BottomLeft;
	BottomLeft.LocalPlayerIndex = 2;
	BottomLeft.Rect.Origin01 = FVector2f(0.0f, 0.5f);
	BottomLeft.Rect.Size01 = FVector2f(0.5f, 0.5f);
	Template.Panes.Add(BottomLeft);

	// Bottom-right pane
	FVMSplitPane BottomRight;
	BottomRight.LocalPlayerIndex = 3;
	BottomRight.Rect.Origin01 = FVector2f(0.5f, 0.5f);
	BottomRight.Rect.Size01 = FVector2f(0.5f, 0.5f);
	Template.Panes.Add(BottomRight);

	return Template;
}

FVMLayoutTemplate FVMLayoutTemplateLibrary::CreatePictureInPicture()
{
	FVMLayoutTemplate Template(
		FText::FromString(TEXT("Picture-in-Picture")),
		FText::FromString(TEXT("Large main viewport with small inset viewport in bottom-right corner"))
	);

	// Main fullscreen pane
	FVMSplitPane MainPane;
	MainPane.LocalPlayerIndex = 0;
	MainPane.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	MainPane.Rect.Size01 = FVector2f(1.0f, 1.0f);
	Template.Panes.Add(MainPane);

	// Small inset pane (bottom-right corner, 25% width, 25% height)
	FVMSplitPane InsetPane;
	InsetPane.LocalPlayerIndex = 1;
	InsetPane.Rect.Origin01 = FVector2f(0.75f, 0.75f);
	InsetPane.Rect.Size01 = FVector2f(0.25f, 0.25f);
	Template.Panes.Add(InsetPane);

	return Template;
}

FVMLayoutTemplate FVMLayoutTemplateLibrary::CreateThreePlayerAsymmetric()
{
	FVMLayoutTemplate Template(
		FText::FromString(TEXT("Three Player - Asymmetric")),
		FText::FromString(TEXT("One large viewport on left, two stacked viewports on right"))
	);

	// Large left pane (takes up 2/3 width)
	FVMSplitPane LeftPane;
	LeftPane.LocalPlayerIndex = 0;
	LeftPane.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	LeftPane.Rect.Size01 = FVector2f(0.667f, 1.0f);
	Template.Panes.Add(LeftPane);

	// Top-right pane (1/3 width, top half)
	FVMSplitPane TopRight;
	TopRight.LocalPlayerIndex = 1;
	TopRight.Rect.Origin01 = FVector2f(0.667f, 0.0f);
	TopRight.Rect.Size01 = FVector2f(0.333f, 0.5f);
	Template.Panes.Add(TopRight);

	// Bottom-right pane (1/3 width, bottom half)
	FVMSplitPane BottomRight;
	BottomRight.LocalPlayerIndex = 2;
	BottomRight.Rect.Origin01 = FVector2f(0.667f, 0.5f);
	BottomRight.Rect.Size01 = FVector2f(0.333f, 0.5f);
	Template.Panes.Add(BottomRight);

	return Template;
}

FVMLayoutTemplate FVMLayoutTemplateLibrary::CreateSixPlayerGrid()
{
	FVMLayoutTemplate Template(
		FText::FromString(TEXT("Six Player - Grid")),
		FText::FromString(TEXT("Six equal viewports in a 3x2 grid"))
	);

	// Row 1
	FVMSplitPane Pane1;
	Pane1.LocalPlayerIndex = 0;
	Pane1.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	Pane1.Rect.Size01 = FVector2f(0.333f, 0.5f);
	Template.Panes.Add(Pane1);

	FVMSplitPane Pane2;
	Pane2.LocalPlayerIndex = 1;
	Pane2.Rect.Origin01 = FVector2f(0.333f, 0.0f);
	Pane2.Rect.Size01 = FVector2f(0.334f, 0.5f);
	Template.Panes.Add(Pane2);

	FVMSplitPane Pane3;
	Pane3.LocalPlayerIndex = 2;
	Pane3.Rect.Origin01 = FVector2f(0.667f, 0.0f);
	Pane3.Rect.Size01 = FVector2f(0.333f, 0.5f);
	Template.Panes.Add(Pane3);

	// Row 2
	FVMSplitPane Pane4;
	Pane4.LocalPlayerIndex = 3;
	Pane4.Rect.Origin01 = FVector2f(0.0f, 0.5f);
	Pane4.Rect.Size01 = FVector2f(0.333f, 0.5f);
	Template.Panes.Add(Pane4);

	FVMSplitPane Pane5;
	Pane5.LocalPlayerIndex = 4;
	Pane5.Rect.Origin01 = FVector2f(0.333f, 0.5f);
	Pane5.Rect.Size01 = FVector2f(0.334f, 0.5f);
	Template.Panes.Add(Pane5);

	FVMSplitPane Pane6;
	Pane6.LocalPlayerIndex = 5;
	Pane6.Rect.Origin01 = FVector2f(0.667f, 0.5f);
	Pane6.Rect.Size01 = FVector2f(0.333f, 0.5f);
	Template.Panes.Add(Pane6);

	return Template;
}
