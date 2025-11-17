#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "VMSplitLayoutAsset.generated.h"

UENUM(BlueprintType)
enum class EVMViewportCameraMode : uint8
{
	Orbit	UMETA(DisplayName = "Orbit Camera"),
	Free	UMETA(DisplayName = "Free Camera"),
	Custom	UMETA(DisplayName = "Custom Pawn")
};

USTRUCT(BlueprintType)
struct VIEWPORTMANAGER_API FVMCameraControlSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls")
	bool bCameraControlsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls", meta = (EditCondition = "bCameraControlsEnabled"))
	bool bOrbitEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls", meta = (EditCondition = "bCameraControlsEnabled"))
	bool bPanEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls", meta = (EditCondition = "bCameraControlsEnabled"))
	bool bZoomEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls", meta = (EditCondition = "bCameraControlsEnabled"))
	bool bTargetActorTrackingEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls")
	TSoftObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls")
	bool bKeepMouseCursorVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Input", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float OrbitYawSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Input", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float OrbitPitchSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Input", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float PanSpeedScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Input", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float ZoomSpeedScalar = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Input")
	bool bInvertOrbitYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Input")
	bool bInvertOrbitPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Input")
	bool bInvertZoom = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Input")
	bool bLockPanHorizontal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Input")
	bool bLockPanVertical = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Free Camera", meta = (ClampMin = "10.0", ClampMax = "5000.0"))
	float FreeMoveSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Free Camera", meta = (ClampMin = "10.0", ClampMax = "10000.0"))
	float FreeSprintSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls|Free Camera", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float FreeLookSensitivity = 1.0f;
};

USTRUCT(BlueprintType)
struct VIEWPORTMANAGER_API FVMSplitRect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	FVector2f Origin01 = FVector2f(0.f, 0.f); // [0..1] top-left

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", meta = (ClampMin = "0.01", ClampMax = "1.0", UIMin = "0.01", UIMax = "1.0"))
	FVector2f Size01 = FVector2f(1.f, 1.f); // [0..1] width/height
};

USTRUCT(BlueprintType)
struct VIEWPORTMANAGER_API FVMSplitPane
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pane")
	int32 LocalPlayerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pane")
	FVMSplitRect Rect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pane", meta = (ClampMin = "0", ClampMax = "100", UIMin = "0", UIMax = "100", ToolTip = "Z-order depth - higher values render on top of lower values"))
	int32 Depth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pane")
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pane", meta = (AllowedClasses = "/Script/UMGEditor.WidgetBlueprint, /Script/UMG.UserWidget", ToolTip = "Optional HUD widget class to display overlay UI for this viewport"))
	TSubclassOf<UUserWidget> ViewportHUDClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pane")
	bool bReceivesKeyboardMouse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	EVMViewportCameraMode CameraMode = EVMViewportCameraMode::Orbit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "CameraMode == EVMViewportCameraMode::Custom"))
	TSubclassOf<APawn> CustomPawnClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "bUseCustomCameraTransform"))
	FTransform CameraTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 100.0f), FVector::OneVector);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bUseCustomCameraTransform = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "bUseCustomFocusPoint"))
	FVector FocusPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bUseCustomFocusPoint = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "bUseCustomFocusPoint"))
	float OrbitDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Controls")
	FVMCameraControlSettings CameraControls;
};

UCLASS(BlueprintType, Blueprintable)
class VIEWPORTMANAGER_API UVMSplitLayoutAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TArray<FVMSplitPane> Panes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	bool bAutoSpawnPlayers = true;

	UFUNCTION(CallInEditor, Category = "Layout")
	void ValidateLayout();

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};


