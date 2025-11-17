#include "VMGameViewportClient.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UnrealClient.h"
#include "VMCameraPawn.h"
#include "VMFreeCameraPawn.h"
#include "VMHUDRootWidget.h"
#include "VMViewportHUDWidget.h"
#include "VMViewportManagerSettings.h"
#include "Components/Widget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetTree.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "VMLog.h"

UVMGameViewportClient::UVMGameViewportClient()
{
	ActiveKeyboardMouseLP = 0;
	FocusedPlayerIndex = 0;
	CurrentLayoutAsset = nullptr;
	HUDRootWidget = nullptr;
	HUDRootCanvas = nullptr;
	bClickToFocusEnabled = true;
	bFocusHighlightingEnabled = false;
}

void UVMGameViewportClient::LayoutPlayers()
{
	if (!CurrentLayoutAsset || PlayerRects.Num() == 0)
	{
		Super::LayoutPlayers();
		return;
	}

	UGameInstance* LocalGameInstance = GetGameInstance();
	if (!LocalGameInstance)
	{
		Super::LayoutPlayers();
		return;
	}

	for (const auto& PlayerRectPair : PlayerRects)
	{
		const int32 LocalPlayerIndex = PlayerRectPair.Key;
		const FVMSplitRect& Rect = PlayerRectPair.Value;

		if (ULocalPlayer* LocalPlayer = LocalGameInstance->GetLocalPlayerByIndex(LocalPlayerIndex))
		{
			LocalPlayer->Origin = FVector2D(Rect.Origin01.X, Rect.Origin01.Y);
			LocalPlayer->Size = FVector2D(Rect.Size01.X, Rect.Size01.Y);
		}
	}

	const int32 NumLocalPlayers = LocalGameInstance->GetNumLocalPlayers();
	for (int32 i = 0; i < NumLocalPlayers; ++i)
	{
		if (!PlayerRects.Contains(i))
		{
			if (ULocalPlayer* LocalPlayer = LocalGameInstance->GetLocalPlayerByIndex(i))
			{
				LocalPlayer->Size = FVector2D(0.0f, 0.0f);
			}
		}
	}
}

bool UVMGameViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (!EventArgs.IsGamepad())
	{
		FVector2D MousePos;
		if (GetMousePosition(MousePos))
		{
			const FIntPoint VPSize = Viewport->GetSizeXY();
			const FVector2D N = FVector2D(MousePos.X / VPSize.X, MousePos.Y / VPSize.Y);
			const int32 TargetLP = FindPaneAtScreenPosition(N);

			if (TargetLP != -1)
			{
				bool bShouldReceiveInput = true;
				if (CurrentLayoutAsset)
				{
					for (const FVMSplitPane& Pane : CurrentLayoutAsset->Panes)
					{
						if (Pane.LocalPlayerIndex == TargetLP)
						{
							bShouldReceiveInput = Pane.bReceivesKeyboardMouse;
							break;
						}
					}
				}

				if (bShouldReceiveInput)
				{
					if (EventArgs.Event == IE_Pressed && EventArgs.Key == EKeys::LeftMouseButton)
					{
						HandleClickToFocus(N);
					}

					if (ULocalPlayer* LP = GetGameInstance()->GetLocalPlayerByIndex(TargetLP))
					{
						if (APlayerController* PC = LP->GetPlayerController(GetWorld()))
						{
							PRAGMA_DISABLE_DEPRECATION_WARNINGS
							return PC->InputKey(EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.IsGamepad());
							PRAGMA_ENABLE_DEPRECATION_WARNINGS
						}
					}
				}
			}
		}
		return false;
	}

	return Super::InputKey(EventArgs);
}

bool UVMGameViewportClient::InputAxis(FViewport* InViewport, FInputDeviceId InputDevice, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	if (!bGamepad)
	{
		FVector2D MousePos;
		if (GetMousePosition(MousePos))
		{
			const FIntPoint VPSize = Viewport->GetSizeXY();
			const FVector2D N = FVector2D(MousePos.X / VPSize.X, MousePos.Y / VPSize.Y);
			const int32 TargetLP = FindPaneAtScreenPosition(N);

			if (TargetLP != -1)
			{
				bool bShouldReceiveInput = true;
				if (CurrentLayoutAsset)
				{
					for (const FVMSplitPane& Pane : CurrentLayoutAsset->Panes)
					{
						if (Pane.LocalPlayerIndex == TargetLP)
						{
							bShouldReceiveInput = Pane.bReceivesKeyboardMouse;
							break;
						}
					}
				}

				if (bShouldReceiveInput)
				{
					if (ULocalPlayer* LP = GetGameInstance()->GetLocalPlayerByIndex(TargetLP))
					{
						if (APlayerController* PC = LP->GetPlayerController(GetWorld()))
						{
							PRAGMA_DISABLE_DEPRECATION_WARNINGS
							return PC->InputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
							PRAGMA_ENABLE_DEPRECATION_WARNINGS
						}
					}
				}
			}
		}
		return false;
	}

	return Super::InputAxis(InViewport, InputDevice, Key, Delta, DeltaTime, NumSamples, bGamepad);
}

void UVMGameViewportClient::ApplyLayout(UVMSplitLayoutAsset* LayoutAsset)
{
	if (!LayoutAsset)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMGameViewportClient::ApplyLayout - LayoutAsset is null"));
		return;
	}

	CurrentLayoutAsset = LayoutAsset;
	PlayerRects.Empty();

	TSet<int32> UsedIndices;

	for (const FVMSplitPane& Pane : LayoutAsset->Panes)
	{
		if (Pane.LocalPlayerIndex < 0)
		{
			UE_LOG(LogViewportManager, Warning, TEXT("UVMGameViewportClient::ApplyLayout - Invalid LocalPlayerIndex %d"), Pane.LocalPlayerIndex);
			continue;
		}

		if (UsedIndices.Contains(Pane.LocalPlayerIndex))
		{
			UE_LOG(LogViewportManager, Warning, TEXT("UVMGameViewportClient::ApplyLayout - Duplicate LocalPlayerIndex %d"), Pane.LocalPlayerIndex);
			continue;
		}

		UsedIndices.Add(Pane.LocalPlayerIndex);
		PlayerRects.Add(Pane.LocalPlayerIndex, Pane.Rect);
	}

	const int32 ProcessedPanes = PlayerRects.Num();

	if (!PlayerRects.Contains(ActiveKeyboardMouseLP))
	{
		if (PlayerRects.Num() > 0)
		{
			ActiveKeyboardMouseLP = PlayerRects.CreateConstIterator()->Key;
		}
		else
		{
			ActiveKeyboardMouseLP = 0;
		}
	}

	EnsureLocalPlayersExist();
	SpawnAndPossessPawns();
	EnsureCursorVisibility();
	SetupViewportHUDs();
	RefreshLayout();

	UE_LOG(LogViewportManager, Log, TEXT("UVMGameViewportClient::ApplyLayout - Applied layout with %d panes"), ProcessedPanes);
}

void UVMGameViewportClient::RefreshLayout()
{
	LayoutPlayers();
}

void UVMGameViewportClient::SetActiveLocalPlayer(int32 LocalPlayerIndex)
{
	if (LocalPlayerIndex >= 0 && PlayerRects.Contains(LocalPlayerIndex))
	{
		int32 OldPlayerIndex = FocusedPlayerIndex;
		ActiveKeyboardMouseLP = LocalPlayerIndex;
		FocusedPlayerIndex = LocalPlayerIndex;

		if (OldPlayerIndex != FocusedPlayerIndex)
		{
			OnFocusChanged.Broadcast(OldPlayerIndex, FocusedPlayerIndex);
			UE_LOG(LogViewportManager, Log, TEXT("Focus changed from player %d to player %d"), OldPlayerIndex, FocusedPlayerIndex);
		}

		UE_LOG(LogViewportManager, Log, TEXT("UVMGameViewportClient::SetActiveLocalPlayer - Set active player to %d"), LocalPlayerIndex);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMGameViewportClient::SetActiveLocalPlayer - LocalPlayerIndex %d is not part of the current layout"), LocalPlayerIndex);
	}
}

void UVMGameViewportClient::EnsureLocalPlayersExist()
{
	if (!GetGameInstance())
	{
		return;
	}

	const UVMViewportManagerSettings* Settings = GetDefault<UVMViewportManagerSettings>();
	const bool bAutoAddPlayers = Settings ? Settings->bAutoAddMissingLocalPlayers : true;

	for (auto& PlayerRectPair : PlayerRects)
	{
		const int32 LocalPlayerIndex = PlayerRectPair.Key;

		if (GetGameInstance()->GetLocalPlayerByIndex(LocalPlayerIndex))
		{
			continue;
		}

		const bool bShouldCreatePlayer = bAutoAddPlayers || (CurrentLayoutAsset && CurrentLayoutAsset->bAutoSpawnPlayers);
		if (!bShouldCreatePlayer)
		{
			UE_LOG(LogViewportManager, Verbose, TEXT("Missing LocalPlayer %d but auto-spawn is disabled."), LocalPlayerIndex);
			continue;
		}

		FString CreatePlayerError;
		ULocalPlayer* NewLocalPlayer = GetGameInstance()->CreateLocalPlayer(LocalPlayerIndex, CreatePlayerError, true);
		if (NewLocalPlayer)
		{
			UE_LOG(LogViewportManager, Log, TEXT("Created LocalPlayer %d for layout."), LocalPlayerIndex);
		}
		else
		{
			UE_LOG(LogViewportManager, Warning, TEXT("Failed to create LocalPlayer %d."), LocalPlayerIndex);
			if (!CreatePlayerError.IsEmpty())
			{
				UE_LOG(LogViewportManager, Warning, TEXT("CreateLocalPlayer error: %s"), *CreatePlayerError);
			}
		}
	}
}

void UVMGameViewportClient::SpawnAndPossessPawns()
{
	if (!CurrentLayoutAsset || !GetWorld())
	{
		return;
	}

	for (const FVMSplitPane& Pane : CurrentLayoutAsset->Panes)
	{
		if (!PlayerRects.Contains(Pane.LocalPlayerIndex))
		{
			continue;
		}

		ULocalPlayer* LocalPlayer = GetGameInstance()->GetLocalPlayerByIndex(Pane.LocalPlayerIndex);
		if (!LocalPlayer)
		{
			continue;
		}

		APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld());
		if (!PC)
		{
			continue;
		}

		const TSubclassOf<APawn> DesiredPawnClass = ResolvePawnClass(Pane);
		if (!DesiredPawnClass)
		{
			UE_LOG(LogViewportManager, Warning, TEXT("Pane %d has no valid pawn class; skipping spawn."), Pane.LocalPlayerIndex);
			continue;
		}

		APawn* ActivePawn = PC->GetPawn();
		if (ActivePawn && ActivePawn->GetClass() != DesiredPawnClass.Get())
		{
			PC->UnPossess();
			ActivePawn->Destroy();
			ActivePawn = nullptr;

			UE_LOG(LogViewportManager, Log, TEXT("Destroyed mismatched pawn for LocalPlayer %d (expected %s)."),
				Pane.LocalPlayerIndex, *DesiredPawnClass->GetName());
		}

		if (!ActivePawn)
		{
			FTransform SpawnTransform;
			if (Pane.bUseCustomCameraTransform)
			{
				SpawnTransform = Pane.CameraTransform;
			}
			else if (Pane.bUseCustomFocusPoint)
			{
				const FVector CameraLocation = Pane.FocusPoint + FVector(-Pane.OrbitDistance, 0.0f, Pane.OrbitDistance * 0.5f);
				const FRotator CameraRotation = (Pane.FocusPoint - CameraLocation).Rotation();
				SpawnTransform = FTransform(CameraRotation, CameraLocation, FVector::OneVector);
			}
			else
			{
				const FVector SpawnLocation = FVector(Pane.LocalPlayerIndex * 200.0f, 0.0f, 100.0f);
				SpawnTransform = FTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector);
			}

			ActivePawn = GetWorld()->SpawnActor<APawn>(DesiredPawnClass, SpawnTransform);
			if (!ActivePawn)
			{
				UE_LOG(LogViewportManager, Warning, TEXT("Failed to spawn pawn of class %s for LocalPlayer %d."),
					*DesiredPawnClass->GetName(), Pane.LocalPlayerIndex);
				continue;
			}

			if (Pane.bUseCustomCameraTransform)
			{
				PC->SetControlRotation(SpawnTransform.GetRotation().Rotator());
			}

			PC->Possess(ActivePawn);

			if (Pane.bUseCustomCameraTransform)
			{
				PC->SetControlRotation(SpawnTransform.GetRotation().Rotator());
			}

			UE_LOG(LogViewportManager, Log, TEXT("Spawned %s for LocalPlayer %d at %s (rotation %s)"),
				*DesiredPawnClass->GetName(), Pane.LocalPlayerIndex,
				*ActivePawn->GetActorLocation().ToString(), *ActivePawn->GetActorRotation().ToString());
		}

		ConfigurePawnForPane(ActivePawn, Pane, PC);
	}
}

TSubclassOf<APawn> UVMGameViewportClient::ResolvePawnClass(const FVMSplitPane& Pane) const
{
	switch (Pane.CameraMode)
	{
	case EVMViewportCameraMode::Free:
		if (Pane.PawnClass)
		{
			return Pane.PawnClass;
		}
		return AVMFreeCameraPawn::StaticClass();
	case EVMViewportCameraMode::Custom:
		if (Pane.CustomPawnClass)
		{
			return Pane.CustomPawnClass;
		}
		if (Pane.PawnClass)
		{
			return Pane.PawnClass;
		}
		return AVMCameraPawn::StaticClass();
	case EVMViewportCameraMode::Orbit:
	default:
		if (Pane.PawnClass)
		{
			return Pane.PawnClass;
		}
		return AVMCameraPawn::StaticClass();
	}
}

void UVMGameViewportClient::ConfigurePawnForPane(APawn* Pawn, const FVMSplitPane& Pane, APlayerController* PlayerController)
{
	if (!Pawn)
	{
		return;
	}

	if (AVMCameraPawn* OrbitPawn = Cast<AVMCameraPawn>(Pawn))
	{
		if (Pane.bUseCustomCameraTransform)
		{
			const FVector Location = Pane.CameraTransform.GetLocation();
			const FRotator Rotation = Pane.CameraTransform.GetRotation().Rotator();

			OrbitPawn->SetActorTransform(Pane.CameraTransform);
			OrbitPawn->SetStartingPosition(Location, Rotation);
			OrbitPawn->FocusPoint = Location;
			OrbitPawn->StartingOrbitDistance = Pane.OrbitDistance;
			if (OrbitPawn->SpringArm)
			{
				OrbitPawn->SpringArm->TargetArmLength = Pane.OrbitDistance;
			}

			if (PlayerController)
			{
				PlayerController->SetControlRotation(Rotation);
			}
		}
		else if (Pane.bUseCustomFocusPoint)
		{
			OrbitPawn->FocusPoint = Pane.FocusPoint;
			OrbitPawn->SetOrbitDistance(Pane.OrbitDistance);
		}
		else
		{
			OrbitPawn->StartingOrbitDistance = Pane.OrbitDistance;
			if (OrbitPawn->SpringArm)
			{
				OrbitPawn->SpringArm->TargetArmLength = Pane.OrbitDistance;
			}
		}

		OrbitPawn->ApplyControlSettings(Pane.CameraControls);
		return;
	}

	if (AVMFreeCameraPawn* FreePawn = Cast<AVMFreeCameraPawn>(Pawn))
	{
		if (Pane.bUseCustomCameraTransform)
		{
			const FTransform CameraTransform = Pane.CameraTransform;
			FreePawn->SetActorTransform(CameraTransform);

			if (PlayerController)
			{
				PlayerController->SetControlRotation(CameraTransform.GetRotation().Rotator());
			}
		}

		FreePawn->ApplyControlSettings(Pane.CameraControls);
		return;
	}

	UE_LOG(LogViewportManager, Verbose, TEXT("Pane %d uses custom pawn %s; no automatic configuration applied."),
		Pane.LocalPlayerIndex, *Pawn->GetClass()->GetName());
}

void UVMGameViewportClient::EnsureHUDRoot()
{
    if (HUDRootWidget && HUDRootCanvas) return;

    HUDRootWidget = CreateWidget<UVMHUDRootWidget>(GetWorld(), UVMHUDRootWidget::StaticClass());
    if (!HUDRootWidget)
    {
        UE_LOG(LogViewportManager, Error, TEXT("Failed to create HUD root widget"));
        return;
    }

    if (UVMHUDRootWidget* VMRoot = Cast<UVMHUDRootWidget>(HUDRootWidget))
    {
        HUDRootCanvas = VMRoot->GetRootCanvas();
    }

    if (!HUDRootCanvas)
    {
        UE_LOG(LogViewportManager, Error, TEXT("Failed to get root canvas from HUD root widget"));
        return;
    }

    HUDRootWidget->AddToViewport(1000);
}

void UVMGameViewportClient::ClearPaneHUDs()
{
    for (auto& Pair : ActivePaneHUDs)
    {
        if (UUserWidget* W = Pair.Value.Get())
        {
            if (HUDRootCanvas)
            {
                HUDRootCanvas->RemoveChild(W);
            }
            W->RemoveFromParent();
        }
    }
    ActivePaneHUDs.Empty();
}

void UVMGameViewportClient::SetupViewportHUDs()
{
    if (!CurrentLayoutAsset || !GetWorld()) return;

    ClearPaneHUDs();

    bool bHasAnyHUDs = false;
    for (const FVMSplitPane& Pane : CurrentLayoutAsset->Panes)
    {
        if (PlayerRects.Contains(Pane.LocalPlayerIndex) && Pane.ViewportHUDClass)
        {
            bHasAnyHUDs = true;
            break;
        }
    }

    if (!bHasAnyHUDs)
    {
        UE_LOG(LogViewportManager, Log, TEXT("No HUD classes assigned to any panes, skipping HUD setup"));
        return;
    }

    EnsureHUDRoot();
    if (!HUDRootCanvas)
    {
        UE_LOG(LogViewportManager, Warning, TEXT("Failed to create HUD root canvas"));
        return;
    }

    for (const FVMSplitPane& Pane : CurrentLayoutAsset->Panes)
    {
        if (!PlayerRects.Contains(Pane.LocalPlayerIndex) || !Pane.ViewportHUDClass) continue;

        ULocalPlayer* LP = GetGameInstance()->GetLocalPlayerByIndex(Pane.LocalPlayerIndex);
        if (!LP) continue;

        APlayerController* PC = LP->GetPlayerController(GetWorld());
        if (!PC) continue;

        UUserWidget* HUD = CreateWidget<UUserWidget>(PC, Pane.ViewportHUDClass);
        if (!HUD) continue;

        UCanvasPanelSlot* Slot = HUDRootCanvas->AddChildToCanvas(HUD);
        if (!Slot) { HUD->RemoveFromParent(); continue; }

        const FVMSplitRect& R = PlayerRects[Pane.LocalPlayerIndex];
        const float MinX = R.Origin01.X;
        const float MinY = R.Origin01.Y;
        const float MaxX = R.Origin01.X + R.Size01.X;
        const float MaxY = R.Origin01.Y + R.Size01.Y;

        Slot->SetAnchors(FAnchors(MinX, MinY, MaxX, MaxY));
        Slot->SetOffsets(FMargin(0, 0, 0, 0));
        Slot->SetAlignment(FVector2D(0.f, 0.f));

		HUD->SetClipping(EWidgetClipping::ClipToBounds);
		HUD->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

		ActivePaneHUDs.Add(Pane.LocalPlayerIndex, HUD);

		if (UVMViewportHUDWidget* VMHUD = Cast<UVMViewportHUDWidget>(HUD))
		{
			VMHUD->SetViewportInfo(Pane.LocalPlayerIndex, R);
		}

		UE_LOG(LogViewportManager, Log, TEXT("HUD added for LP %d with anchors (%.2f,%.2f)-(%.2f,%.2f)"),
			Pane.LocalPlayerIndex, MinX, MinY, MaxX, MaxY);
	}
}

void UVMGameViewportClient::HandleClickToFocus(const FVector2D& ScreenPosition)
{
	if (!bClickToFocusEnabled)
	{
		return;
	}

	int32 PaneIndex = FindPaneAtScreenPosition(ScreenPosition);
	if (PaneIndex != -1)
	{
		if (PaneIndex != FocusedPlayerIndex)
		{
			SetActiveLocalPlayer(PaneIndex);
		}

		if (ULocalPlayer* LocalPlayer = GetGameInstance()->GetLocalPlayerByIndex(PaneIndex))
		{
			if (APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
			{
				bool bWantCursorVisible = true;
				if (CurrentLayoutAsset)
				{
					if (const FVMSplitPane* Pane = CurrentLayoutAsset->Panes.FindByPredicate([PaneIndex](const FVMSplitPane& Candidate)
						{
							return Candidate.LocalPlayerIndex == PaneIndex;
						}))
					{
						bWantCursorVisible = Pane->CameraControls.bKeepMouseCursorVisible;
					}
				}

				PC->bShowMouseCursor = bWantCursorVisible;
				if (bWantCursorVisible)
				{
					PC->SetInputMode(FInputModeGameAndUI());
				}

				UE_LOG(LogViewportManager, Verbose, TEXT("HandleClickToFocus - Pane %d focused, cursor %s"),
					PaneIndex, bWantCursorVisible ? TEXT("visible") : TEXT("hidden"));
			}
		}
	}
}

int32 UVMGameViewportClient::FindPaneAtScreenPosition(const FVector2D& ScreenPosition) const
{
	for (auto& PlayerRectPair : PlayerRects)
	{
		int32 LocalPlayerIndex = PlayerRectPair.Key;
		const FVMSplitRect& Rect = PlayerRectPair.Value;

		if (ScreenPosition.X >= Rect.Origin01.X && ScreenPosition.X <= (Rect.Origin01.X + Rect.Size01.X) &&
			ScreenPosition.Y >= Rect.Origin01.Y && ScreenPosition.Y <= (Rect.Origin01.Y + Rect.Size01.Y))
		{
			return LocalPlayerIndex;
		}
	}

	return -1;
}

void UVMGameViewportClient::EnsureCursorVisibility()
{
	for (auto& PlayerRectPair : PlayerRects)
	{
		int32 LocalPlayerIndex = PlayerRectPair.Key;
		if (ULocalPlayer* LocalPlayer = GetGameInstance()->GetLocalPlayerByIndex(LocalPlayerIndex))
		{
			bool bWantCursorVisible = true;
			if (CurrentLayoutAsset)
			{
				if (const FVMSplitPane* Pane = CurrentLayoutAsset->Panes.FindByPredicate([LocalPlayerIndex](const FVMSplitPane& Candidate)
					{
						return Candidate.LocalPlayerIndex == LocalPlayerIndex;
					}))
				{
					bWantCursorVisible = Pane->CameraControls.bKeepMouseCursorVisible;
				}
			}

			if (APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
			{
				PC->bShowMouseCursor = bWantCursorVisible;
				if (bWantCursorVisible)
				{
					PC->SetInputMode(FInputModeGameAndUI());
				}
				else
				{
					PC->SetInputMode(FInputModeGameOnly());
				}

				UE_LOG(LogViewportManager, Verbose, TEXT("EnsureCursorVisibility -> LP%d cursor %s"),
					LocalPlayerIndex,
					bWantCursorVisible ? TEXT("visible") : TEXT("hidden"));
			}
		}
	}
}




