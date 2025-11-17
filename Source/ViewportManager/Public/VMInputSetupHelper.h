#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VMInputSetupHelper.generated.h"

/**
 * Helper class to automatically create and configure Enhanced Input assets for ViewportManager
 */
UCLASS()
class VIEWPORTMANAGER_API UVMInputSetupHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Automatically creates all InputAction assets and configures IMC_ViewportCamera
	 * Call this once from the editor to set up all input bindings
	 */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "ViewportManager|Setup")
	static void SetupEnhancedInputAssets();
};
