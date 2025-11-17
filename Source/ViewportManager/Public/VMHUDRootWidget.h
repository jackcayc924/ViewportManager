#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "VMHUDRootWidget.generated.h"

/**
 * Root widget that provides a CanvasPanel for hosting viewport-specific HUD widgets
 * This ensures proper clipping and anchoring for viewport HUDs
 */
UCLASS(BlueprintType)
class VIEWPORTMANAGER_API UVMHUDRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "VM HUD Root")
	UCanvasPanel* GetRootCanvas();

private:
	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> RootCanvas = nullptr;

	void EnsureRootCanvas();
};


