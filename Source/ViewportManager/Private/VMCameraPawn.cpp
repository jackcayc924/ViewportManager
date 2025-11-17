#include "VMCameraPawn.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "VMLog.h"
#include "VMSplitLayoutAsset.h"

AVMCameraPawn::AVMCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = StartingOrbitDistance;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = false;
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	FocusPoint = FVector::ZeroVector;
	TargetLocation = FVector::ZeroVector;
	TargetRotation = FRotator::ZeroRotator;
	TargetOrbitDistance = StartingOrbitDistance;

	DefaultOrbitSensitivity = OrbitSensitivity;
	DefaultPanSpeed = PanSpeed;
	DefaultZoomSpeed = ZoomSpeed;
}

void AVMCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	UpdateCameraProjection();

	// CRITICAL: Don't override spawn position or rotation - preserve what was set during spawn
	SetupStartingPosition();

	if (SpringArm)
	{
		SpringArm->TargetArmLength = StartingOrbitDistance;
	}

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::BeginPlay - Camera at %s, Controller rotation: %s, OrbitDistance: %.2f"),
		*GetActorLocation().ToString(), *GetControlRotation().ToString(), StartingOrbitDistance);
}

void AVMCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTargetTracking();
}

void AVMCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_Look && IA_Zoom && IA_OrbitButton && IA_PanButton && IA_LeftMouseButton)
		{
			EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AVMCameraPawn::OnLookInput);
			EnhancedInputComponent->BindAction(IA_Zoom, ETriggerEvent::Triggered, this, &AVMCameraPawn::OnZoomInput);
			EnhancedInputComponent->BindAction(IA_OrbitButton, ETriggerEvent::Started, this, &AVMCameraPawn::OnOrbitButtonInput);
			EnhancedInputComponent->BindAction(IA_OrbitButton, ETriggerEvent::Completed, this, &AVMCameraPawn::OnOrbitButtonInput);
			EnhancedInputComponent->BindAction(IA_PanButton, ETriggerEvent::Started, this, &AVMCameraPawn::OnPanButtonInput);
			EnhancedInputComponent->BindAction(IA_PanButton, ETriggerEvent::Completed, this, &AVMCameraPawn::OnPanButtonInput);
			EnhancedInputComponent->BindAction(IA_LeftMouseButton, ETriggerEvent::Started, this, &AVMCameraPawn::OnLeftMouseButtonInput);

			UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn: Using Enhanced Input bindings"));
			return;
		}
		else
		{
			UE_LOG(LogViewportManager, Warning, TEXT("AVMCameraPawn: Enhanced Input available but InputActions not assigned, falling back to legacy input"));
		}
	}

	PlayerInputComponent->BindAxis("Turn", this, &AVMCameraPawn::OnOrbitYaw);
	PlayerInputComponent->BindAxis("LookUp", this, &AVMCameraPawn::OnOrbitPitch);
	PlayerInputComponent->BindAxis("MouseWheelAxis", this, &AVMCameraPawn::OnZoom);
	PlayerInputComponent->BindAction("RightMouseButton", IE_Pressed, this, &AVMCameraPawn::OnRightMousePressed);
	PlayerInputComponent->BindAction("RightMouseButton", IE_Released, this, &AVMCameraPawn::OnRightMouseReleased);
	PlayerInputComponent->BindAction("MiddleMouseButton", IE_Pressed, this, &AVMCameraPawn::OnMiddleMousePressed);
	PlayerInputComponent->BindAction("MiddleMouseButton", IE_Released, this, &AVMCameraPawn::OnMiddleMouseReleased);
	PlayerInputComponent->BindAction("LeftMouseButton", IE_Pressed, this, &AVMCameraPawn::OnLeftMousePressed);

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn: Using legacy input bindings"));
}

void AVMCameraPawn::ZoomStep(float WheelDelta)
{
	const float EffectiveDelta = bInvertZoom ? -WheelDelta : WheelDelta;

	if (bUseDistanceBasedZoom && SpringArm)
	{
		float ZoomAmount = EffectiveDelta * ZoomDistanceMultiplier;
		float NewDistance = SpringArm->TargetArmLength - ZoomAmount;
		SetOrbitDistance(NewDistance);

		if (TargetActor && IsValid(TargetActor))
		{
			FVector ActorLocation = TargetActor->GetActorLocation();
			if (!FocusPoint.Equals(ActorLocation, 1.0f))
			{
				FocusPoint = ActorLocation;
				SetActorLocation(ActorLocation);
			}
		}
	}
	else if (bUseOrthographic)
	{
		float NewOrthoWidth = OrthoWidth - (EffectiveDelta * ZoomSpeed * 10.0f);
		OrthoWidth = FMath::Clamp(NewOrthoWidth, MinOrthoWidth, MaxOrthoWidth);
		Camera->SetOrthoWidth(OrthoWidth);
	}
	else
	{
		float NewFOV = Camera->FieldOfView - (EffectiveDelta * ZoomSpeed);
		Camera->SetFieldOfView(FMath::Clamp(NewFOV, MinFOV, MaxFOV));
	}
}

void AVMCameraPawn::OrbitDelta(float DeltaYawDeg, float DeltaPitchDeg)
{
	FRotator CurrentRotation = GetControlRotation();
	FRotator NewRotation = CurrentRotation + FRotator(DeltaPitchDeg, DeltaYawDeg, 0.0f);

	// Clamp pitch to prevent flipping
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch, -89.0f, 89.0f);

	GetController()->SetControlRotation(NewRotation);

	if (TargetActor && IsValid(TargetActor))
	{
		FVector ActorLocation = TargetActor->GetActorLocation();
		if (!FocusPoint.Equals(ActorLocation, 1.0f))
		{
			FocusPoint = ActorLocation;
			SetActorLocation(ActorLocation);
		}
	}
}

void AVMCameraPawn::PanDelta(float DeltaRight, float DeltaUp)
{
	if (bLockPanHorizontal)
	{
		DeltaRight = 0.0f;
	}

	if (bLockPanVertical)
	{
		DeltaUp = 0.0f;
	}

	FVector RightVector = GetCameraRightVector();
	FVector UpVector = GetCameraUpVector();

	FVector PanMovement = (RightVector * DeltaRight) + (UpVector * DeltaUp);

	if (TargetActor && IsValid(TargetActor))
	{
		FVector NewLocation = GetActorLocation() + PanMovement;
		SetActorLocation(NewLocation);
	}
	else
	{
		FVector NewLocation = GetActorLocation() + PanMovement;
		SetActorLocation(NewLocation);
		FocusPoint += PanMovement;
	}
}

void AVMCameraPawn::SetFocusPoint(FVector World)
{
	FocusPoint = World;
	SetActorLocation(World);
	
	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetFocusPoint - Set focus point to %s"), *World.ToString());
}

void AVMCameraPawn::SetOrbitDistance(float Distance)
{
	if (SpringArm)
	{
		float ClampedDistance = FMath::Clamp(Distance, MinOrbitDistance, MaxOrbitDistance);
		SpringArm->TargetArmLength = ClampedDistance;
		
		UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetOrbitDistance - Set orbit distance to %.2f"), ClampedDistance);
	}
}

float AVMCameraPawn::GetOrbitDistance() const
{
	if (SpringArm)
	{
		return SpringArm->TargetArmLength;
	}
	return 500.0f; // Default distance
}

void AVMCameraPawn::OnOrbitYaw(float Value)
{
	if (!bCameraControlsEnabled || FMath::Abs(Value) <= 0.01f) return;

	if (bIsPanning && bPanEnabled)
	{
		PanDelta(Value * PanSpeed * GetWorld()->GetDeltaSeconds(), 0.0f);
	}
	else if (bIsOrbiting && bOrbitEnabled)
	{
		const float Sign = bInvertOrbitYaw ? -1.0f : 1.0f;
		const float Delta = Value * OrbitSensitivity * OrbitYawSensitivity * Sign;
		OrbitDelta(Delta, 0.0f);
	}
}

void AVMCameraPawn::OnOrbitPitch(float Value)
{
	if (!bCameraControlsEnabled || FMath::Abs(Value) <= 0.01f) return;

	if (bIsPanning && bPanEnabled)
	{
		PanDelta(0.0f, Value * PanSpeed * GetWorld()->GetDeltaSeconds());
	}
	else if (bIsOrbiting && bOrbitEnabled)
	{
		const float Sign = bInvertOrbitPitch ? 1.0f : -1.0f;
		const float Delta = Value * OrbitSensitivity * OrbitPitchSensitivity * Sign;
		OrbitDelta(0.0f, Delta);
	}
}

void AVMCameraPawn::OnZoom(float Value)
{
	if (bCameraControlsEnabled && bZoomEnabled && FMath::Abs(Value) > 0.01f)
	{
		ZoomStep(Value);
	}
}

void AVMCameraPawn::OnRightMousePressed()
{
	if (bCameraControlsEnabled && bOrbitEnabled)
	{
		bIsOrbiting = true;

		if (!bKeepMouseCursorVisible)
		{
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->bShowMouseCursor = false;
				PC->SetInputMode(FInputModeGameOnly());
			}
		}
	}
}

void AVMCameraPawn::OnRightMouseReleased()
{
	bIsOrbiting = false;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

void AVMCameraPawn::OnMiddleMousePressed()
{
	if (bCameraControlsEnabled && bPanEnabled)
	{
		bIsPanning = true;

		if (!bKeepMouseCursorVisible)
		{
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->bShowMouseCursor = false;
				PC->SetInputMode(FInputModeGameOnly());
			}
		}
	}
}

void AVMCameraPawn::OnMiddleMouseReleased()
{
	bIsPanning = false;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

void AVMCameraPawn::OnLeftMousePressed()
{
	if (!bDisableViewportFocusing)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FVector WorldLocation, WorldDirection;
			if (PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
			{
				FHitResult HitResult;
				FVector TraceEnd = WorldLocation + (WorldDirection * 10000.0f);

				if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ECC_Visibility))
				{
					if (!TargetActor || !IsValid(TargetActor))
					{
						SetFocusPoint(HitResult.Location);
						UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn: Set focus point to %s"), *HitResult.Location.ToString());
					}
					else
					{
						UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn: Left click ignored - target actor is set"));
					}
				}
			}
		}
	}
}

void AVMCameraPawn::UpdateCameraProjection()
{
	if (Camera)
	{
		if (bUseOrthographic)
		{
			Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
			Camera->SetOrthoWidth(OrthoWidth);
		}
		else
		{
			Camera->SetProjectionMode(ECameraProjectionMode::Perspective);
			Camera->SetFieldOfView(FMath::Clamp(Camera->FieldOfView, MinFOV, MaxFOV));
		}
	}
}

FVector AVMCameraPawn::GetCameraRightVector() const
{
	if (Camera)
	{
		return Camera->GetRightVector();
	}
	return GetActorRightVector();
}

FVector AVMCameraPawn::GetCameraUpVector() const
{
	if (Camera)
	{
		return Camera->GetUpVector();
	}
	return GetActorUpVector();
}

void AVMCameraPawn::SetTargetActor(AActor* Actor)
{
	TargetActor = Actor;

	if (TargetActor)
	{
		FVector ActorLocation = TargetActor->GetActorLocation();
		FocusPoint = ActorLocation;

		SetActorLocation(ActorLocation);

		if (SpringArm && SpringArm->TargetArmLength < MinOrbitDistance)
		{
			SpringArm->TargetArmLength = FMath::Max(MinOrbitDistance, 300.0f);
		}

		UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetTargetActor - Set target actor: %s at location %s, orbit distance: %.2f"),
			*TargetActor->GetName(), *ActorLocation.ToString(), SpringArm ? SpringArm->TargetArmLength : 0.0f);
	}
	else
	{
		UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetTargetActor - Cleared target actor"));
	}
}

void AVMCameraPawn::ZoomToDistance(float Distance)
{
	SetOrbitDistance(Distance);
}

void AVMCameraPawn::FocusOnActor(AActor* Actor, float Distance)
{
	if (Actor)
	{
		TargetActor = Actor;
		FVector ActorLocation = Actor->GetActorLocation();

		FocusPoint = ActorLocation;
		SetActorLocation(ActorLocation);

		float FinalDistance = Distance > 0.0f ? Distance : 500.0f;
		SetOrbitDistance(FinalDistance);

		FVector CameraOffset = FVector(-FinalDistance, 0.0f, FinalDistance * 0.3f);
		FRotator LookAtRotation = (ActorLocation - (ActorLocation + CameraOffset)).Rotation();

		if (AController* PawnController = GetController())
		{
			PawnController->SetControlRotation(LookAtRotation);
		}

		UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::FocusOnActor - Focused on %s at location %s, distance %.2f, rotation %s"),
			*Actor->GetName(), *ActorLocation.ToString(), FinalDistance, *LookAtRotation.ToString());
	}
}

void AVMCameraPawn::ResetToStartingPosition()
{
	SetActorLocation(StartingLocation);
	GetController()->SetControlRotation(StartingRotation);
	if (SpringArm)
	{
		SpringArm->TargetArmLength = StartingOrbitDistance;
	}

	FocusPoint = StartingLocation;
	TargetActor = nullptr;

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::ResetToStartingPosition - Reset camera to starting position"));
}

void AVMCameraPawn::SetCameraMode(bool bOrthographic)
{
	bUseOrthographic = bOrthographic;
	UpdateCameraProjection();

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetCameraMode - Set camera mode to %s"),
		bOrthographic ? TEXT("Orthographic") : TEXT("Perspective"));
}

void AVMCameraPawn::UpdateTargetTracking()
{
	if (bCameraControlsEnabled && bTargetActorTrackingEnabled && TargetActor && IsValid(TargetActor))
	{
		FVector ActorLocation = TargetActor->GetActorLocation();
		if (!FocusPoint.Equals(ActorLocation, 1.0f))
		{
			SetFocusPoint(ActorLocation);
		}
	}
}

FVector AVMCameraPawn::GetCurrentFocusPoint() const
{
	if (TargetActor && IsValid(TargetActor))
	{
		return TargetActor->GetActorLocation();
	}
	return FocusPoint;
}

void AVMCameraPawn::SetStartingPosition(FVector Location, FRotator Rotation)
{
	StartingLocation = Location;
	StartingRotation = Rotation;

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetStartingPosition - Set starting position: %s, rotation: %s"),
		*StartingLocation.ToString(), *StartingRotation.ToString());
}

void AVMCameraPawn::SetupStartingPosition()
{
	StartingLocation = GetActorLocation();
	StartingRotation = GetControlRotation();

	if (StartingLocation.IsZero())
	{
		StartingLocation = FocusPoint;
	}

	if (StartingRotation.IsZero())
	{
		StartingRotation = GetActorRotation();
	}

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetupStartingPosition - Starting position: %s, rotation: %s"),
		*StartingLocation.ToString(), *StartingRotation.ToString());
}

void AVMCameraPawn::EnableDirectCameraPositioning(bool bEnable)
{
	bUseDirectCameraPositioning = bEnable;
	
	if (bEnable && Camera && SpringArm)
	{
		Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Camera->SetupAttachment(RootComponent);
		Camera->bUsePawnControlRotation = true;

		UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::EnableDirectCameraPositioning - ENABLED direct positioning, controller rotation: %s"),
			*GetControlRotation().ToString());
	}
	else if (!bEnable && Camera && SpringArm)
	{
		Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
		Camera->bUsePawnControlRotation = false;

		UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::EnableDirectCameraPositioning - DISABLED direct positioning, using spring arm"));
	}
}

void AVMCameraPawn::SetCameraControlsEnabled(bool bEnabled)
{
	bCameraControlsEnabled = bEnabled;

	if (!bEnabled)
	{
		bIsOrbiting = false;
		bIsPanning = false;

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeGameAndUI());
		}
	}

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetCameraControlsEnabled - Camera controls %s"),
		bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AVMCameraPawn::SetOrbitEnabled(bool bEnabled)
{
	bOrbitEnabled = bEnabled;

	if (!bEnabled && bIsOrbiting)
	{
		bIsOrbiting = false;

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeGameAndUI());
		}
	}

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetOrbitEnabled - Orbit controls %s"),
		bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AVMCameraPawn::SetPanEnabled(bool bEnabled)
{
	bPanEnabled = bEnabled;

	if (!bEnabled && bIsPanning)
	{
		bIsPanning = false;

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeGameAndUI());
		}
	}

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetPanEnabled - Pan controls %s"),
		bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AVMCameraPawn::SetZoomEnabled(bool bEnabled)
{
	bZoomEnabled = bEnabled;

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetZoomEnabled - Zoom controls %s"),
		bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AVMCameraPawn::SetTargetActorTrackingEnabled(bool bEnabled)
{
	bTargetActorTrackingEnabled = bEnabled;

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetTargetActorTrackingEnabled - Target actor tracking %s"),
		bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AVMCameraPawn::SetKeepMouseCursorVisible(bool bVisible)
{
	bKeepMouseCursorVisible = bVisible;

	if (bIsOrbiting || bIsPanning)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (bVisible)
			{
				PC->bShowMouseCursor = true;
				PC->SetInputMode(FInputModeGameAndUI());
			}
			else
			{
				PC->bShowMouseCursor = false;
				PC->SetInputMode(FInputModeGameOnly());
			}
		}
	}

	UE_LOG(LogViewportManager, Log, TEXT("AVMCameraPawn::SetKeepMouseCursorVisible - Mouse cursor visibility %s"),
		bVisible ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AVMCameraPawn::ToggleCameraControls()
{
	SetCameraControlsEnabled(!bCameraControlsEnabled);
}

void AVMCameraPawn::ToggleOrbit()
{
	SetOrbitEnabled(!bOrbitEnabled);
}

void AVMCameraPawn::TogglePan()
{
	SetPanEnabled(!bPanEnabled);
}

void AVMCameraPawn::ToggleZoom()
{
	SetZoomEnabled(!bZoomEnabled);
}

void AVMCameraPawn::ToggleTargetActorTracking()
{
	SetTargetActorTrackingEnabled(!bTargetActorTrackingEnabled);
}

void AVMCameraPawn::ApplyControlSettings(const FVMCameraControlSettings& Settings)
{
	SetCameraControlsEnabled(Settings.bCameraControlsEnabled);
	SetOrbitEnabled(Settings.bOrbitEnabled);
	SetPanEnabled(Settings.bPanEnabled);
	SetZoomEnabled(Settings.bZoomEnabled);
	SetTargetActorTrackingEnabled(Settings.bTargetActorTrackingEnabled);
	SetKeepMouseCursorVisible(Settings.bKeepMouseCursorVisible);

	OrbitYawSensitivity = Settings.OrbitYawSensitivity;
	OrbitPitchSensitivity = Settings.OrbitPitchSensitivity;
	PanSpeed = FMath::Max(DefaultPanSpeed * Settings.PanSpeedScalar, 1.0f);
	ZoomSpeed = FMath::Max(DefaultZoomSpeed * Settings.ZoomSpeedScalar, 0.1f);

	bInvertOrbitYaw = Settings.bInvertOrbitYaw;
	bInvertOrbitPitch = Settings.bInvertOrbitPitch;
	bInvertZoom = Settings.bInvertZoom;
	bLockPanHorizontal = Settings.bLockPanHorizontal;
	bLockPanVertical = Settings.bLockPanVertical;

	if (Settings.TargetActor.IsValid())
	{
		SetTargetActor(Settings.TargetActor.Get());
	}
	else if (Settings.TargetActor.ToSoftObjectPath().IsValid())
	{
		if (AActor* LoadedActor = Settings.TargetActor.LoadSynchronous())
		{
			SetTargetActor(LoadedActor);
		}
	}
	else
	{
		SetTargetActor(nullptr);
	}
}

void AVMCameraPawn::OnLookInput(const FInputActionValue& Value)
{
	if (!bCameraControlsEnabled) return;

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (bIsPanning && bPanEnabled)
	{
		PanDelta(LookAxisVector.X * PanSpeed * GetWorld()->GetDeltaSeconds(),
		         -LookAxisVector.Y * PanSpeed * GetWorld()->GetDeltaSeconds());
	}
	else if (bIsOrbiting && bOrbitEnabled)
	{
		const float YawSign = bInvertOrbitYaw ? -1.0f : 1.0f;
		const float PitchSign = bInvertOrbitPitch ? 1.0f : -1.0f;

		const float DeltaYaw = LookAxisVector.X * OrbitSensitivity * OrbitYawSensitivity * YawSign;
		const float DeltaPitch = LookAxisVector.Y * OrbitSensitivity * OrbitPitchSensitivity * PitchSign;

		OrbitDelta(DeltaYaw, DeltaPitch);
	}
}

void AVMCameraPawn::OnZoomInput(const FInputActionValue& Value)
{
	if (bCameraControlsEnabled && bZoomEnabled)
	{
		const float ZoomValue = Value.Get<float>();
		if (FMath::Abs(ZoomValue) > 0.01f)
		{
			ZoomStep(ZoomValue);
		}
	}
}

void AVMCameraPawn::OnOrbitButtonInput(const FInputActionValue& Value)
{
	if (!bCameraControlsEnabled || !bOrbitEnabled) return;

	const bool bIsPressed = Value.Get<bool>();

	if (bIsPressed)
	{
		bIsOrbiting = true;

		if (!bKeepMouseCursorVisible)
		{
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->bShowMouseCursor = false;
				PC->SetInputMode(FInputModeGameOnly());
			}
		}
	}
	else
	{
		bIsOrbiting = false;

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeGameAndUI());
		}
	}
}

void AVMCameraPawn::OnPanButtonInput(const FInputActionValue& Value)
{
	if (!bCameraControlsEnabled || !bPanEnabled) return;

	const bool bIsPressed = Value.Get<bool>();

	if (bIsPressed)
	{
		bIsPanning = true;

		if (!bKeepMouseCursorVisible)
		{
			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				PC->bShowMouseCursor = false;
				PC->SetInputMode(FInputModeGameOnly());
			}
		}
	}
	else
	{
		bIsPanning = false;

		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->bShowMouseCursor = true;
			PC->SetInputMode(FInputModeGameAndUI());
		}
	}
}

void AVMCameraPawn::OnLeftMouseButtonInput(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		OnLeftMousePressed();
	}
}



