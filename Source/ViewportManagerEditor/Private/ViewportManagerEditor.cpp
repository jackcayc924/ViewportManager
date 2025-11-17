#include "ViewportManagerEditor.h"
#include "AssetToolsModule.h"
#include "VMSplitLayoutFactory.h"
#include "ToolMenus.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "SVMLayoutDesignerWidget.h"
#include "SVMQuickStartWizardWidget.h"
#include "Styles/VMLayoutEditorStyle.h"
#include "VMInputSetupWidget.h"
#include "GameFramework/InputSettings.h"
#include "InputCoreTypes.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "FViewportManagerEditorModule"

const FName FViewportManagerEditorModule::LayoutDesignerMenuEntryName(TEXT("ViewportManager_OpenLayoutDesigner"));
const FName FViewportManagerEditorModule::QuickStartMenuEntryName(TEXT("ViewportManager_OpenQuickStartWizard"));
const FName FViewportManagerEditorModule::SetupInputMenuEntryName(TEXT("ViewportManager_SetupLegacyInput"));

void FViewportManagerEditorModule::StartupModule()
{
	// Initialize modern style system
	FVMLayoutEditorStyle::Initialize();

	// Register asset type actions
	RegisterAssetTypeActions();
	RegisterMenus();
}

void FViewportManagerEditorModule::ShutdownModule()
{
	if (LayoutDesignerWindow.IsValid())
	{
		LayoutDesignerWindow.Pin()->RequestDestroyWindow();
		LayoutDesignerWindow.Reset();
	}

	if (QuickStartWindow.IsValid())
	{
		QuickStartWindow.Pin()->RequestDestroyWindow();
		QuickStartWindow.Reset();
	}

	UnregisterMenus();
	// Unregister asset type actions
	UnregisterAssetTypeActions();

	// Shutdown style system
	FVMLayoutEditorStyle::Shutdown();
}

void FViewportManagerEditorModule::RegisterAssetTypeActions()
{
	// Get the asset tools module
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register VM Split Layout asset type actions
	TSharedRef<IAssetTypeActions> VMSplitLayoutActions = MakeShared<FAssetTypeActions_VMSplitLayout>();
	AssetTools.RegisterAssetTypeActions(VMSplitLayoutActions);
	CreatedAssetTypeActions.Add(VMSplitLayoutActions);
}

void FViewportManagerEditorModule::UnregisterAssetTypeActions()
{
	// Unregister asset type actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();
}

void FViewportManagerEditorModule::RegisterMenus()
{
	MenuRegistrationHandle = UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FViewportManagerEditorModule::ExtendMenus));
}

void FViewportManagerEditorModule::UnregisterMenus()
{
	if (MenuRegistrationHandle.IsValid())
	{
		UToolMenus::UnRegisterStartupCallback(MenuRegistrationHandle);
		MenuRegistrationHandle.Reset();
	}

	if (UToolMenus::Get())
	{
		if (UToolMenu* Menu = UToolMenus::Get()->FindMenu("LevelEditor.MainMenu.Window"))
		{
			Menu->RemoveSection("ViewportManager");
		}
	}
}

void FViewportManagerEditorModule::ExtendMenus()
{
	if (!UToolMenus::Get())
	{
		return;
	}

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	if (!Menu)
	{
		return;
	}

	FToolMenuSection* Section = Menu->FindSection("ViewportManager");
	if (!Section)
	{
		Section = &Menu->AddSection("ViewportManager", LOCTEXT("ViewportManagerSection", "Viewport Manager"));
	}

	Section->AddMenuEntry(
		LayoutDesignerMenuEntryName,
		LOCTEXT("OpenLayoutDesigner_Label", "Layout Designer"),
		LOCTEXT("OpenLayoutDesigner_Tooltip", "Open the Viewport Manager layout designer window."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FViewportManagerEditorModule::OpenLayoutDesigner))
	);

	Section->AddMenuEntry(
		QuickStartMenuEntryName,
		LOCTEXT("OpenQuickStart_Label", "Quick Start Wizard"),
		LOCTEXT("OpenQuickStart_Tooltip", "Launch the guided setup wizard for Viewport Manager."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FViewportManagerEditorModule::OpenQuickStartWizard))
	);

	Section->AddMenuEntry(
		SetupInputMenuEntryName,
		LOCTEXT("SetupInput_Label", "Setup Camera Input"),
		LOCTEXT("SetupInput_Tooltip", "Automatically configure camera input bindings for ViewportManager."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FViewportManagerEditorModule::SetupLegacyInput))
	);
}

void FViewportManagerEditorModule::OpenLayoutDesigner()
{
	if (LayoutDesignerWindow.IsValid())
	{
		LayoutDesignerWindow.Pin()->BringToFront(true);
		return;
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("LayoutDesignerWindowTitle", "Viewport Layout Designer"))
		.ClientSize(FVector2D(1040.f, 720.f))
		.SupportsMinimize(true)
		.SupportsMaximize(true);

	Window->SetContent(SNew(SVMLayoutDesignerWidget));
	FSlateApplication::Get().AddWindow(Window);
	LayoutDesignerWindow = Window;
}

void FViewportManagerEditorModule::OpenQuickStartWizard()
{
	if (QuickStartWindow.IsValid())
	{
		QuickStartWindow.Pin()->BringToFront(true);
		return;
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("QuickStartWindowTitle", "Viewport Manager Quick Start"))
		.ClientSize(FVector2D(600.f, 520.f))
		.SupportsMinimize(false)
		.SupportsMaximize(false);

	Window->SetContent(SNew(SVMQuickStartWizardWidget));
	FSlateApplication::Get().AddWindow(Window);
	QuickStartWindow = Window;
}

void FViewportManagerEditorModule::SetupLegacyInput()
{
	UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
	if (!InputSettings)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Error: Could not access Input Settings.")));
		return;
	}

	const int32 InitialActionCount = InputSettings->GetActionMappings().Num();
	const int32 InitialAxisCount = InputSettings->GetAxisMappings().Num();

	// Helper lambdas to add mappings if they don't exist
	auto AddActionMapping = [&](const FName& Name, const FKey& Key)
	{
		FInputActionKeyMapping Mapping(Name, Key);
		InputSettings->AddActionMapping(Mapping, false); // AddUnique = false to avoid duplicates
	};

	auto AddAxisMapping = [&](const FName& Name, const FKey& Key, float Scale)
	{
		FInputAxisKeyMapping Mapping(Name, Key, Scale);
		InputSettings->AddAxisMapping(Mapping, false); // AddUnique = false to avoid duplicates
	};

	// Action Mappings for VMCameraPawn & VMFreeCameraPawn
	AddActionMapping("RightMouseButton", EKeys::RightMouseButton);
	AddActionMapping("MiddleMouseButton", EKeys::MiddleMouseButton);
	AddActionMapping("LeftMouseButton", EKeys::LeftMouseButton);
	AddActionMapping("LeftShift", EKeys::LeftShift);

	// Axis Mappings for camera controls
	AddAxisMapping("Turn", EKeys::MouseX, 1.0f);
	AddAxisMapping("LookUp", EKeys::MouseY, -1.0f);
	AddAxisMapping("MouseWheelAxis", EKeys::MouseWheelAxis, 1.0f);
	AddAxisMapping("MoveForward", EKeys::W, 1.0f);
	AddAxisMapping("MoveForward", EKeys::S, -1.0f);
	AddAxisMapping("MoveRight", EKeys::D, 1.0f);
	AddAxisMapping("MoveRight", EKeys::A, -1.0f);
	AddAxisMapping("MoveUp", EKeys::E, 1.0f);
	AddAxisMapping("MoveUp", EKeys::Q, -1.0f);

	const int32 ActionsAdded = InputSettings->GetActionMappings().Num() - InitialActionCount;
	const int32 AxesAdded = InputSettings->GetAxisMappings().Num() - InitialAxisCount;

	FString ResultMessage;
	if (ActionsAdded > 0 || AxesAdded > 0)
	{
		InputSettings->SaveConfig();
		InputSettings->TryUpdateDefaultConfigFile();
		ResultMessage = TEXT("Camera Input Setup Complete!\n\n");
		ResultMessage += FString::Printf(TEXT("Added %d action and %d axis mapping(s) to Project Settings.\n\n"), ActionsAdded, AxesAdded);
		ResultMessage += TEXT("Camera controls are now configured and ready to use!");
	}
	else
	{
		ResultMessage = TEXT("Camera Input Already Configured\n\nAll required input mappings already exist. No changes were needed.\n\nCamera controls are ready to use!");
	}

	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(ResultMessage));
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FViewportManagerEditorModule, ViewportManagerEditor)



