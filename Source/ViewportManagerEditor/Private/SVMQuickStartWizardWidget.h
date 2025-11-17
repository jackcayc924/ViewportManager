#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class AVMAutoLayoutActor;

class SVMQuickStartWizardWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVMQuickStartWizardWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// Validation checks
	bool IsGameViewportClientConfigured() const;
	bool IsGameInstanceConfigured() const;
	bool IsGameModeConfigured() const;
	bool IsCameraInputConfigured() const;
	bool IsMaxPlayersConfigured() const;
	bool AreAllSystemChecksValid() const;

	// Status text getters
	FText GetGameViewportClientStatus() const;
	FText GetGameInstanceStatus() const;
	FText GetGameModeStatus() const;
	FText GetCameraInputStatus() const;
	FText GetMaxPlayersStatus() const;
	FText GetOverallStatus() const;

	// Fix button handlers
	FReply FixGameViewportClient();
	FReply FixGameInstance();
	FReply FixGameMode();
	FReply FixCameraInput();
	FReply FixMaxPlayers();
	FReply FixAllIssues();

	// Utility
	void RefreshConfigFile(const FString& ConfigPath);
	bool UpdateConfigValue(const FString& ConfigPath, const FString& Section, const FString& Key, const FString& Value);

	// Legacy handlers
	FReply HandleOpenProjectSettings();
	FReply HandleOpenDocumentation();

	// Timer for auto-refresh
	float TimeSinceLastCheck = 0.f;
	static constexpr float CheckInterval = 1.0f;
};
