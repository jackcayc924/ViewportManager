#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "VMSplitLayoutAsset.h"
#include "VMViewportHUDWidget.generated.h"

/**
 * Optional convenience base class for viewport-specific HUD widgets
 * Provides helper functions to access the local player and controller
 * No longer required for rendering - any UUserWidget can be used directly
 */
UCLASS(BlueprintType, Blueprintable)
class VIEWPORTMANAGER_API UVMViewportHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UVMViewportHUDWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintCallable, Category = "VM Viewport HUD")
	void SetViewportInfo(int32 InLocalPlayerIndex, const FVMSplitRect& InViewportRect);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VM Viewport HUD")
	int32 GetLocalPlayerIndex() const { return LocalPlayerIndex; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VM Viewport HUD")
	FVMSplitRect GetViewportRect() const { return ViewportRect; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VM Viewport HUD")
	class APlayerController* GetViewportPlayerController() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "VM Viewport HUD")
	class ULocalPlayer* GetViewportLocalPlayer() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "VM Viewport HUD")
	void OnViewportAssigned(int32 InLocalPlayerIndex);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "VM Viewport HUD")
	int32 LocalPlayerIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "VM Viewport HUD")
	FVMSplitRect ViewportRect;
};


