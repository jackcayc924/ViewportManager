// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "VMSplitLayoutAsset.h"
#include "VMGameViewportClient.generated.h"


// Delegate for when focus changes between viewports
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVMFocusChangedDelegate, int32, OldPlayerIndex, int32, NewPlayerIndex);

UCLASS(BlueprintType)
class VIEWPORTMANAGER_API UVMGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	UVMGameViewportClient();

	UPROPERTY(BlueprintAssignable, Category = "Viewport Manager|Events")
	FVMFocusChangedDelegate OnFocusChanged;

	virtual void LayoutPlayers() override;
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	void ApplyLayout(UVMSplitLayoutAsset* LayoutAsset);

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	void RefreshLayout();

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	void SetActiveLocalPlayer(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	int32 GetActiveLocalPlayer() const { return ActiveKeyboardMouseLP; }

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager|Focus")
	void SetClickToFocusEnabled(bool bEnabled) { bClickToFocusEnabled = bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager|Focus", BlueprintPure)
	bool IsClickToFocusEnabled() const { return bClickToFocusEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager|Focus")
	void SetFocusHighlightingEnabled(bool bEnabled) { bFocusHighlightingEnabled = bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager|Focus", BlueprintPure)
	bool IsFocusHighlightingEnabled() const { return bFocusHighlightingEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager|Focus", BlueprintPure)
	int32 GetFocusedPlayer() const { return FocusedPlayerIndex; }

protected:
	UPROPERTY()
	TObjectPtr<UVMSplitLayoutAsset> CurrentLayoutAsset;

	TMap<int32, FVMSplitRect> PlayerRects;

	int32 ActiveKeyboardMouseLP = 0;

	int32 FocusedPlayerIndex = 0;

	bool bClickToFocusEnabled = true;

	bool bFocusHighlightingEnabled = false;

	TSubclassOf<APawn> ResolvePawnClass(const FVMSplitPane& Pane) const;

	void ConfigurePawnForPane(APawn* Pawn, const FVMSplitPane& Pane, APlayerController* PlayerController);

	UPROPERTY(Transient)
	UUserWidget* HUDRootWidget = nullptr;

	UPROPERTY(Transient)
	class UCanvasPanel* HUDRootCanvas = nullptr;

	UPROPERTY(Transient)
	TMap<int32, TWeakObjectPtr<UUserWidget>> ActivePaneHUDs;

	void EnsureHUDRoot();
	void ClearPaneHUDs();
	void EnsureLocalPlayersExist();
	void SpawnAndPossessPawns();
	void SetupViewportHUDs();
	void HandleClickToFocus(const FVector2D& ScreenPosition);
	int32 FindPaneAtScreenPosition(const FVector2D& ScreenPosition) const;
	void EnsureCursorVisibility();
};


