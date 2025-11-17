#include "VMSplitLayoutAsset.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "VMLog.h"

void UVMSplitLayoutAsset::ValidateLayout()
{
	TSet<int32> UsedIndices;
	TArray<FString> Warnings;

	for (int32 i = 0; i < Panes.Num(); ++i)
	{
		const FVMSplitPane& Pane = Panes[i];
		
		if (Pane.LocalPlayerIndex < 0)
		{
			Warnings.Add(FString::Printf(TEXT("Pane %d has invalid LocalPlayerIndex %d. Must be zero or positive."), i, Pane.LocalPlayerIndex));
		}
		else if (UsedIndices.Contains(Pane.LocalPlayerIndex))
		{
			Warnings.Add(FString::Printf(TEXT("LocalPlayerIndex %d is used by multiple panes."), Pane.LocalPlayerIndex));
		}
		else
		{
			UsedIndices.Add(Pane.LocalPlayerIndex);
		}

		if (Pane.CameraMode == EVMViewportCameraMode::Custom && !Pane.CustomPawnClass)
		{
			Warnings.Add(FString::Printf(TEXT("Pane %d is set to Custom camera mode but has no CustomPawnClass assigned."), i));
		}

		if (Pane.Rect.Origin01.X < 0.f || Pane.Rect.Origin01.X > 1.f ||
			Pane.Rect.Origin01.Y < 0.f || Pane.Rect.Origin01.Y > 1.f)
		{
			Warnings.Add(FString::Printf(TEXT("Pane %d has invalid origin (%.2f, %.2f). Must be in range [0,1]."), 
				i, Pane.Rect.Origin01.X, Pane.Rect.Origin01.Y));
		}

		if (Pane.Rect.Size01.X <= 0.f || Pane.Rect.Size01.X > 1.f ||
			Pane.Rect.Size01.Y <= 0.f || Pane.Rect.Size01.Y > 1.f)
		{
			Warnings.Add(FString::Printf(TEXT("Pane %d has invalid size (%.2f, %.2f). Must be in range (0,1]."), 
				i, Pane.Rect.Size01.X, Pane.Rect.Size01.Y));
		}

		if (Pane.Rect.Origin01.X + Pane.Rect.Size01.X > 1.f ||
			Pane.Rect.Origin01.Y + Pane.Rect.Size01.Y > 1.f)
		{
			Warnings.Add(FString::Printf(TEXT("Pane %d extends beyond viewport bounds."), i));
		}
	}

	for (const FString& Warning : Warnings)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("VMSplitLayoutAsset: %s"), *Warning);
	}

	if (Warnings.Num() == 0)
	{
		UE_LOG(LogViewportManager, Log, TEXT("VMSplitLayoutAsset: Layout validation passed."));
	}
}

#if WITH_EDITOR
void UVMSplitLayoutAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		ValidateLayout();
	}
}
#endif



