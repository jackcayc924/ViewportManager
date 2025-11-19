// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VMViewportHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "VMExampleHUDWidget.generated.h"

/**
 * Example HUD widget that demonstrates proper viewport-specific HUD implementation
 * This shows how to create HUD elements that properly clip to their assigned viewport
 */
UCLASS(BlueprintType, Blueprintable)
class VIEWPORTMANAGER_API UVMExampleHUDWidget : public UVMViewportHUDWidget
{
	GENERATED_BODY()

public:
	UVMExampleHUDWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Example HUD")
	TObjectPtr<UTextBlock> PlayerIndexText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Example HUD")
	TObjectPtr<UTextBlock> ViewportInfoText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Example HUD")
	TObjectPtr<UProgressBar> HealthBar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "VM Example HUD")
	TObjectPtr<UButton> TestButton = nullptr;

	UFUNCTION(BlueprintImplementableEvent, Category = "VM Example HUD")
	void OnHUDUpdated();

	UFUNCTION()
	void OnTestButtonClicked();

	void UpdateHUDDisplay();

	void CreateUIElements();
};


