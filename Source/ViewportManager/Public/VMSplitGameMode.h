// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "VMSplitGameMode.generated.h"

class UVMSplitLayoutAsset;

/**
 * Custom GameMode that works seamlessly with ViewportManager
 * Prevents default pawn spawning and lets ViewportManager handle everything
 */
UCLASS(BlueprintType, Blueprintable)
class VIEWPORTMANAGER_API AVMSplitGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AVMSplitGameMode();

protected:
	virtual void BeginPlay() override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewportManager")
	TSoftObjectPtr<UVMSplitLayoutAsset> DefaultLayout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewportManager")
	bool bAutoApplyDefaultLayout = true;

private:
	void ApplyDefaultLayoutIfSet();
};


