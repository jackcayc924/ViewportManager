// Copyright jackcayc924 2025. All Rights Reserved.

#include "VMSplitSubsystem.h"
#include "VMGameViewportClient.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "VMViewportManagerSettings.h"
#include "VMLog.h"
void UVMSplitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UVMSplitSubsystem::OnWorldInitialized);

	UE_LOG(LogViewportManager, Log, TEXT("UVMSplitSubsystem::Initialize - Subsystem initialized"));
}

void UVMSplitSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);

	Super::Deinitialize();

	UE_LOG(LogViewportManager, Log, TEXT("UVMSplitSubsystem::Deinitialize - Subsystem deinitialized"));
}

void UVMSplitSubsystem::ApplyLayout(UVMSplitLayoutAsset* Layout)
{
	if (!Layout)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitSubsystem::ApplyLayout - Layout is null"));
		return;
	}

	CurrentLayout = Layout;

	if (UVMGameViewportClient* ViewportClient = GetViewportClient())
	{
		ViewportClient->ApplyLayout(Layout);
		UE_LOG(LogViewportManager, Log, TEXT("UVMSplitSubsystem::ApplyLayout - Applied layout to viewport client"));
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitSubsystem::ApplyLayout - Could not get viewport client"));
	}
}

void UVMSplitSubsystem::SetPaneRect(int32 LocalPlayerIndex, float OriginX, float OriginY, float SizeX, float SizeY)
{
	if (!CurrentLayout)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitSubsystem::SetPaneRect - No current layout"));
		return;
	}

	for (FVMSplitPane& Pane : CurrentLayout->Panes)
	{
		if (Pane.LocalPlayerIndex == LocalPlayerIndex)
		{
			Pane.Rect.Origin01 = FVector2f(OriginX, OriginY);
			Pane.Rect.Size01 = FVector2f(SizeX, SizeY);

			if (UVMGameViewportClient* ViewportClient = GetViewportClient())
			{
				ViewportClient->ApplyLayout(CurrentLayout);
			}

			UE_LOG(LogViewportManager, Log, TEXT("UVMSplitSubsystem::SetPaneRect - Updated pane %d rect to (%.2f, %.2f, %.2f, %.2f)"), 
				LocalPlayerIndex, OriginX, OriginY, SizeX, SizeY);
			return;
		}
	}

	UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitSubsystem::SetPaneRect - Could not find pane with LocalPlayerIndex %d"), LocalPlayerIndex);
}

bool UVMSplitSubsystem::GetPaneRect(int32 LocalPlayerIndex, FVector2f& OutOrigin01, FVector2f& OutSize01)
{
	if (!CurrentLayout)
	{
		return false;
	}

	for (const FVMSplitPane& Pane : CurrentLayout->Panes)
	{
		if (Pane.LocalPlayerIndex == LocalPlayerIndex)
		{
			OutOrigin01 = Pane.Rect.Origin01;
			OutSize01 = Pane.Rect.Size01;
			return true;
		}
	}

	return false;
}

void UVMSplitSubsystem::SetPanePawnClass(int32 LocalPlayerIndex, TSubclassOf<APawn> PawnClass)
{
	if (!CurrentLayout)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitSubsystem::SetPanePawnClass - No current layout"));
		return;
	}

	for (FVMSplitPane& Pane : CurrentLayout->Panes)
	{
		if (Pane.LocalPlayerIndex == LocalPlayerIndex)
		{
			Pane.PawnClass = PawnClass;

			if (UVMGameViewportClient* ViewportClient = GetViewportClient())
			{
				ViewportClient->ApplyLayout(CurrentLayout);
			}

			UE_LOG(LogViewportManager, Log, TEXT("UVMSplitSubsystem::SetPanePawnClass - Updated pane %d pawn class"), LocalPlayerIndex);
			return;
		}
	}

	UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitSubsystem::SetPanePawnClass - Could not find pane with LocalPlayerIndex %d"), LocalPlayerIndex);
}

void UVMSplitSubsystem::SetActiveKeyboardMousePlayer(int32 LocalPlayerIndex)
{
	if (UVMGameViewportClient* ViewportClient = GetViewportClient())
	{
		ViewportClient->SetActiveLocalPlayer(LocalPlayerIndex);
	}
	else
	{
		UE_LOG(LogViewportManager, Warning, TEXT("UVMSplitSubsystem::SetActiveKeyboardMousePlayer - Could not get viewport client"));
	}
}

int32 UVMSplitSubsystem::GetActiveKeyboardMousePlayer() const
{
	if (UVMGameViewportClient* ViewportClient = GetViewportClient())
	{
		return ViewportClient->GetActiveLocalPlayer();
	}
	return 0;
}

APlayerController* UVMSplitSubsystem::GetPlayerController(int32 LocalPlayerIndex) const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULocalPlayer* LocalPlayer = GameInstance->GetLocalPlayerByIndex(LocalPlayerIndex))
		{
			if (UWorld* World = GetWorld())
			{
				return LocalPlayer->GetPlayerController(World);
			}
		}
	}
	return nullptr;
}

int32 UVMSplitSubsystem::GetLocalPlayerCount() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetNumLocalPlayers();
	}
	return 0;
}

bool UVMSplitSubsystem::IsVMGameViewportClientActive() const
{
	return GetViewportClient() != nullptr;
}

UVMGameViewportClient* UVMSplitSubsystem::GetViewportClient() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGameViewportClient* ViewportClient = GameInstance->GetGameViewportClient())
		{
			return Cast<UVMGameViewportClient>(ViewportClient);
		}
	}
	return nullptr;
}

void UVMSplitSubsystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (World && World->IsGameWorld())
	{
		LoadDefaultLayout();
	}
}

void UVMSplitSubsystem::LoadDefaultLayout()
{
	const UVMViewportManagerSettings* Settings = GetDefault<UVMViewportManagerSettings>();
	if (!Settings)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("Viewport Manager settings class is unavailable; skipping default layout load."));
		return;
	}

	if (!Settings->bApplyDefaultLayoutOnWorldInit)
	{
		UE_LOG(LogViewportManager, Verbose, TEXT("Default layout application disabled in settings."));
		return;
	}

	if (!Settings->DefaultLayout.IsValid() && !Settings->DefaultLayout.ToSoftObjectPath().IsValid())
	{
		UE_LOG(LogViewportManager, Verbose, TEXT("No default layout configured."));
		return;
	}

	UVMSplitLayoutAsset* Layout = Settings->DefaultLayout.IsValid()
		? Settings->DefaultLayout.Get()
		: Settings->DefaultLayout.LoadSynchronous();

	if (!Layout)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("Failed to load default layout asset '%s'."), *Settings->DefaultLayout.ToString());
		return;
	}

	UE_LOG(LogViewportManager, Log, TEXT("Applying default layout '%s' from settings."), *Layout->GetName());
	ApplyLayout(Layout);
}





