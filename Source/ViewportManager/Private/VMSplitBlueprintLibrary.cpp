// Copyright jackcayc924 2025. All Rights Reserved.

#include "VMSplitBlueprintLibrary.h"
#include "VMSplitSubsystem.h"
#include "VMGameViewportClient.h"
#include "VMCameraPawn.h"
#include "VMExampleHUDWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "VMLog.h"

bool UVMSplitBlueprintLibrary::ApplyLayout(UVMSplitLayoutAsset* LayoutAsset)
{
	if (!LayoutAsset)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::ApplyLayout - LayoutAsset is null"));
		return false;
	}

	if (UVMSplitSubsystem* VMSubsystem = GetVMSplitSubsystem())
	{
		VMSubsystem->ApplyLayout(LayoutAsset);
		UE_LOG(LogViewportManager, Log, TEXT("UVMSplitBlueprintLibrary::ApplyLayout - Applied layout successfully"));
		return true;
	}

	UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::ApplyLayout - Could not get VMSplitSubsystem"));
	return false;
}

bool UVMSplitBlueprintLibrary::ApplySimple2PlayerLayout(TSubclassOf<UUserWidget> HUDClass)
{
	UVMSplitSubsystem* VMSubsystem = GetVMSplitSubsystem();
	if (!VMSubsystem)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::ApplySimple2PlayerLayout - Could not get VMSplitSubsystem"));
		return false;
	}

	UVMSplitLayoutAsset* TempLayout = NewObject<UVMSplitLayoutAsset>();

	FVMSplitPane Pane1;
	Pane1.LocalPlayerIndex = 0;
	Pane1.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	Pane1.Rect.Size01 = FVector2f(0.5f, 1.0f);
	if (HUDClass)
	{
		Pane1.ViewportHUDClass = HUDClass;
	}
	else
	{
		Pane1.ViewportHUDClass = UVMExampleHUDWidget::StaticClass();
	}

	FVMSplitPane Pane2;
	Pane2.LocalPlayerIndex = 1;
	Pane2.Rect.Origin01 = FVector2f(0.5f, 0.0f);
	Pane2.Rect.Size01 = FVector2f(0.5f, 1.0f);
	if (HUDClass)
	{
		Pane2.ViewportHUDClass = HUDClass;
	}
	else
	{
		Pane2.ViewportHUDClass = UVMExampleHUDWidget::StaticClass();
	}

	TempLayout->Panes.Add(Pane1);
	TempLayout->Panes.Add(Pane2);
	TempLayout->bAutoSpawnPlayers = true;

	VMSubsystem->ApplyLayout(TempLayout);
	UE_LOG(LogViewportManager, Log, TEXT("UVMSplitBlueprintLibrary::ApplySimple2PlayerLayout - Applied 2-player layout"));
	return true;
}

bool UVMSplitBlueprintLibrary::ApplySimple4PlayerLayout(TSubclassOf<UUserWidget> HUDClass)
{
	UVMSplitSubsystem* VMSubsystem = GetVMSplitSubsystem();
	if (!VMSubsystem)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::ApplySimple4PlayerLayout - Could not get VMSplitSubsystem"));
		return false;
	}

	UVMSplitLayoutAsset* TempLayout = NewObject<UVMSplitLayoutAsset>();

	FVMSplitPane Pane1;
	Pane1.LocalPlayerIndex = 0;
	Pane1.Rect.Origin01 = FVector2f(0.0f, 0.0f);
	Pane1.Rect.Size01 = FVector2f(0.5f, 0.5f);
	if (HUDClass)
	{
		Pane1.ViewportHUDClass = HUDClass;
	}
	else
	{
		Pane1.ViewportHUDClass = UVMExampleHUDWidget::StaticClass();
	}

	FVMSplitPane Pane2;
	Pane2.LocalPlayerIndex = 1;
	Pane2.Rect.Origin01 = FVector2f(0.5f, 0.0f);
	Pane2.Rect.Size01 = FVector2f(0.5f, 0.5f);
	if (HUDClass)
	{
		Pane2.ViewportHUDClass = HUDClass;
	}
	else
	{
		Pane2.ViewportHUDClass = UVMExampleHUDWidget::StaticClass();
	}

	FVMSplitPane Pane3;
	Pane3.LocalPlayerIndex = 2;
	Pane3.Rect.Origin01 = FVector2f(0.0f, 0.5f);
	Pane3.Rect.Size01 = FVector2f(0.5f, 0.5f);
	if (HUDClass)
	{
		Pane3.ViewportHUDClass = HUDClass;
	}
	else
	{
		Pane3.ViewportHUDClass = UVMExampleHUDWidget::StaticClass();
	}

	FVMSplitPane Pane4;
	Pane4.LocalPlayerIndex = 3;
	Pane4.Rect.Origin01 = FVector2f(0.5f, 0.5f);
	Pane4.Rect.Size01 = FVector2f(0.5f, 0.5f);
	if (HUDClass)
	{
		Pane4.ViewportHUDClass = HUDClass;
	}
	else
	{
		Pane4.ViewportHUDClass = UVMExampleHUDWidget::StaticClass();
	}

	TempLayout->Panes.Add(Pane1);
	TempLayout->Panes.Add(Pane2);
	TempLayout->Panes.Add(Pane3);
	TempLayout->Panes.Add(Pane4);
	TempLayout->bAutoSpawnPlayers = true;

	VMSubsystem->ApplyLayout(TempLayout);
	UE_LOG(LogViewportManager, Log, TEXT("UVMSplitBlueprintLibrary::ApplySimple4PlayerLayout - Applied 4-player layout"));
	return true;
}

UVMSplitSubsystem* UVMSplitBlueprintLibrary::GetVMSplitSubsystem()
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UVMSplitSubsystem>();
		}
	}
	return nullptr;
}

bool UVMSplitBlueprintLibrary::IsVMGameViewportClientActive()
{
	if (UVMSplitSubsystem* VMSubsystem = GetVMSplitSubsystem())
	{
		return VMSubsystem->IsVMGameViewportClientActive();
	}
	return false;
}

void UVMSplitBlueprintLibrary::SetActiveKeyboardMousePlayer(int32 LocalPlayerIndex)
{
	if (UVMSplitSubsystem* VMSubsystem = GetVMSplitSubsystem())
	{
		VMSubsystem->SetActiveKeyboardMousePlayer(LocalPlayerIndex);
	}
}

int32 UVMSplitBlueprintLibrary::GetActiveKeyboardMousePlayer()
{
	if (UVMSplitSubsystem* VMSubsystem = GetVMSplitSubsystem())
	{
		return VMSubsystem->GetActiveKeyboardMousePlayer();
	}
	return 0;
}

// Camera control toggle function implementations
AVMCameraPawn* UVMSplitBlueprintLibrary::GetVMCameraPawn(int32 LocalPlayerIndex)
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(LocalPlayerIndex))
			{
				if (APlayerController* PC = LocalPlayer->GetPlayerController(World))
				{
					return Cast<AVMCameraPawn>(PC->GetPawn());
				}
			}
		}
	}
	return nullptr;
}

void UVMSplitBlueprintLibrary::SetCameraControlsEnabled(int32 LocalPlayerIndex, bool bEnabled)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->SetCameraControlsEnabled(bEnabled);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::SetCameraControlsEnabled - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

bool UVMSplitBlueprintLibrary::GetCameraControlsEnabled(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		return CameraPawn->GetCameraControlsEnabled();
	}
	return false;
}

void UVMSplitBlueprintLibrary::ToggleCameraControls(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->ToggleCameraControls();
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::ToggleCameraControls - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

void UVMSplitBlueprintLibrary::SetOrbitEnabled(int32 LocalPlayerIndex, bool bEnabled)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->SetOrbitEnabled(bEnabled);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::SetOrbitEnabled - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

bool UVMSplitBlueprintLibrary::GetOrbitEnabled(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		return CameraPawn->GetOrbitEnabled();
	}
	return false;
}

void UVMSplitBlueprintLibrary::ToggleOrbit(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->ToggleOrbit();
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::ToggleOrbit - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

void UVMSplitBlueprintLibrary::SetPanEnabled(int32 LocalPlayerIndex, bool bEnabled)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->SetPanEnabled(bEnabled);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::SetPanEnabled - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

bool UVMSplitBlueprintLibrary::GetPanEnabled(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		return CameraPawn->GetPanEnabled();
	}
	return false;
}

void UVMSplitBlueprintLibrary::TogglePan(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->TogglePan();
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::TogglePan - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

void UVMSplitBlueprintLibrary::SetZoomEnabled(int32 LocalPlayerIndex, bool bEnabled)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->SetZoomEnabled(bEnabled);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::SetZoomEnabled - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

bool UVMSplitBlueprintLibrary::GetZoomEnabled(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		return CameraPawn->GetZoomEnabled();
	}
	return false;
}

void UVMSplitBlueprintLibrary::ToggleZoom(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->ToggleZoom();
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::ToggleZoom - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

void UVMSplitBlueprintLibrary::SetTargetActorTrackingEnabled(int32 LocalPlayerIndex, bool bEnabled)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->SetTargetActorTrackingEnabled(bEnabled);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::SetTargetActorTrackingEnabled - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

bool UVMSplitBlueprintLibrary::GetTargetActorTrackingEnabled(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		return CameraPawn->GetTargetActorTrackingEnabled();
	}
	return false;
}

void UVMSplitBlueprintLibrary::ToggleTargetActorTracking(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->ToggleTargetActorTracking();
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::ToggleTargetActorTracking - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

void UVMSplitBlueprintLibrary::SetTargetActor(int32 LocalPlayerIndex, AActor* TargetActor)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->SetTargetActor(TargetActor);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::SetTargetActor - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

AActor* UVMSplitBlueprintLibrary::GetTargetActor(int32 LocalPlayerIndex)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		return CameraPawn->GetTargetActor();
	}
	return nullptr;
}

void UVMSplitBlueprintLibrary::FocusOnActor(int32 LocalPlayerIndex, AActor* Actor, float Distance)
{
	if (AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex))
	{
		CameraPawn->FocusOnActor(Actor, Distance);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::FocusOnActor - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
	}
}

bool UVMSplitBlueprintLibrary::QuickSetupSplitScreen(int32 NumPlayers, bool bHorizontalSplit, TSubclassOf<UUserWidget> HUDClass)
{
	if (NumPlayers == 2)
	{
		if (bHorizontalSplit)
		{
			return ApplySimple2PlayerLayout(HUDClass);
		}
		else
		{
			UVMSplitSubsystem* VMSubsystem = GetVMSplitSubsystem();
			if (!VMSubsystem)
			{
				UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitBlueprintLibrary::QuickSetupSplitScreen - Could not get VMSplitSubsystem"));
				return false;
			}

			UVMSplitLayoutAsset* TempLayout = NewObject<UVMSplitLayoutAsset>();

			FVMSplitPane Pane1;
			Pane1.LocalPlayerIndex = 0;
			Pane1.Rect.Origin01 = FVector2f(0.0f, 0.0f);
			Pane1.Rect.Size01 = FVector2f(1.0f, 0.5f);
			if (HUDClass) Pane1.ViewportHUDClass = HUDClass;

			FVMSplitPane Pane2;
			Pane2.LocalPlayerIndex = 1;
			Pane2.Rect.Origin01 = FVector2f(0.0f, 0.5f);
			Pane2.Rect.Size01 = FVector2f(1.0f, 0.5f);
			if (HUDClass) Pane2.ViewportHUDClass = HUDClass;

			TempLayout->Panes.Add(Pane1);
			TempLayout->Panes.Add(Pane2);
			TempLayout->bAutoSpawnPlayers = true;

			VMSubsystem->ApplyLayout(TempLayout);
			UE_LOG(LogViewportManager, Log, TEXT("QuickSetupSplitScreen - Applied 2-player vertical split"));
			return true;
		}
	}
	else if (NumPlayers == 4)
	{
		return ApplySimple4PlayerLayout(HUDClass);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("QuickSetupSplitScreen - Only 2 or 4 players supported (requested: %d)"), NumPlayers);
		return false;
	}
}

void UVMSplitBlueprintLibrary::ConfigureCameraControls(int32 LocalPlayerIndex, bool bEnableOrbit, bool bEnablePan, bool bEnableZoom, bool bShowCursor)
{
	if (!IsPlayerIndexValid(LocalPlayerIndex))
	{
		UE_LOG(LogViewportManager, Warning, TEXT("ConfigureCameraControls - Invalid player index: %d"), LocalPlayerIndex);
		return;
	}

	AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex);
	if (!CameraPawn)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("ConfigureCameraControls - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
		return;
	}

	CameraPawn->SetOrbitEnabled(bEnableOrbit);
	CameraPawn->SetPanEnabled(bEnablePan);
	CameraPawn->SetZoomEnabled(bEnableZoom);

	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (ULocalPlayer* LP = GameInstance->GetLocalPlayerByIndex(LocalPlayerIndex))
			{
				if (APlayerController* PC = LP->GetPlayerController(World))
				{
					PC->bShowMouseCursor = bShowCursor;
				}
			}
		}
	}

	UE_LOG(LogViewportManager, Log, TEXT("ConfigureCameraControls - Player %d: Orbit=%s, Pan=%s, Zoom=%s, Cursor=%s"),
		LocalPlayerIndex,
		bEnableOrbit ? TEXT("ON") : TEXT("OFF"),
		bEnablePan ? TEXT("ON") : TEXT("OFF"),
		bEnableZoom ? TEXT("ON") : TEXT("OFF"),
		bShowCursor ? TEXT("ON") : TEXT("OFF"));
}

void UVMSplitBlueprintLibrary::FocusCameraOnActor(int32 LocalPlayerIndex, AActor* TargetActor, float Distance, bool bTrackActor)
{
	if (!TargetActor)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("FocusCameraOnActor - TargetActor is null"));
		return;
	}

	AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex);
	if (!CameraPawn)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("FocusCameraOnActor - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
		return;
	}

	CameraPawn->SetTargetActor(TargetActor);

	CameraPawn->SetTargetActorTrackingEnabled(bTrackActor);

	CameraPawn->FocusOnActor(TargetActor, Distance);

	UE_LOG(LogViewportManager, Log, TEXT("FocusCameraOnActor - Player %d focusing on %s (tracking=%s)"),
		LocalPlayerIndex,
		*TargetActor->GetName(),
		bTrackActor ? TEXT("ON") : TEXT("OFF"));
}

int32 UVMSplitBlueprintLibrary::GetActivePlayerCount()
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetNumLocalPlayers();
		}
	}
	return 0;
}

bool UVMSplitBlueprintLibrary::IsPlayerIndexValid(int32 LocalPlayerIndex)
{
	if (LocalPlayerIndex < 0)
		return false;

	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetLocalPlayerByIndex(LocalPlayerIndex) != nullptr;
		}
	}
	return false;
}

APlayerController* UVMSplitBlueprintLibrary::GetPlayerController(int32 LocalPlayerIndex)
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (ULocalPlayer* LP = GameInstance->GetLocalPlayerByIndex(LocalPlayerIndex))
			{
				return LP->GetPlayerController(World);
			}
		}
	}
	return nullptr;
}

void UVMSplitBlueprintLibrary::EnableAllCameraControls(int32 LocalPlayerIndex, bool bEnabled)
{
	AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex);
	if (!CameraPawn)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("EnableAllCameraControls - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
		return;
	}

	CameraPawn->SetOrbitEnabled(bEnabled);
	CameraPawn->SetPanEnabled(bEnabled);
	CameraPawn->SetZoomEnabled(bEnabled);

	UE_LOG(LogViewportManager, Log, TEXT("EnableAllCameraControls - Player %d: All controls %s"),
		LocalPlayerIndex,
		bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UVMSplitBlueprintLibrary::ResetCamera(int32 LocalPlayerIndex)
{
	AVMCameraPawn* CameraPawn = GetVMCameraPawn(LocalPlayerIndex);
	if (!CameraPawn)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("ResetCamera - Could not find VMCameraPawn for LocalPlayer %d"), LocalPlayerIndex);
		return;
	}

	CameraPawn->ResetToStartingPosition();

	UE_LOG(LogViewportManager, Log, TEXT("ResetCamera - Player %d camera reset to starting position"), LocalPlayerIndex);
}

int32 UVMSplitBlueprintLibrary::GetFocusedPlayer()
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UVMGameViewportClient* VMViewportClient = Cast<UVMGameViewportClient>(GameInstance->GetGameViewportClient()))
			{
				return VMViewportClient->GetFocusedPlayer();
			}
		}
	}
	return -1;
}

void UVMSplitBlueprintLibrary::SetFocusToPlayer(int32 LocalPlayerIndex)
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UVMGameViewportClient* VMViewportClient = Cast<UVMGameViewportClient>(GameInstance->GetGameViewportClient()))
			{
				VMViewportClient->SetActiveLocalPlayer(LocalPlayerIndex);
			}
		}
	}
}

void UVMSplitBlueprintLibrary::SetClickToFocusEnabled(bool bEnabled)
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UVMGameViewportClient* VMViewportClient = Cast<UVMGameViewportClient>(GameInstance->GetGameViewportClient()))
			{
				VMViewportClient->SetClickToFocusEnabled(bEnabled);
				UE_LOG(LogViewportManager, Log, TEXT("SetClickToFocusEnabled - Click-to-focus %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
			}
		}
	}
}

bool UVMSplitBlueprintLibrary::IsClickToFocusEnabled()
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UVMGameViewportClient* VMViewportClient = Cast<UVMGameViewportClient>(GameInstance->GetGameViewportClient()))
			{
				return VMViewportClient->IsClickToFocusEnabled();
			}
		}
	}
	return false;
}

void UVMSplitBlueprintLibrary::SetFocusHighlightingEnabled(bool bEnabled)
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UVMGameViewportClient* VMViewportClient = Cast<UVMGameViewportClient>(GameInstance->GetGameViewportClient()))
			{
				VMViewportClient->SetFocusHighlightingEnabled(bEnabled);
				UE_LOG(LogViewportManager, Log, TEXT("SetFocusHighlightingEnabled - Focus highlighting %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
			}
		}
	}
}

bool UVMSplitBlueprintLibrary::IsFocusHighlightingEnabled()
{
	if (UWorld* World = GEngine->GetCurrentPlayWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UVMGameViewportClient* VMViewportClient = Cast<UVMGameViewportClient>(GameInstance->GetGameViewportClient()))
			{
				return VMViewportClient->IsFocusHighlightingEnabled();
			}
		}
	}
	return false;
}



