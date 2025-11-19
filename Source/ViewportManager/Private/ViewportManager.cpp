// Copyright jackcayc924 2025. All Rights Reserved.

#include "ViewportManager.h"
#include "Delegates/IDelegateInstance.h"

#include "Engine/Engine.h"
#include "Engine/GameEngine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "VMGameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Widgets/SWindow.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"

#if WITH_EDITOR
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "ISettingsModule.h"
#endif

DEFINE_LOG_CATEGORY(LogViewportManager);

#define LOCTEXT_NAMESPACE "FViewportManagerModule"

void FViewportManagerModule::StartupModule()
{
	UE_LOG(LogViewportManager, Log, TEXT("Viewport Manager module starting"));

#if WITH_EDITOR
	ValidateViewportClientSetting();
#endif

	if (GEngine)
	{
		InstallViewportClient();
	}
	else
	{
		PostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddRaw(this, &FViewportManagerModule::OnPostEngineInit);
	}
}

void FViewportManagerModule::ShutdownModule()
{
	if (PostEngineInitHandle.IsValid())
	{
		FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
		PostEngineInitHandle.Reset();
	}
}

void FViewportManagerModule::OnPostEngineInit()
{
	InstallViewportClient();

	if (PostEngineInitHandle.IsValid())
	{
		FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
		PostEngineInitHandle.Reset();
	}
}

void FViewportManagerModule::InstallViewportClient()
{
	if (IsRunningCommandlet() || IsRunningDedicatedServer())
	{
		return;
	}

	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (!GameEngine)
	{
		return;
	}

	GameEngine->GameViewportClientClass = UVMGameViewportClient::StaticClass();

	if (GameEngine->GameViewport == nullptr)
	{
		UE_LOG(LogViewportManager, Verbose, TEXT("Waiting for game viewport creation; Viewport Manager will activate automatically."));
		return;
	}

	if (GameEngine->GameViewport->IsA<UVMGameViewportClient>())
	{
		UE_LOG(LogViewportManager, Verbose, TEXT("Viewport Manager already active (%s)."), *GameEngine->GameViewport->GetClass()->GetName());
		return;
	}

	UE_LOG(LogViewportManager, Warning, TEXT("Detected GameViewportClient '%s'. Set GameViewportClientClassName=/Script/ViewportManager.VMGameViewportClient (or use the Viewport Manager setup prompt) to enable the plugin."), *GameEngine->GameViewport->GetClass()->GetPathName());
}

#if WITH_EDITOR

void FViewportManagerModule::ValidateViewportClientSetting()
{
	if (bSuppressViewportClientPromptForSession || !GIsEditor)
	{
		return;
	}

	const FString ExpectedClassPath(TEXT("/Script/ViewportManager.VMGameViewportClient"));
	FString ConfigValue;
	const bool bHasValue = GConfig->GetString(TEXT("/Script/Engine.Engine"), TEXT("GameViewportClientClassName"), ConfigValue, GEngineIni);

	if (!bHasValue || !ConfigValue.Equals(ExpectedClassPath, ESearchCase::IgnoreCase))
	{
		OpenViewportClientSetupDialog(ConfigValue);
	}
}

void FViewportManagerModule::OpenViewportClientSetupDialog(const FString& CurrentValue)
{
	if (ViewportClientSetupWindow.IsValid() || bSuppressViewportClientPromptForSession)
	{
		return;
	}

	const FText CurrentValueText = CurrentValue.IsEmpty()
		? LOCTEXT("ViewportClientUnsetValue", "<unset>")
		: FText::FromString(CurrentValue);

	const TSharedRef<SWindow> DialogWindow = SNew(SWindow)
		.Title(LOCTEXT("ViewportManagerSetupTitle", "Viewport Manager Setup"))
		.ClientSize(FVector2D(520.f, 260.f))
		.SizingRule(ESizingRule::Autosized)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.ActivationPolicy(EWindowActivationPolicy::Always)
		.HasCloseButton(true);

	DialogWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>&)
	{
		bSuppressViewportClientPromptForSession = true;
		ViewportClientSetupWindow.Reset();
	}));

	TWeakPtr<SWindow> WeakDialogWindow = DialogWindow;

	DialogWindow->SetContent(
		SNew(SBorder)
		.Padding(16.f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 12.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ViewportManagerSetupBody", "Viewport Manager needs Unreal to use VMGameViewportClient. Apply the fix automatically or choose another option below."))
				.WrapTextAt(480.f)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.f, 0.f, 0.f, 16.f)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("ViewportManagerCurrentValue", "Current GameViewportClientClassName: {0}"), CurrentValueText))
				.AutoWrapText(true)
				.Font(FAppStyle::Get().GetFontStyle("Bold"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SButton)
					.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("PrimaryButton"))
					.OnClicked_Lambda([this, WeakDialogWindow]()
					{
						HandleApplyViewportClientSetting();
						if (TSharedPtr<SWindow> Window = WeakDialogWindow.Pin())
						{
							Window->RequestDestroyWindow();
						}
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ViewportManagerApplyFix", "Apply Automatic Fix"))
						.Justification(ETextJustify::Center)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 0.f, 0.f, 8.f)
				[
					SNew(SButton)
					.OnClicked_Lambda([this, WeakDialogWindow]()
					{
						HandleOpenProjectSettings();
						if (TSharedPtr<SWindow> Window = WeakDialogWindow.Pin())
						{
							Window->RequestDestroyWindow();
						}
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ViewportManagerOpenSettings", "Open Project Settings..."))
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SButton)
					.OnClicked_Lambda([this, WeakDialogWindow]()
					{
						HandleDismissViewportClientPrompt();
						if (TSharedPtr<SWindow> Window = WeakDialogWindow.Pin())
						{
							Window->RequestDestroyWindow();
						}
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ViewportManagerDismiss", "Ignore for this Session"))
					]
				]
			]
		]
	);

	FSlateApplication::Get().AddWindow(DialogWindow, true);
	DialogWindow->BringToFront();
	ViewportClientSetupWindow = DialogWindow;
}

void FViewportManagerModule::CloseViewportClientSetupDialog()
{
	if (TSharedPtr<SWindow> Window = ViewportClientSetupWindow.Pin())
	{
		Window->RequestDestroyWindow();
	}
	ViewportClientSetupWindow.Reset();
}

void FViewportManagerModule::HandleApplyViewportClientSetting()
{
	const FString Section(TEXT("/Script/Engine.Engine"));
	const FString Key(TEXT("GameViewportClientClassName"));
	const FString ExpectedClassPath(TEXT("/Script/ViewportManager.VMGameViewportClient"));

	const FString ProjectDefaultEngine = FConfigCacheIni::NormalizeConfigIniPath(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("DefaultEngine.ini")));

	auto ApplyToConfig = [&](const FString& IniPath)
	{
		if (!IniPath.IsEmpty())
		{
			GConfig->LoadFile(IniPath);
			GConfig->RemoveKey(*Section, *Key, IniPath);
			GConfig->SetString(*Section, *Key, *ExpectedClassPath, IniPath);
			GConfig->Flush(false, IniPath);
			UE_LOG(LogViewportManager, Log, TEXT("Set %s=%s in %s"), *Key, *ExpectedClassPath, *IniPath);
		}
	};

	ApplyToConfig(GEngineIni);
	ApplyToConfig(ProjectDefaultEngine);

	InstallViewportClient();

	CloseViewportClientSetupDialog();

#if WITH_EDITOR
	bool bConfigMatches = false;
	bool bDefaultConfigMatches = false;
	{
		FString CurrentValue;
		if (GConfig->GetString(*Section, *Key, CurrentValue, GEngineIni))
		{
			bConfigMatches = CurrentValue.Equals(ExpectedClassPath, ESearchCase::IgnoreCase);
		}
		if (GConfig->GetString(*Section, *Key, CurrentValue, ProjectDefaultEngine))
		{
			bDefaultConfigMatches = CurrentValue.Equals(ExpectedClassPath, ESearchCase::IgnoreCase);
		}
	}

	FNotificationInfo Info(
		(bConfigMatches && bDefaultConfigMatches)
		? LOCTEXT("ViewportManagerFixApplied", "Viewport Manager applied the viewport client override. Restart the editor if the change doesn’t appear immediately.")
		: LOCTEXT("ViewportManagerFixFailed", "Viewport Manager could not persist the viewport client override. Please set it manually in Project Settings → Engine → General Settings.")
	);
	Info.ExpireDuration = 5.0f;
	Info.FadeOutDuration = 0.6f;
	if (TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info))
	{
		Notification->SetCompletionState((bConfigMatches && bDefaultConfigMatches) ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
	}

	if (!bConfigMatches || !bDefaultConfigMatches)
	{
		UE_LOG(LogViewportManager, Warning, TEXT("Failed to verify %s value in %s or %s after applying fix."), *Key, *GEngineIni, *ProjectDefaultEngine);
		return;
	}
#endif

	bSuppressViewportClientPromptForSession = true;
}

void FViewportManagerModule::HandleOpenProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->ShowViewer("Project", "Engine", "GeneralSettings");
	}
}

void FViewportManagerModule::HandleDismissViewportClientPrompt()
{
	CloseViewportClientSetupDialog();
	bSuppressViewportClientPromptForSession = true;
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FViewportManagerModule, ViewportManager)
