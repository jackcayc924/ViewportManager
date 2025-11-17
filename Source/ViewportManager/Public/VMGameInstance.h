#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "VMGameInstance.generated.h"

/**
 * Custom Game Instance for ViewportManager that allows unlimited local players
 * The ViewportManager system creates LocalPlayers on-demand by index,
 * bypassing the default engine limit of 4 players
 */
UCLASS()
class VIEWPORTMANAGER_API UVMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UVMGameInstance();
};
