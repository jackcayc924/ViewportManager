#include "VMSplitGameMode.h"
#include "VMSplitSubsystem.h"
#include "VMSplitLayoutAsset.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "VMLog.h"

AVMSplitGameMode::AVMSplitGameMode()
{
	DefaultPawnClass = nullptr;

	bAutoApplyDefaultLayout = true;

	UE_LOG(LogViewportManager, Log, TEXT("AVMSplitGameMode::Constructor - ViewportManager GameMode initialized"));
}

void AVMSplitGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogViewportManager, Log, TEXT("AVMSplitGameMode::BeginPlay - Starting ViewportManager GameMode"));

	if (bAutoApplyDefaultLayout)
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AVMSplitGameMode::ApplyDefaultLayoutIfSet, 0.1f, false);
	}
}

APawn* AVMSplitGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	UE_LOG(LogViewportManager, Log, TEXT("AVMSplitGameMode::SpawnDefaultPawnFor - Blocking default pawn spawn, ViewportManager will handle it"));
	return nullptr;
}

APawn* AVMSplitGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	UE_LOG(LogViewportManager, Log, TEXT("AVMSplitGameMode::SpawnDefaultPawnAtTransform - Blocking default pawn spawn, ViewportManager will handle it"));
	return nullptr;
}

void AVMSplitGameMode::ApplyDefaultLayoutIfSet()
{
	if (!DefaultLayout.IsNull())
	{
		UVMSplitLayoutAsset* LayoutAsset = DefaultLayout.LoadSynchronous();
		if (LayoutAsset)
		{
			if (UVMSplitSubsystem* VMSubsystem = GetGameInstance()->GetSubsystem<UVMSplitSubsystem>())
			{
				VMSubsystem->ApplyLayout(LayoutAsset);
				UE_LOG(LogViewportManager, Log, TEXT("AVMSplitGameMode::ApplyDefaultLayoutIfSet - Applied default layout"));
			}
		}
	}
	else
	{
		UE_LOG(LogViewportManager, Log, TEXT("AVMSplitGameMode::ApplyDefaultLayoutIfSet - No default layout set"));
	}
}



