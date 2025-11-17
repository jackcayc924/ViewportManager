#include "SVMQuickStartWizardWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Styling/AppStyle.h"
#include "Editor.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "ISettingsModule.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformProcess.h"
#include "GameFramework/InputSettings.h"
#include "InputCoreTypes.h"

#define LOCTEXT_NAMESPACE "SVMQuickStartWizardWidget"

void SVMQuickStartWizardWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot()
		.Padding(16.f)
		[
			SNew(SVerticalBox)

			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 16.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("WizardTitle", "Viewport Manager Setup Wizard"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 4.f, 0.f, 0.f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("WizardSubtitle", "Automated system configuration and validation"))
					.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				]
			]

			// Overall Status Banner
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 16.f)
			[
				SNew(SBorder)
				.Padding(12.f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 8.f, 0.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() { return AreAllSystemChecksValid() ? LOCTEXT("StatusReady", "✅") : LOCTEXT("StatusNotReady", "⚠"); })
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() { return GetOverallStatus(); })
						.WrapTextAt(500.f)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("FixAllButton", "Fix All Issues"))
						.OnClicked(this, &SVMQuickStartWizardWidget::FixAllIssues)
						.IsEnabled_Lambda([this]() { return !AreAllSystemChecksValid(); })
					]
				]
			]

			// Check 1: Game Viewport Client
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.Padding(12.f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() { return IsGameViewportClientConfigured() ? LOCTEXT("CheckPass", "✅") : LOCTEXT("CheckFail", "❌"); })
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Check1Title", "Game Viewport Client"))
							.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return GetGameViewportClientStatus(); })
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.WrapTextAt(450.f)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("FixButton", "Fix"))
						.OnClicked(this, &SVMQuickStartWizardWidget::FixGameViewportClient)
						.IsEnabled_Lambda([this]() { return !IsGameViewportClientConfigured(); })
					]
				]
			]

			// Check 2: Game Instance
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.Padding(12.f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() { return IsGameInstanceConfigured() ? LOCTEXT("CheckPass", "✅") : LOCTEXT("CheckFail", "❌"); })
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Check2Title", "Game Instance"))
							.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return GetGameInstanceStatus(); })
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.WrapTextAt(450.f)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("FixButton", "Fix"))
						.OnClicked(this, &SVMQuickStartWizardWidget::FixGameInstance)
						.IsEnabled_Lambda([this]() { return !IsGameInstanceConfigured(); })
					]
				]
			]

			// Check 3: Game Mode
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.Padding(12.f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() { return IsGameModeConfigured() ? LOCTEXT("CheckPass", "✅") : LOCTEXT("CheckFail", "❌"); })
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Check3Title", "Default Game Mode"))
							.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return GetGameModeStatus(); })
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.WrapTextAt(450.f)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("FixButton", "Fix"))
						.OnClicked(this, &SVMQuickStartWizardWidget::FixGameMode)
						.IsEnabled_Lambda([this]() { return !IsGameModeConfigured(); })
					]
				]
			]

			// Check 4: Camera Input
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SBorder)
				.Padding(12.f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() { return IsCameraInputConfigured() ? LOCTEXT("CheckPass", "✅") : LOCTEXT("CheckFail", "❌"); })
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Check4Title", "Camera Input Mappings"))
							.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return GetCameraInputStatus(); })
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.WrapTextAt(450.f)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("FixButton", "Fix"))
						.OnClicked(this, &SVMQuickStartWizardWidget::FixCameraInput)
						.IsEnabled_Lambda([this]() { return !IsCameraInputConfigured(); })
					]
				]
			]

			// Check 5: Max Players
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 16.f)
			[
				SNew(SBorder)
				.Padding(12.f)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.f, 0.f, 12.f, 0.f)
					[
						SNew(STextBlock)
						.Text_Lambda([this]() { return IsMaxPlayersConfigured() ? LOCTEXT("CheckPass", "✅") : LOCTEXT("CheckFail", "❌"); })
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Check5Title", "Max Splitscreen Players"))
							.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.f, 2.f, 0.f, 0.f)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return GetMaxPlayersStatus(); })
							.ColorAndOpacity(FSlateColor::UseSubduedForeground())
							.WrapTextAt(450.f)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("FixButton", "Fix"))
						.OnClicked(this, &SVMQuickStartWizardWidget::FixMaxPlayers)
						.IsEnabled_Lambda([this]() { return !IsMaxPlayersConfigured(); })
					]
				]
			]

			// Additional Resources Section
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 8.f)
			[
				SNew(SSeparator)
				.Thickness(2.f)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 8.f, 0.f, 8.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AdditionalResourcesTitle", "Additional Resources"))
				.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 4.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenProjectSettingsButton", "Open Project Settings"))
				.OnClicked(this, &SVMQuickStartWizardWidget::HandleOpenProjectSettings)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SButton)
				.Text(LOCTEXT("OpenDocumentationButton", "Open Documentation"))
				.OnClicked(this, &SVMQuickStartWizardWidget::HandleOpenDocumentation)
			]
		]
	];
}

void SVMQuickStartWizardWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Auto-refresh checks periodically
	TimeSinceLastCheck += InDeltaTime;
	if (TimeSinceLastCheck >= CheckInterval)
	{
		TimeSinceLastCheck = 0.f;
		// Force widget refresh by invalidating
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

// ================================================================================
// VALIDATION CHECKS
// ================================================================================

bool SVMQuickStartWizardWidget::IsGameViewportClientConfigured() const
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *ConfigPath))
	{
		return FileContent.Contains(TEXT("GameViewportClientClassName=/Script/ViewportManager.VMGameViewportClient"));
	}
	return false;
}

bool SVMQuickStartWizardWidget::IsGameInstanceConfigured() const
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *ConfigPath))
	{
		return FileContent.Contains(TEXT("GameInstanceClass=/Script/ViewportManager.VMGameInstance"));
	}
	return false;
}

bool SVMQuickStartWizardWidget::IsGameModeConfigured() const
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *ConfigPath))
	{
		return FileContent.Contains(TEXT("GlobalDefaultGameMode=/Script/ViewportManager.VMSplitGameMode"));
	}
	return false;
}

bool SVMQuickStartWizardWidget::IsCameraInputConfigured() const
{
	UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
	if (!InputSettings)
	{
		return false;
	}

	// Check for required axis mappings
	const TArray<FInputAxisKeyMapping>& AxisMappings = InputSettings->GetAxisMappings();
	bool bHasMoveForward = false;
	bool bHasTurn = false;
	bool bHasLookUp = false;

	for (const FInputAxisKeyMapping& Mapping : AxisMappings)
	{
		if (Mapping.AxisName == TEXT("MoveForward")) bHasMoveForward = true;
		if (Mapping.AxisName == TEXT("Turn")) bHasTurn = true;
		if (Mapping.AxisName == TEXT("LookUp")) bHasLookUp = true;
	}

	// Check for required action mappings
	const TArray<FInputActionKeyMapping>& ActionMappings = InputSettings->GetActionMappings();
	bool bHasRightMouseButton = false;

	for (const FInputActionKeyMapping& Mapping : ActionMappings)
	{
		if (Mapping.ActionName == TEXT("RightMouseButton")) bHasRightMouseButton = true;
	}

	return bHasMoveForward && bHasTurn && bHasLookUp && bHasRightMouseButton;
}

bool SVMQuickStartWizardWidget::IsMaxPlayersConfigured() const
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	FString FileContent;
	if (FFileHelper::LoadFileToString(FileContent, *ConfigPath))
	{
		return FileContent.Contains(TEXT("MaxSplitscreenPlayers=32"));
	}
	return false;
}

bool SVMQuickStartWizardWidget::AreAllSystemChecksValid() const
{
	return IsGameViewportClientConfigured() &&
	       IsGameInstanceConfigured() &&
	       IsGameModeConfigured() &&
	       IsCameraInputConfigured() &&
	       IsMaxPlayersConfigured();
}

// ================================================================================
// STATUS TEXT GETTERS
// ================================================================================

FText SVMQuickStartWizardWidget::GetGameViewportClientStatus() const
{
	return IsGameViewportClientConfigured()
		? LOCTEXT("ViewportClientOK", "VMGameViewportClient is configured in DefaultEngine.ini")
		: LOCTEXT("ViewportClientMissing", "VMGameViewportClient not set. Click Fix to configure.");
}

FText SVMQuickStartWizardWidget::GetGameInstanceStatus() const
{
	return IsGameInstanceConfigured()
		? LOCTEXT("GameInstanceOK", "VMGameInstance is configured in DefaultEngine.ini")
		: LOCTEXT("GameInstanceMissing", "VMGameInstance not set. Click Fix to configure.");
}

FText SVMQuickStartWizardWidget::GetGameModeStatus() const
{
	return IsGameModeConfigured()
		? LOCTEXT("GameModeOK", "VMSplitGameMode is set as GlobalDefaultGameMode")
		: LOCTEXT("GameModeMissing", "VMSplitGameMode not set. Click Fix to configure.");
}

FText SVMQuickStartWizardWidget::GetCameraInputStatus() const
{
	return IsCameraInputConfigured()
		? LOCTEXT("InputOK", "All camera input mappings are configured")
		: LOCTEXT("InputMissing", "Camera input mappings missing. Click Fix to add them.");
}

FText SVMQuickStartWizardWidget::GetMaxPlayersStatus() const
{
	return IsMaxPlayersConfigured()
		? LOCTEXT("MaxPlayersOK", "MaxSplitscreenPlayers set to 32")
		: LOCTEXT("MaxPlayersMissing", "MaxSplitscreenPlayers not set to 32. Click Fix to configure.");
}

FText SVMQuickStartWizardWidget::GetOverallStatus() const
{
	if (AreAllSystemChecksValid())
	{
		return LOCTEXT("OverallStatusReady", "System Ready - All checks passed! You can now use Viewport Manager.");
	}
	else
	{
		int32 PassedChecks = 0;
		if (IsGameViewportClientConfigured()) PassedChecks++;
		if (IsGameInstanceConfigured()) PassedChecks++;
		if (IsGameModeConfigured()) PassedChecks++;
		if (IsCameraInputConfigured()) PassedChecks++;
		if (IsMaxPlayersConfigured()) PassedChecks++;

		return FText::Format(LOCTEXT("OverallStatusNotReady", "Configuration Incomplete - {0}/5 checks passed. Click Fix All or individual Fix buttons."),
			FText::AsNumber(PassedChecks));
	}
}

// ================================================================================
// FIX BUTTON HANDLERS
// ================================================================================

FReply SVMQuickStartWizardWidget::FixGameViewportClient()
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");

	if (UpdateConfigValue(ConfigPath, TEXT("[/Script/Engine.Engine]"), TEXT("GameViewportClientClassName"), TEXT("/Script/ViewportManager.VMGameViewportClient")))
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixViewportClientSuccess", "GameViewportClientClassName has been set to VMGameViewportClient in DefaultEngine.ini."));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixViewportClientFailed", "Failed to update DefaultEngine.ini. Please check file permissions."));
	}

	return FReply::Handled();
}

FReply SVMQuickStartWizardWidget::FixGameInstance()
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");

	if (UpdateConfigValue(ConfigPath, TEXT("[/Script/EngineSettings.GameMapsSettings]"), TEXT("GameInstanceClass"), TEXT("/Script/ViewportManager.VMGameInstance")))
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixGameInstanceSuccess", "GameInstanceClass has been set to VMGameInstance in DefaultEngine.ini."));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixGameInstanceFailed", "Failed to update DefaultEngine.ini. Please check file permissions."));
	}

	return FReply::Handled();
}

FReply SVMQuickStartWizardWidget::FixGameMode()
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");

	if (UpdateConfigValue(ConfigPath, TEXT("[/Script/EngineSettings.GameMapsSettings]"), TEXT("GlobalDefaultGameMode"), TEXT("/Script/ViewportManager.VMSplitGameMode")))
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixGameModeSuccess", "GlobalDefaultGameMode has been set to VMSplitGameMode in DefaultEngine.ini."));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixGameModeFailed", "Failed to update DefaultEngine.ini. Please check file permissions."));
	}

	return FReply::Handled();
}

FReply SVMQuickStartWizardWidget::FixCameraInput()
{
	UInputSettings* InputSettings = GetMutableDefault<UInputSettings>();
	if (!InputSettings)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixInputFailed", "Error: Could not access Input Settings."));
		return FReply::Handled();
	}

	const int32 InitialActionCount = InputSettings->GetActionMappings().Num();
	const int32 InitialAxisCount = InputSettings->GetAxisMappings().Num();

	// Helper lambdas
	auto AddActionMapping = [&](const FName& Name, const FKey& Key)
	{
		FInputActionKeyMapping Mapping(Name, Key);
		InputSettings->AddActionMapping(Mapping, false);
	};

	auto AddAxisMapping = [&](const FName& Name, const FKey& Key, float Scale)
	{
		FInputAxisKeyMapping Mapping(Name, Key, Scale);
		InputSettings->AddAxisMapping(Mapping, false);
	};

	// Add all camera control mappings
	AddActionMapping(TEXT("RightMouseButton"), EKeys::RightMouseButton);
	AddActionMapping(TEXT("MiddleMouseButton"), EKeys::MiddleMouseButton);
	AddActionMapping(TEXT("LeftMouseButton"), EKeys::LeftMouseButton);
	AddActionMapping(TEXT("LeftShift"), EKeys::LeftShift);

	AddAxisMapping(TEXT("Turn"), EKeys::MouseX, 1.0f);
	AddAxisMapping(TEXT("LookUp"), EKeys::MouseY, -1.0f);
	AddAxisMapping(TEXT("MouseWheelAxis"), EKeys::MouseWheelAxis, 1.0f);
	AddAxisMapping(TEXT("MoveForward"), EKeys::W, 1.0f);
	AddAxisMapping(TEXT("MoveForward"), EKeys::S, -1.0f);
	AddAxisMapping(TEXT("MoveRight"), EKeys::D, 1.0f);
	AddAxisMapping(TEXT("MoveRight"), EKeys::A, -1.0f);
	AddAxisMapping(TEXT("MoveUp"), EKeys::E, 1.0f);
	AddAxisMapping(TEXT("MoveUp"), EKeys::Q, -1.0f);

	const int32 ActionsAdded = InputSettings->GetActionMappings().Num() - InitialActionCount;
	const int32 AxesAdded = InputSettings->GetAxisMappings().Num() - InitialAxisCount;

	if (ActionsAdded > 0 || AxesAdded > 0)
	{
		InputSettings->SaveConfig();
		InputSettings->TryUpdateDefaultConfigFile();
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("FixInputSuccess", "Camera input configured! Added {0} action and {1} axis mapping(s)."),
			FText::AsNumber(ActionsAdded), FText::AsNumber(AxesAdded)));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixInputAlreadyDone", "All camera input mappings already exist."));
	}

	return FReply::Handled();
}

FReply SVMQuickStartWizardWidget::FixMaxPlayers()
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");

	if (UpdateConfigValue(ConfigPath, TEXT("[/Script/Engine.GameViewportClient]"), TEXT("MaxSplitscreenPlayers"), TEXT("32")))
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixMaxPlayersSuccess", "MaxSplitscreenPlayers has been set to 32 in DefaultEngine.ini."));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixMaxPlayersFailed", "Failed to update DefaultEngine.ini. Please check file permissions."));
	}

	return FReply::Handled();
}

FReply SVMQuickStartWizardWidget::FixAllIssues()
{
	int32 FixedCount = 0;

	if (!IsGameViewportClientConfigured())
	{
		FixGameViewportClient();
		FixedCount++;
	}

	if (!IsGameInstanceConfigured())
	{
		FixGameInstance();
		FixedCount++;
	}

	if (!IsGameModeConfigured())
	{
		FixGameMode();
		FixedCount++;
	}

	if (!IsCameraInputConfigured())
	{
		FixCameraInput();
		FixedCount++;
	}

	if (!IsMaxPlayersConfigured())
	{
		FixMaxPlayers();
		FixedCount++;
	}

	if (FixedCount > 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("FixAllSuccess", "Fixed {0} configuration issue(s)! Viewport Manager is now ready to use."),
			FText::AsNumber(FixedCount)));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FixAllNothingToDo", "All configurations are already correct!"));
	}

	return FReply::Handled();
}

// ================================================================================
// UTILITY FUNCTIONS
// ================================================================================

bool SVMQuickStartWizardWidget::UpdateConfigValue(const FString& ConfigPath, const FString& Section, const FString& Key, const FString& Value)
{
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *ConfigPath))
	{
		return false;
	}

	TArray<FString> Lines;
	FileContent.ParseIntoArrayLines(Lines);

	bool bInSection = false;
	bool bKeyFound = false;
	int32 SectionEndIndex = -1;

	// Find the section and check if key exists
	for (int32 i = 0; i < Lines.Num(); i++)
	{
		FString Line = Lines[i].TrimStartAndEnd();

		// Check if we're entering the target section
		if (Line.Equals(Section, ESearchCase::IgnoreCase))
		{
			bInSection = true;
			SectionEndIndex = i;
			continue;
		}

		// If we're in the section
		if (bInSection)
		{
			// Check if we've left the section (found another section header)
			if (Line.StartsWith(TEXT("[")))
			{
				break;
			}

			// Check if this line contains our key
			if (Line.StartsWith(Key + TEXT("=")))
			{
				// Update existing key
				Lines[i] = FString::Printf(TEXT("%s=%s"), *Key, *Value);
				bKeyFound = true;
				break;
			}

			SectionEndIndex = i;
		}
	}

	// If section exists but key doesn't, add the key
	if (bInSection && !bKeyFound && SectionEndIndex >= 0)
	{
		Lines.Insert(FString::Printf(TEXT("%s=%s"), *Key, *Value), SectionEndIndex + 1);
	}
	// If section doesn't exist, create it
	else if (!bInSection)
	{
		Lines.Add(TEXT(""));
		Lines.Add(Section);
		Lines.Add(FString::Printf(TEXT("%s=%s"), *Key, *Value));
	}

	// Write back to file
	FString NewContent = FString::Join(Lines, TEXT("\n"));
	return FFileHelper::SaveStringToFile(NewContent, *ConfigPath);
}

FReply SVMQuickStartWizardWidget::HandleOpenProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->ShowViewer("Project", "Project", "Maps & Modes");
	}
	return FReply::Handled();
}

FReply SVMQuickStartWizardWidget::HandleOpenDocumentation()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("ViewportManager"));
	if (!Plugin.IsValid())
	{
		return FReply::Handled();
	}

	const FString DocPath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Docs/ViewportManagerQuickStart.md"));
	if (FPaths::FileExists(DocPath))
	{
		FPlatformProcess::LaunchFileInDefaultExternalApplication(*DocPath);
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("DocNotFound", "Documentation file not found: {0}"), FText::FromString(DocPath)));
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
