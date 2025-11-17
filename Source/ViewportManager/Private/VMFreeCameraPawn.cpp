#include "VMFreeCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "VMSplitLayoutAsset.h"
#include "VMLog.h"

AVMFreeCameraPawn::AVMFreeCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bUsePawnControlRotation = true;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;
	MovementComponent->MaxSpeed = DefaultMoveSpeed;
	MovementComponent->Acceleration = 2048.f;
	MovementComponent->Deceleration = 2048.f;
}

void AVMFreeCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

void AVMFreeCameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

UPawnMovementComponent* AVMFreeCameraPawn::GetMovementComponent() const
{
	return MovementComponent;
}

void AVMFreeCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Try to use Enhanced Input if we have InputActions assigned
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Check if we have all the required InputActions
		if (IA_Move && IA_Look && IA_Sprint)
		{
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AVMFreeCameraPawn::OnMoveInput);
			EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AVMFreeCameraPawn::OnLookInput);
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AVMFreeCameraPawn::OnSprintInput);
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AVMFreeCameraPawn::OnSprintInput);
			UE_LOG(LogViewportManager, Log, TEXT("AVMFreeCameraPawn: Using Enhanced Input bindings"));
			return;
		}
		else
		{
			UE_LOG(LogViewportManager, Warning, TEXT("AVMFreeCameraPawn: Enhanced Input available but InputActions not assigned, falling back to legacy input"));
		}
	}

	// Fallback to legacy input
	PlayerInputComponent->BindAxis("MoveForward", this, &AVMFreeCameraPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVMFreeCameraPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &AVMFreeCameraPawn::MoveUp);
	PlayerInputComponent->BindAxis("Turn", this, &AVMFreeCameraPawn::LookYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AVMFreeCameraPawn::LookPitch);
	PlayerInputComponent->BindAction("LeftShift", IE_Pressed, this, &AVMFreeCameraPawn::StartSprint);
	PlayerInputComponent->BindAction("LeftShift", IE_Released, this, &AVMFreeCameraPawn::StopSprint);
	UE_LOG(LogViewportManager, Log, TEXT("AVMFreeCameraPawn: Using legacy input bindings"));
}

void AVMFreeCameraPawn::MoveForward(float Value)
{
	if (FMath::IsNearlyZero(Value) || !MovementComponent)
	{
		return;
	}

	if (!bControlsEnabled || !bAllowTranslation)
	{
		return;
	}

	const FRotator Rotation = GetControlRotation();
	const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, Value);
}

void AVMFreeCameraPawn::MoveRight(float Value)
{
	if (FMath::IsNearlyZero(Value) || !MovementComponent)
	{
		return;
	}

	if (!bControlsEnabled || !bAllowTranslation || bLockHorizontalTranslation)
	{
		return;
	}

	const FRotator Rotation = GetControlRotation();
	const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, Value);
}

void AVMFreeCameraPawn::MoveUp(float Value)
{
	if (FMath::IsNearlyZero(Value) || !MovementComponent)
	{
		return;
	}

	if (!bControlsEnabled || !bAllowTranslation || bLockVerticalTranslation)
	{
		return;
	}

	AddMovementInput(FVector::UpVector, Value);
}

void AVMFreeCameraPawn::LookYaw(float Value)
{
	if (FMath::IsNearlyZero(Value) || !bControlsEnabled || !bAllowLook)
	{
		return;
	}

	AddControllerYawInput(Value * LookSensitivity * (bInvertYaw ? -1.0f : 1.0f));
}

void AVMFreeCameraPawn::LookPitch(float Value)
{
	if (FMath::IsNearlyZero(Value) || !bControlsEnabled || !bAllowLook)
	{
		return;
	}

	AddControllerPitchInput(Value * LookSensitivity * (bInvertPitch ? -1.0f : 1.0f));
}

void AVMFreeCameraPawn::StartSprint()
{
	if (!MovementComponent || !bControlsEnabled || !bAllowTranslation)
	{
		return;
	}

	bIsSprinting = true;
	MovementComponent->MaxSpeed = DefaultSprintSpeed;
}

void AVMFreeCameraPawn::StopSprint()
{
	if (!MovementComponent)
	{
		return;
	}

	bIsSprinting = false;
	MovementComponent->MaxSpeed = DefaultMoveSpeed;
}

void AVMFreeCameraPawn::ApplyControlSettings(const FVMCameraControlSettings& Settings)
{
	bControlsEnabled = Settings.bCameraControlsEnabled;
	bAllowTranslation = bControlsEnabled && Settings.bPanEnabled;
	bAllowLook = bControlsEnabled && Settings.bOrbitEnabled;
	bLockHorizontalTranslation = Settings.bLockPanHorizontal;
	bLockVerticalTranslation = Settings.bLockPanVertical;

	const float MoveSpeed = Settings.FreeMoveSpeed * Settings.PanSpeedScalar;
	const float SprintSpeed = Settings.FreeSprintSpeed * Settings.PanSpeedScalar;

	SetMoveSpeed(MoveSpeed);
	SetSprintSpeed(SprintSpeed);
	SetLookSensitivity(Settings.FreeLookSensitivity);

	bInvertYaw = Settings.bInvertOrbitYaw;
	bInvertPitch = Settings.bInvertOrbitPitch;

	if (MovementComponent && !bAllowTranslation)
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->MaxSpeed = DefaultMoveSpeed;
		bIsSprinting = false;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = Settings.bKeepMouseCursorVisible;
		if (Settings.bKeepMouseCursorVisible)
		{
			PC->SetInputMode(FInputModeGameAndUI());
		}
		else
		{
			PC->SetInputMode(FInputModeGameOnly());
		}
	}
}

void AVMFreeCameraPawn::SetMoveSpeed(float NewSpeed)
{
	DefaultMoveSpeed = FMath::Max(NewSpeed, 10.0f);
	if (!bIsSprinting && MovementComponent)
	{
		MovementComponent->MaxSpeed = DefaultMoveSpeed;
	}
}

void AVMFreeCameraPawn::SetSprintSpeed(float NewSpeed)
{
	DefaultSprintSpeed = FMath::Max(NewSpeed, DefaultMoveSpeed);
	if (bIsSprinting && MovementComponent)
	{
		MovementComponent->MaxSpeed = DefaultSprintSpeed;
	}
}

void AVMFreeCameraPawn::SetLookSensitivity(float NewSensitivity)
{
	LookSensitivity = FMath::Clamp(NewSensitivity, 0.1f, 10.0f);
}

void AVMFreeCameraPawn::OnMoveInput(const FInputActionValue& Value)
{
	if (!bControlsEnabled || !bAllowTranslation || !MovementComponent)
	{
		return;
	}

	const FVector MoveVector = Value.Get<FVector>();

	if (!FMath::IsNearlyZero(MoveVector.X))
	{
		const FRotator Rotation = GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, MoveVector.X);
	}

	if (!FMath::IsNearlyZero(MoveVector.Y) && !bLockHorizontalTranslation)
	{
		const FRotator Rotation = GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, MoveVector.Y);
	}

	if (!FMath::IsNearlyZero(MoveVector.Z) && !bLockVerticalTranslation)
	{
		AddMovementInput(FVector::UpVector, MoveVector.Z);
	}
}

void AVMFreeCameraPawn::OnLookInput(const FInputActionValue& Value)
{
	if (!bControlsEnabled || !bAllowLook)
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (!FMath::IsNearlyZero(LookAxisVector.X))
	{
		AddControllerYawInput(LookAxisVector.X * LookSensitivity * (bInvertYaw ? -1.0f : 1.0f));
	}

	if (!FMath::IsNearlyZero(LookAxisVector.Y))
	{
		AddControllerPitchInput(LookAxisVector.Y * LookSensitivity * (bInvertPitch ? -1.0f : 1.0f));
	}
}

void AVMFreeCameraPawn::OnSprintInput(const FInputActionValue& Value)
{
	if (!bControlsEnabled || !bAllowTranslation || !MovementComponent)
	{
		return;
	}

	const bool bIsPressed = Value.Get<bool>();

	if (bIsPressed)
	{
		bIsSprinting = true;
		MovementComponent->MaxSpeed = DefaultSprintSpeed;
	}
	else
	{
		bIsSprinting = false;
		MovementComponent->MaxSpeed = DefaultMoveSpeed;
	}
}
