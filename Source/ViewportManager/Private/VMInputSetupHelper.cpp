#include "VMInputSetupHelper.h"
#include "VMLog.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/DataAssetFactory.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "PlayerMappableInputConfig.h"
#include "UObject/SavePackage.h"
#include "Misc/MessageDialog.h"
#include "VMLog.h"
#endif

void UVMInputSetupHelper::SetupEnhancedInputAssets()
{
#if WITH_EDITOR
	const FString InputFolder = TEXT("/ViewportManager/Input");
	const FString PackagePath = TEXT("/Game") + InputFolder;

	TArray<FString> CreatedAssets;
	TArray<FString> FailedAssets;

	auto CreateInputAction = [&](const FString& Name, EInputActionValueType ValueType) -> UInputAction*
	{
		const FString AssetName = Name;
		const FString PackageName = PackagePath + TEXT("/") + AssetName;

		UPackage* Package = CreatePackage(*PackageName);
		if (!Package)
		{
			FailedAssets.Add(AssetName + TEXT(" (Package creation failed)"));
			return nullptr;
		}

		UInputAction* InputAction = NewObject<UInputAction>(Package, *AssetName, RF_Public | RF_Standalone);
		if (!InputAction)
		{
			FailedAssets.Add(AssetName + TEXT(" (Object creation failed)"));
			return nullptr;
		}

		InputAction->ValueType = ValueType;

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

	UE_LOG(LogViewportManager, Log, TEXT("=== Starting Enhanced Input Setup ==="));

	UInputAction* IA_Look = CreateInputAction(TEXT("IA_Look"), EInputActionValueType::Axis2D);
	UInputAction* IA_Zoom = CreateInputAction(TEXT("IA_Zoom"), EInputActionValueType::Axis1D);
	UInputAction* IA_OrbitButton = CreateInputAction(TEXT("IA_OrbitButton"), EInputActionValueType::Boolean);
	UInputAction* IA_PanButton = CreateInputAction(TEXT("IA_PanButton"), EInputActionValueType::Boolean);
	UInputAction* IA_LeftMouseButton = CreateInputAction(TEXT("IA_LeftMouseButton"), EInputActionValueType::Boolean);
	UInputAction* IA_Move = CreateInputAction(TEXT("IA_Move"), EInputActionValueType::Axis3D);
	UInputAction* IA_Sprint = CreateInputAction(TEXT("IA_Sprint"), EInputActionValueType::Boolean);

	FString ResultMessage = TEXT("Enhanced Input Setup Complete!\n\n");

	if (CreatedAssets.Num() > 0)
	{
		ResultMessage += FString::Printf(TEXT("Created %d InputAction asset(s):\n"), CreatedAssets.Num());
		for (const FString& Asset : CreatedAssets)
		{
			ResultMessage += TEXT("  ✓ ") + Asset + TEXT("\n");
		}
		ResultMessage += TEXT("\n");
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

	ResultMessage += TEXT("Next Steps:\n");
	ResultMessage += TEXT("1. Open IMC_ViewportCamera in /ViewportManager/Input/\n");
	ResultMessage += TEXT("2. Add mappings for each InputAction:\n");
	ResultMessage += TEXT("   - IA_Look → Mouse XY 2D-Axis\n");
	ResultMessage += TEXT("   - IA_Zoom → Mouse Wheel Axis\n");
	ResultMessage += TEXT("   - IA_OrbitButton → Right Mouse Button\n");
	ResultMessage += TEXT("   - IA_PanButton → Middle Mouse Button\n");
	ResultMessage += TEXT("   - IA_LeftMouseButton → Left Mouse Button\n");
	ResultMessage += TEXT("   - IA_Move → WASD + EQ keys (3D vector)\n");
	ResultMessage += TEXT("   - IA_Sprint → Left Shift\n");
	ResultMessage += TEXT("3. Assign the InputActions to your camera pawns\n");

	UE_LOG(LogViewportManager, Log, TEXT("%s"), *ResultMessage);

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultMessage));

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().ScanPathsSynchronous({ InputFolder }, true);

#else
	UE_LOG(LogViewportManager, Warning, TEXT("SetupEnhancedInputAssets can only be called in the editor"));
#endif
}
