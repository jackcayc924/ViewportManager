// Copyright jackcayc924 2025. All Rights Reserved.

#include "VMAutoLayoutActor.h"
#include "VMSplitSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "VMLog.h"

AVMAutoLayoutActor::AVMAutoLayoutActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Set default layout asset path (user can override this)
	LayoutAsset = nullptr;
	ApplyDelay = 0.5f;
}

void AVMAutoLayoutActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Apply layout after a short delay to ensure everything is initialized
	if (LayoutAsset)
	{
		GetWorldTimerManager().SetTimer(
			ApplyLayoutTimerHandle,
			this,
			&AVMAutoLayoutActor::ApplyLayout,
			ApplyDelay,
			false
		);
		
		UE_LOG(LogViewportManager, Log, TEXT("AVMAutoLayoutActor: Scheduled layout application in %.1f seconds"), ApplyDelay);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("AVMAutoLayoutActor: No layout asset assigned! Please set the LayoutAsset property."));
	}
}

void AVMAutoLayoutActor::ApplyLayout()
{
	if (!LayoutAsset)
	{
		UE_LOG(LogViewportManager, Error, TEXT("AVMAutoLayoutActor::ApplyLayout - No layout asset assigned"));
		return;
	}

	UVMSplitSubsystem* SplitSubsystem = GetGameInstance()->GetSubsystem<UVMSplitSubsystem>();
	if (!SplitSubsystem)
	{
		UE_LOG(LogViewportManager, Error, TEXT("AVMAutoLayoutActor::ApplyLayout - Could not get UVMSplitSubsystem"));
		return;
	}

	SplitSubsystem->ApplyLayout(LayoutAsset);

	UE_LOG(LogViewportManager, Log, TEXT("AVMAutoLayoutActor::ApplyLayout - Successfully applied layout with %d panes"), LayoutAsset->Panes.Num());

	TestSetup();
}

void AVMAutoLayoutActor::TestSetup()
{
	UVMSplitSubsystem* SplitSubsystem = GetGameInstance()->GetSubsystem<UVMSplitSubsystem>();
	if (!SplitSubsystem)
	{
		UE_LOG(LogViewportManager, Error, TEXT("AVMAutoLayoutActor::TestSetup - Could not get UVMSplitSubsystem"));
		return;
	}

	UE_LOG(LogViewportManager, Log, TEXT("=== ViewportManager Setup Test ==="));

	int32 PlayerCount = SplitSubsystem->GetLocalPlayerCount();
	UE_LOG(LogViewportManager, Log, TEXT("Local Player Count: %d"), PlayerCount);

	if (LayoutAsset)
	{
		for (int32 i = 0; i < LayoutAsset->Panes.Num(); ++i)
		{
			const FVMSplitPane& Pane = LayoutAsset->Panes[i];

			APlayerController* PC = SplitSubsystem->GetPlayerController(Pane.LocalPlayerIndex);
			if (PC)
			{
				FString PawnName = PC->GetPawn() ? PC->GetPawn()->GetClass()->GetName() : TEXT("None");
				UE_LOG(LogViewportManager, Log, TEXT("Pane %d (LP%d): PlayerController %s, Pawn: %s"), i, Pane.LocalPlayerIndex, *PC->GetName(), *PawnName);
			}
			else
			{
				UE_LOG(LogViewportManager, Warning, TEXT("Pane %d (LP%d): No PlayerController found"), i, Pane.LocalPlayerIndex);
			}

			FVector2f Origin, Size;
			if (SplitSubsystem->GetPaneRect(Pane.LocalPlayerIndex, Origin, Size))
			{
				UE_LOG(LogViewportManager, Log, TEXT("Pane %d Rect: Origin(%.2f, %.2f) Size(%.2f, %.2f)"), 
					i, Origin.X, Origin.Y, Size.X, Size.Y);
			}
		}
	}

	int32 ActivePlayer = SplitSubsystem->GetActiveKeyboardMousePlayer();
	UE_LOG(LogViewportManager, Log, TEXT("Active Keyboard/Mouse Player: %d"), ActivePlayer);
	
	UE_LOG(LogViewportManager, Log, TEXT("=== Setup Test Complete ==="));
	UE_LOG(LogViewportManager, Log, TEXT("Instructions:"));
	UE_LOG(LogViewportManager, Log, TEXT("1. You should see %d viewports on screen"), LayoutAsset ? LayoutAsset->Panes.Num() : 0);
	UE_LOG(LogViewportManager, Log, TEXT("2. Click within a viewport to focus it"));
	UE_LOG(LogViewportManager, Log, TEXT("3. Right Mouse + Drag to orbit camera"));
	UE_LOG(LogViewportManager, Log, TEXT("4. Middle Mouse + Drag to pan camera"));
	UE_LOG(LogViewportManager, Log, TEXT("5. Mouse Wheel to zoom camera"));
	UE_LOG(LogViewportManager, Log, TEXT("6. Left Mouse Click to set focus point"));
}



