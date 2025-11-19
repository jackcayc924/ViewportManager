// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VMSplitLayoutAsset.h"
#include "VMSplitBlueprintLibrary.generated.h"

/**
 * Blueprint function library for ViewportManager
 * Provides easy access to splitscreen functionality from Blueprints
 */
UCLASS()
class VIEWPORTMANAGER_API UVMSplitBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Apply a custom layout asset to the current game
	 * @param LayoutAsset - The layout asset defining viewport configuration
	 * @return True if layout was applied successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Layout",
		meta = (DisplayName = "Apply Layout Asset",
			Keywords = "apply layout asset viewport",
			ToolTip = "Apply a pre-configured layout asset to set up viewports. Create layout assets in the Content Browser using the ViewportManager plugin."))
	static bool ApplyLayout(UVMSplitLayoutAsset* LayoutAsset);

	/**
	 * Create a simple 2-player horizontal split layout (left/right)
	 * @param HUDClass - Optional HUD widget class to display in each viewport
	 * @return True if layout was applied successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Layout",
		meta = (DisplayName = "Apply 2-Player Horizontal Split",
			Keywords = "2 player horizontal split left right",
			ToolTip = "Quick setup for 2-player side-by-side split screen (Player 0 on left, Player 1 on right)."))
	static bool ApplySimple2PlayerLayout(TSubclassOf<UUserWidget> HUDClass = nullptr);

	/**
	 * Create a simple 4-player grid layout (2x2)
	 * @param HUDClass - Optional HUD widget class to display in each viewport
	 * @return True if layout was applied successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Layout",
		meta = (DisplayName = "Apply 4-Player Grid",
			Keywords = "4 player grid quad 2x2",
			ToolTip = "Quick setup for 4-player grid split screen (2x2 layout)."))
	static bool ApplySimple4PlayerLayout(TSubclassOf<UUserWidget> HUDClass = nullptr);

	/**
	 * Get the ViewportManager subsystem
	 * @return The VM split subsystem, or nullptr if not available
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Advanced", BlueprintPure,
		meta = (DisplayName = "Get Viewport Manager Subsystem",
			Keywords = "subsystem viewport manager get",
			ToolTip = "Get the ViewportManager subsystem for advanced operations. Most users should use the simplified Quick Setup functions instead."))
	static class UVMSplitSubsystem* GetVMSplitSubsystem();

	/**
	 * Check if ViewportManager is currently active
	 * @return True if ViewportManager viewport client is active
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Helpers", BlueprintPure,
		meta = (DisplayName = "Is Viewport Manager Active",
			Keywords = "active viewport manager check",
			ToolTip = "Returns true if the ViewportManager system is currently active and managing viewports."))
	static bool IsVMGameViewportClientActive();

	/**
	 * Set which player receives keyboard and mouse input
	 * @param LocalPlayerIndex - The player index to receive keyboard/mouse input
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Input",
		meta = (DisplayName = "Set Active Input Player",
			Keywords = "input keyboard mouse active player",
			ToolTip = "Set which player receives keyboard and mouse input. By default, input goes to the viewport under the mouse cursor."))
	static void SetActiveKeyboardMousePlayer(int32 LocalPlayerIndex);

	/**
	 * Get which player is currently receiving keyboard and mouse input
	 * @return The active keyboard/mouse player index
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Input", BlueprintPure,
		meta = (DisplayName = "Get Active Input Player",
			Keywords = "input keyboard mouse active player get",
			ToolTip = "Returns which player index is currently receiving keyboard and mouse input."))
	static int32 GetActiveKeyboardMousePlayer();

	// Camera control toggle functions
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void SetCameraControlsEnabled(int32 LocalPlayerIndex, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls", BlueprintPure)
	static bool GetCameraControlsEnabled(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void ToggleCameraControls(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void SetOrbitEnabled(int32 LocalPlayerIndex, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls", BlueprintPure)
	static bool GetOrbitEnabled(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void ToggleOrbit(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void SetPanEnabled(int32 LocalPlayerIndex, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls", BlueprintPure)
	static bool GetPanEnabled(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void TogglePan(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void SetZoomEnabled(int32 LocalPlayerIndex, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls", BlueprintPure)
	static bool GetZoomEnabled(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void ToggleZoom(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void SetTargetActorTrackingEnabled(int32 LocalPlayerIndex, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls", BlueprintPure)
	static bool GetTargetActorTrackingEnabled(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void ToggleTargetActorTracking(int32 LocalPlayerIndex);

	// Camera target actor functions
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void SetTargetActor(int32 LocalPlayerIndex, AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls", BlueprintPure)
	static AActor* GetTargetActor(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls")
	static void FocusOnActor(int32 LocalPlayerIndex, AActor* Actor, float Distance = -1.0f);

	// Helper function to get VMCameraPawn for a local player
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Camera Controls", BlueprintPure)
	static class AVMCameraPawn* GetVMCameraPawn(int32 LocalPlayerIndex);

	// ==================== Simplified Composite Functions ====================
	// These functions combine common operations to reduce Blueprint complexity

	/**
	 * Apply a simple viewport layout with common configurations
	 * @param NumPlayers - Number of players (2 or 4 supported)
	 * @param Orientation - Horizontal or Vertical split (only for 2-player)
	 * @param HUDClass - Optional HUD class to apply to all viewports
	 * @return True if layout was applied successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Quick Setup",
		meta = (DisplayName = "Quick Setup Split Screen",
			Keywords = "viewport layout quick easy simple",
			ToolTip = "Quickly setup a 2 or 4 player viewport layout. Example: QuickSetupSplitScreen(2, Horizontal) creates a left/right split."))
	static bool QuickSetupSplitScreen(int32 NumPlayers = 2, bool bHorizontalSplit = true, TSubclassOf<UUserWidget> HUDClass = nullptr);

	/**
	 * Configure all camera controls for a player at once
	 * @param LocalPlayerIndex - The local player index to configure
	 * @param bEnableOrbit - Enable orbit camera movement
	 * @param bEnablePan - Enable pan camera movement
	 * @param bEnableZoom - Enable zoom camera movement
	 * @param bShowCursor - Show mouse cursor for this player
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Quick Setup",
		meta = (DisplayName = "Configure Camera Controls",
			Keywords = "camera controls setup configure all",
			ToolTip = "Configure all camera controls for a player in one call. Example: ConfigureCameraControls(0, true, true, true, true) enables all controls for player 0."))
	static void ConfigureCameraControls(int32 LocalPlayerIndex, bool bEnableOrbit = true, bool bEnablePan = true, bool bEnableZoom = true, bool bShowCursor = true);

	/**
	 * Setup a camera to focus on an actor with common settings
	 * @param LocalPlayerIndex - The local player index
	 * @param TargetActor - Actor to focus on
	 * @param Distance - Distance from target (use -1 for default)
	 * @param bTrackActor - Whether to continuously track the actor
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Quick Setup",
		meta = (DisplayName = "Focus Camera On Actor",
			Keywords = "camera focus look target actor",
			ToolTip = "Quickly setup a camera to focus on an actor. Example: FocusCameraOnActor(0, MyActor, 500, true) makes player 0's camera orbit MyActor at 500 units distance."))
	static void FocusCameraOnActor(int32 LocalPlayerIndex, AActor* TargetActor, float Distance = -1.0f, bool bTrackActor = true);

	/**
	 * Get the number of active viewports/players
	 * @return Number of local players currently active
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Helpers", BlueprintPure,
		meta = (DisplayName = "Get Active Player Count",
			Keywords = "count players viewports number",
			ToolTip = "Returns the number of active local players/viewports."))
	static int32 GetActivePlayerCount();

	/**
	 * Check if a local player index is valid and active
	 * @param LocalPlayerIndex - Index to check
	 * @return True if the player index is valid and has an active viewport
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Helpers", BlueprintPure,
		meta = (DisplayName = "Is Player Index Valid",
			Keywords = "validate check player index valid",
			ToolTip = "Check if a player index is valid. Returns true if the index exists and has an active viewport."))
	static bool IsPlayerIndexValid(int32 LocalPlayerIndex);

	/**
	 * Get the controller for a specific local player
	 * @param LocalPlayerIndex - The local player index
	 * @return The player controller, or nullptr if not found
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Helpers", BlueprintPure,
		meta = (DisplayName = "Get Player Controller",
			Keywords = "controller player get",
			ToolTip = "Get the PlayerController for a specific local player index."))
	static APlayerController* GetPlayerController(int32 LocalPlayerIndex);

	/**
	 * Enable or disable all camera controls at once
	 * @param LocalPlayerIndex - The local player index
	 * @param bEnabled - True to enable all controls, false to disable all
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Quick Setup",
		meta = (DisplayName = "Enable All Camera Controls",
			Keywords = "camera controls enable disable all",
			ToolTip = "Enable or disable all camera controls (orbit, pan, zoom) at once. Example: EnableAllCameraControls(0, false) disables all camera controls for player 0."))
	static void EnableAllCameraControls(int32 LocalPlayerIndex, bool bEnabled);

	/**
	 * Reset camera to its starting position and settings
	 * @param LocalPlayerIndex - The local player index
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Quick Setup",
		meta = (DisplayName = "Reset Camera",
			Keywords = "camera reset restore default",
			ToolTip = "Reset the camera to its initial position and settings."))
	static void ResetCamera(int32 LocalPlayerIndex);

	// ==================== Focus Management Functions ====================

	/**
	 * Get the currently focused viewport/player
	 * @return The focused player index, or -1 if none
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Focus", BlueprintPure,
		meta = (DisplayName = "Get Focused Player",
			Keywords = "focus player active current get",
			ToolTip = "Returns which player viewport currently has focus for input."))
	static int32 GetFocusedPlayer();

	/**
	 * Set focus to a specific viewport/player
	 * @param LocalPlayerIndex - The player index to focus
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Focus",
		meta = (DisplayName = "Set Focus To Player",
			Keywords = "focus player set active",
			ToolTip = "Set input focus to a specific player viewport."))
	static void SetFocusToPlayer(int32 LocalPlayerIndex);

	/**
	 * Enable or disable click-to-focus behavior
	 * @param bEnabled - True to enable click-to-focus, false to disable
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Focus",
		meta = (DisplayName = "Enable Click To Focus",
			Keywords = "click focus enable disable",
			ToolTip = "Enable or disable clicking on a viewport to give it focus. Enabled by default."))
	static void SetClickToFocusEnabled(bool bEnabled);

	/**
	 * Check if click-to-focus is enabled
	 * @return True if click-to-focus is enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Focus", BlueprintPure,
		meta = (DisplayName = "Is Click To Focus Enabled",
			Keywords = "click focus check enabled",
			ToolTip = "Returns true if clicking on viewports changes focus."))
	static bool IsClickToFocusEnabled();

	/**
	 * Enable or disable focus highlighting
	 * @param bEnabled - True to enable visual highlighting of focused viewport
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Focus",
		meta = (DisplayName = "Enable Focus Highlighting",
			Keywords = "focus highlight border outline",
			ToolTip = "Enable or disable visual highlighting of the focused viewport."))
	static void SetFocusHighlightingEnabled(bool bEnabled);

	/**
	 * Check if focus highlighting is enabled
	 * @return True if focus highlighting is enabled
	 */
	UFUNCTION(BlueprintCallable, Category = "ViewportManager|Focus", BlueprintPure,
		meta = (DisplayName = "Is Focus Highlighting Enabled",
			Keywords = "focus highlight check enabled",
			ToolTip = "Returns true if the focused viewport is visually highlighted."))
	static bool IsFocusHighlightingEnabled();
};


