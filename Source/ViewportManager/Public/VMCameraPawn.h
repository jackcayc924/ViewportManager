#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "VMCameraPawn.generated.h"

class UInputMappingContext;
class UInputAction;
struct FVMSplitRect;
struct FVMCameraControlSettings;

UCLASS(BlueprintType, Blueprintable)
class VIEWPORTMANAGER_API AVMCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AVMCameraPawn();

	// Enhanced Input assets (optional - will use legacy input as fallback)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Look = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Zoom = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_OrbitButton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_PanButton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_LeftMouseButton = nullptr;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

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
	bool bKeepMouseCursorVisible = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float OrbitSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float OrbitYawSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float OrbitPitchSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings", meta = (ClampMin = "10.0", ClampMax = "1000.0"))
	float PanSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings", meta = (ClampMin = "1.0", ClampMax = "50.0"))
	float ZoomSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	bool bInvertOrbitYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	bool bInvertOrbitPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	bool bInvertZoom = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	bool bLockPanHorizontal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Settings")
	bool bLockPanVertical = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projection Settings")
	bool bUseOrthographic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projection Settings", meta = (EditCondition = "!bUseOrthographic", ClampMin = "5.0", ClampMax = "170.0"))
	float MinFOV = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projection Settings", meta = (EditCondition = "!bUseOrthographic", ClampMin = "5.0", ClampMax = "170.0"))
	float MaxFOV = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projection Settings", meta = (EditCondition = "bUseOrthographic", ClampMin = "10.0"))
	float OrthoWidth = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projection Settings", meta = (EditCondition = "bUseOrthographic", ClampMin = "10.0"))
	float MinOrthoWidth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projection Settings", meta = (EditCondition = "bUseOrthographic", ClampMin = "10.0"))
	float MaxOrthoWidth = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Settings")
	FVector FocusPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Settings")
	TObjectPtr<AActor> TargetActor = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Settings", meta = (ClampMin = "10.0"))
	float StartingOrbitDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Settings", meta = (ClampMin = "1.0"))
	float MinOrbitDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit Settings", meta = (ClampMin = "1.0"))
	float MaxOrbitDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings")
	bool bDisableViewportFocusing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings")
	bool bUseDistanceBasedZoom = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings", meta = (EditCondition = "bUseDistanceBasedZoom", ClampMin = "1.0"))
	float ZoomDistanceMultiplier = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings")
	bool bUseSmoothMovement = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Settings", meta = (EditCondition = "bUseSmoothMovement", ClampMin = "0.1", ClampMax = "20.0"))
	float SmoothMovementSpeed = 5.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Advanced Settings")
	bool bUseDirectCameraPositioning = false;
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void ZoomStep(float WheelDelta);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void OrbitDelta(float DeltaYawDeg, float DeltaPitchDeg);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void PanDelta(float DeltaRight, float DeltaUp);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetFocusPoint(FVector World);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetTargetActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	AActor* GetTargetActor() const { return TargetActor; }

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetOrbitDistance(float Distance);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	float GetOrbitDistance() const;

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void ZoomToDistance(float Distance);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void FocusOnActor(AActor* Actor, float Distance = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void ResetToStartingPosition();

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetCameraMode(bool bOrthographic);

	// Public setters for starting position (needed by VMGameViewportClient)
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetStartingPosition(FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	FVector GetStartingLocation() const { return StartingLocation; }

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	FRotator GetStartingRotation() const { return StartingRotation; }

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void EnableDirectCameraPositioning(bool bEnable = true);
	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetCameraControlsEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	bool GetCameraControlsEnabled() const { return bCameraControlsEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetOrbitEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	bool GetOrbitEnabled() const { return bOrbitEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetPanEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	bool GetPanEnabled() const { return bPanEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetZoomEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	bool GetZoomEnabled() const { return bZoomEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetTargetActorTrackingEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	bool GetTargetActorTrackingEnabled() const { return bTargetActorTrackingEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void ToggleCameraControls();

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void ToggleOrbit();

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void TogglePan();

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void ToggleZoom();

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void ToggleTargetActorTracking();

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	void SetKeepMouseCursorVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Camera Control")
	bool GetKeepMouseCursorVisible() const { return bKeepMouseCursorVisible; }

	void ApplyControlSettings(const FVMCameraControlSettings& Settings);

protected:
	// Legacy input handling (fallback)
	void OnOrbitYaw(float Value);
	void OnOrbitPitch(float Value);
	void OnPanRight(float Value);
	void OnPanUp(float Value);
	void OnZoom(float Value);
	void OnRightMousePressed();
	void OnRightMouseReleased();
	void OnMiddleMousePressed();
	void OnMiddleMouseReleased();
	void OnLeftMousePressed();

	// Enhanced Input handling
	void OnLookInput(const FInputActionValue& Value);
	void OnZoomInput(const FInputActionValue& Value);
	void OnOrbitButtonInput(const FInputActionValue& Value);
	void OnPanButtonInput(const FInputActionValue& Value);
	void OnLeftMouseButtonInput(const FInputActionValue& Value);

	bool bIsOrbiting = false;
	bool bIsPanning = false;
	FVector2D LastMousePosition;

	float DefaultOrbitSensitivity = 1.0f;
	float DefaultPanSpeed = 100.0f;
	float DefaultZoomSpeed = 10.0f;

	FVector TargetLocation;
	FRotator TargetRotation;
	float TargetOrbitDistance;
	bool bIsMovingToTarget = false;

	FVector StartingLocation;
	FRotator StartingRotation;
	void UpdateCameraProjection();
	void UpdateTargetTracking();
	void UpdateSmoothMovement(float DeltaTime);
	FVector GetCameraRightVector() const;
	FVector GetCameraUpVector() const;
	FVector GetCurrentFocusPoint() const;
	void SetupStartingPosition();
};


