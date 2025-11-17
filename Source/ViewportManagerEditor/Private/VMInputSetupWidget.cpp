#include "VMInputSetupWidget.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "PlayerMappableInputConfig.h"
#include "UObject/SavePackage.h"
#include "Misc/MessageDialog.h"
#include "VMLog.h"
#include "VMCameraPawn.h"
#include "VMFreeCameraPawn.h"
#include "EnhancedInputSubsystemInterface.h"

void UVMInputSetupWidget::CreateInputAssets()
{
	const FString InputFolder = TEXT("/ViewportManager/Input");
	const FString PackagePath = TEXT("/Game") + InputFolder;

	TArray<FString> CreatedAssets;
	TArray<FString> FailedAssets;

	// Helper to create InputAction assets
	auto CreateInputAction = [&](const FString& Name, EInputActionValueType ValueType) -> UInputAction*
	{
		const FString AssetName = Name;
		const FString PackageName = PackagePath + TEXT("/") + AssetName;

		// Check if asset already exists
		UPackage* Package = FindPackage(nullptr, *PackageName);
		if (Package)
		{
			// Asset already exists, skip
			UE_LOG(LogViewportManager, Warning, TEXT("InputAction already exists: %s"), *AssetName);
			return nullptr;
		}

		Package = CreatePackage(*PackageName);
		if (!Package)
		{
			FailedAssets.Add(AssetName + TEXT(" (Package creation failed)"));
			return nullptr;
		}

		// Create the InputAction
		UInputAction* InputAction = NewObject<UInputAction>(Package, *AssetName, RF_Public | RF_Standalone);
		if (!InputAction)
		{
			FailedAssets.Add(AssetName + TEXT(" (Object creation failed)"));
			return nullptr;
		}

		InputAction->ValueType = ValueType;

		// Save the asset
		Package->MarkPackageDirty();
		FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		if (UPackage::SavePackage(Package, InputAction, *PackageFileName, SaveArgs))
		{
			CreatedAssets.Add(AssetName);
			UE_LOG(LogViewportManager, Log, TEXT("Created InputAction: %s"), *AssetName);
			return InputAction;
		}
		else
		{
			FailedAssets.Add(AssetName + TEXT(" (Save failed)"));
			return nullptr;
		}
	};

	// Create InputAction assets
	UE_LOG(LogViewportManager, Log, TEXT("=== Starting Enhanced Input Setup ==="));

	UInputAction* IA_Look = CreateInputAction(TEXT("IA_Look"), EInputActionValueType::Axis2D);
	UInputAction* IA_Zoom = CreateInputAction(TEXT("IA_Zoom"), EInputActionValueType::Axis1D);
	UInputAction* IA_OrbitButton = CreateInputAction(TEXT("IA_OrbitButton"), EInputActionValueType::Boolean);
	UInputAction* IA_PanButton = CreateInputAction(TEXT("IA_PanButton"), EInputActionValueType::Boolean);
	UInputAction* IA_LeftMouseButton = CreateInputAction(TEXT("IA_LeftMouseButton"), EInputActionValueType::Boolean);
	UInputAction* IA_Move = CreateInputAction(TEXT("IA_Move"), EInputActionValueType::Axis3D);
	UInputAction* IA_Sprint = CreateInputAction(TEXT("IA_Sprint"), EInputActionValueType::Boolean);

	// Configure IMC_ViewportCamera with mappings
	TArray<FString> ConfiguredSteps;
	UInputMappingContext* IMC = nullptr;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	// Try to find existing IMC
	FAssetData IMCAsset = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(TEXT("/ViewportManager/Input/IMC_ViewportCamera.IMC_ViewportCamera")));

	if (IMCAsset.IsValid())
	{
		// Use existing IMC
		IMC = Cast<UInputMappingContext>(IMCAsset.GetAsset());
		UE_LOG(LogViewportManager, Log, TEXT("Found existing IMC_ViewportCamera"));
	}
	else
	{
		// Create new IMC
		const FString IMCName = TEXT("IMC_ViewportCamera");
		const FString IMCPackageName = PackagePath + TEXT("/") + IMCName;

		UPackage* IMCPackage = CreatePackage(*IMCPackageName);
		if (IMCPackage)
		{
			IMC = NewObject<UInputMappingContext>(IMCPackage, *IMCName, RF_Public | RF_Standalone);
			if (IMC)
			{
				IMCPackage->MarkPackageDirty();
				FString IMCPackageFileName = FPackageName::LongPackageNameToFilename(IMCPackageName, FPackageName::GetAssetPackageExtension());

				FSavePackageArgs SaveArgs;
				SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
				SaveArgs.SaveFlags = SAVE_NoError;

				if (UPackage::SavePackage(IMCPackage, IMC, *IMCPackageFileName, SaveArgs))
				{
					CreatedAssets.Add(TEXT("IMC_ViewportCamera"));
					UE_LOG(LogViewportManager, Log, TEXT("Created InputMappingContext: IMC_ViewportCamera"));
				}
				else
				{
					FailedAssets.Add(TEXT("IMC_ViewportCamera (Save failed)"));
					IMC = nullptr;
				}
			}
			else
			{
				FailedAssets.Add(TEXT("IMC_ViewportCamera (Object creation failed)"));
			}
		}
		else
		{
			FailedAssets.Add(TEXT("IMC_ViewportCamera (Package creation failed)"));
		}
	}

	if (IMC && IA_Look && IA_Zoom && IA_OrbitButton && IA_PanButton && IA_LeftMouseButton && IA_Move && IA_Sprint)
	{
		// Clear existing mappings to avoid duplicates
		IMC->UnmapAll();

		// Add mappings for camera controls
		if (IA_Look) { IMC->MapKey(IA_Look, FKey("Mouse2D")); }
		if (IA_Zoom) { IMC->MapKey(IA_Zoom, FKey("MouseWheelAxis")); }
		if (IA_OrbitButton) { IMC->MapKey(IA_OrbitButton, FKey("RightMouseButton")); }
		if (IA_PanButton) { IMC->MapKey(IA_PanButton, FKey("MiddleMouseButton")); }
		if (IA_LeftMouseButton) { IMC->MapKey(IA_LeftMouseButton, FKey("LeftMouseButton")); }

		// Add movement keys for free camera
		if (IA_Move)
		{
			IMC->MapKey(IA_Move, FKey("W"));
			IMC->MapKey(IA_Move, FKey("S"));
			IMC->MapKey(IA_Move, FKey("A"));
			IMC->MapKey(IA_Move, FKey("D"));
			IMC->MapKey(IA_Move, FKey("E"));
			IMC->MapKey(IA_Move, FKey("Q"));
		}
		if (IA_Sprint) { IMC->MapKey(IA_Sprint, FKey("LeftShift")); }

		IMC->MarkPackageDirty();
		ConfiguredSteps.Add(TEXT("✓ Configured IMC_ViewportCamera with all mappings"));
	}

	// Assign InputActions to camera pawn defaults
	if (IA_Look && IA_Zoom && IA_OrbitButton && IA_PanButton && IA_LeftMouseButton)
	{
		AVMCameraPawn* CameraPawnCDO = GetMutableDefault<AVMCameraPawn>();
		if (CameraPawnCDO)
		{
			CameraPawnCDO->IA_Look = IA_Look;
			CameraPawnCDO->IA_Zoom = IA_Zoom;
			CameraPawnCDO->IA_OrbitButton = IA_OrbitButton;
			CameraPawnCDO->IA_PanButton = IA_PanButton;
			CameraPawnCDO->IA_LeftMouseButton = IA_LeftMouseButton;
			CameraPawnCDO->InputMappingContext = IMC;
			CameraPawnCDO->GetClass()->GetDefaultObject()->MarkPackageDirty();
			ConfiguredSteps.Add(TEXT("✓ Assigned InputActions to VMCameraPawn"));
		}
	}

	if (IA_Move && IA_Look && IA_Sprint)
	{
		AVMFreeCameraPawn* FreeCameraPawnCDO = GetMutableDefault<AVMFreeCameraPawn>();
		if (FreeCameraPawnCDO)
		{
			FreeCameraPawnCDO->IA_Move = IA_Move;
			FreeCameraPawnCDO->IA_Look = IA_Look;
			FreeCameraPawnCDO->IA_Sprint = IA_Sprint;
			FreeCameraPawnCDO->InputMappingContext = IMC;
			FreeCameraPawnCDO->GetClass()->GetDefaultObject()->MarkPackageDirty();
			ConfiguredSteps.Add(TEXT("✓ Assigned InputActions to VMFreeCameraPawn"));
		}
	}

	// Build result message
	FString ResultMessage = TEXT("✓ Enhanced Input Setup Complete!\n\n");

	if (CreatedAssets.Num() > 0)
	{
		ResultMessage += FString::Printf(TEXT("Created %d InputAction asset(s):\n"), CreatedAssets.Num());
		for (const FString& Asset : CreatedAssets)
		{
			ResultMessage += TEXT("  ✓ ") + Asset + TEXT("\n");
		}
		ResultMessage += TEXT("\n");
	}
	else
	{
		ResultMessage += TEXT("All InputAction assets already exist!\n\n");
	}

	if (FailedAssets.Num() > 0)
	{
		ResultMessage += FString::Printf(TEXT("Failed to create %d asset(s):\n"), FailedAssets.Num());
		for (const FString& Asset : FailedAssets)
		{
			ResultMessage += TEXT("  ✗ ") + Asset + TEXT("\n");
		}
		ResultMessage += TEXT("\n");
	}

	if (ConfiguredSteps.Num() > 0)
	{
		ResultMessage += TEXT("Automatic Configuration:\n");
		for (const FString& Step : ConfiguredSteps)
		{
			ResultMessage += TEXT("  ") + Step + TEXT("\n");
		}
		ResultMessage += TEXT("\n");
	}

	ResultMessage += TEXT("✓ Everything is ready! Camera controls will work immediately!\n");
	ResultMessage += TEXT("Assets saved to: /ViewportManager/Input/\n");

	UE_LOG(LogViewportManager, Log, TEXT("%s"), *ResultMessage);

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultMessage));

	// Refresh asset registry
	AssetRegistryModule.Get().ScanPathsSynchronous({ InputFolder }, true);
}
