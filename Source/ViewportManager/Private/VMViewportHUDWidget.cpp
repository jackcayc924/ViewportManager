#include "VMViewportHUDWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "VMLog.h"

UVMViewportHUDWidget::UVMViewportHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LocalPlayerIndex = -1;
	ViewportRect = FVMSplitRect();
}

void UVMViewportHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UVMViewportHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (LocalPlayerIndex >= 0)
	{
		OnViewportAssigned(LocalPlayerIndex);
	}

	UE_LOG(LogViewportManager, Log, TEXT("UVMViewportHUDWidget::NativeConstruct - HUD constructed for LocalPlayer %d"), LocalPlayerIndex);
}

void UVMViewportHUDWidget::SetViewportInfo(int32 InLocalPlayerIndex, const FVMSplitRect& InViewportRect)
{
	LocalPlayerIndex = InLocalPlayerIndex;
	ViewportRect = InViewportRect;

	if (IsConstructed())
	{
		OnViewportAssigned(LocalPlayerIndex);
	}

	UE_LOG(LogViewportManager, Log, TEXT("UVMViewportHUDWidget::SetViewportInfo - Set viewport info for LocalPlayer %d, Rect: (%.3f, %.3f, %.3f, %.3f)"), 
		LocalPlayerIndex, ViewportRect.Origin01.X, ViewportRect.Origin01.Y, ViewportRect.Size01.X, ViewportRect.Size01.Y);
}

APlayerController* UVMViewportHUDWidget::GetViewportPlayerController() const
{
	if (LocalPlayerIndex < 0)
		return nullptr;

	if (ULocalPlayer* LocalPlayer = GetViewportLocalPlayer())
	{
		return LocalPlayer->GetPlayerController(GetWorld());
	}

	return nullptr;
}

ULocalPlayer* UVMViewportHUDWidget::GetViewportLocalPlayer() const
{
	if (LocalPlayerIndex < 0 || !GEngine)
		return nullptr;

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetLocalPlayerByIndex(LocalPlayerIndex);
	}

	return nullptr;
}



