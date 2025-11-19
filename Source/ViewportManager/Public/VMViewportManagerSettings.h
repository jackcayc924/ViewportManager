// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "VMViewportManagerSettings.generated.h"

class UVMSplitLayoutAsset;

UCLASS(config=Game, defaultconfig)
class VIEWPORTMANAGER_API UVMViewportManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UVMViewportManagerSettings();

	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;

	UPROPERTY(EditAnywhere, Config, Category = "Layouts", meta = (AllowedClasses = "/Script/ViewportManager.VMSplitLayoutAsset"))
	TSoftObjectPtr<UVMSplitLayoutAsset> DefaultLayout;

	UPROPERTY(EditAnywhere, Config, Category = "Layouts")
	bool bApplyDefaultLayoutOnWorldInit;

	UPROPERTY(EditAnywhere, Config, Category = "Layouts")
	bool bAutoAddMissingLocalPlayers;
};
