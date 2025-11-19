// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "VMFreeCameraPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class UInputMappingContext;
class UInputAction;
struct FVMCameraControlSettings;

/**
 * Lightweight free-flying camera pawn for ViewportManager panes.
 * Supports configurable move speeds, sprinting, and look sensitivities.
 */
UCLASS(BlueprintType, Blueprintable)
class VIEWPORTMANAGER_API AVMFreeCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AVMFreeCameraPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Move = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Look = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	TObjectPtr<UInputAction> IA_Sprint = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;
	UFUNCTION(BlueprintCallable, Category = "VM Free Camera")
	void ApplyControlSettings(const FVMCameraControlSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "VM Free Camera")
	void SetMoveSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "VM Free Camera")
	void SetSprintSpeed(float NewSpeed);

	UFUNCTION(BlueprintCallable, Category = "VM Free Camera")
	void SetLookSensitivity(float NewSensitivity);

protected:
	virtual void BeginPlay() override;

	// Legacy input handling (fallback)
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MoveUp(float Value);
	void LookYaw(float Value);
	void LookPitch(float Value);
	void StartSprint();
	void StopSprint();

	// Enhanced Input handling
	void OnMoveInput(const FInputActionValue& Value);
	void OnLookInput(const FInputActionValue& Value);
	void OnSprintInput(const FInputActionValue& Value);

	float DefaultMoveSpeed = 600.0f;
	float DefaultSprintSpeed = 1200.0f;
	float LookSensitivity = 1.0f;

	bool bInvertYaw = false;
	bool bInvertPitch = false;
	bool bIsSprinting = false;
	bool bControlsEnabled = true;
	bool bAllowTranslation = true;
	bool bAllowLook = true;
	bool bLockHorizontalTranslation = false;
	bool bLockVerticalTranslation = false;
};
