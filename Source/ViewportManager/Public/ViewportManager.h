#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "VMLog.h"

class FDelegateHandle;

class FViewportManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void OnPostEngineInit();
	void InstallViewportClient();
#if WITH_EDITOR
	void ValidateViewportClientSetting();
	void OpenViewportClientSetupDialog(const FString& CurrentValue);
	void CloseViewportClientSetupDialog();
	void HandleApplyViewportClientSetting();
	void HandleOpenProjectSettings();
	void HandleDismissViewportClientPrompt();

	mutable TWeakPtr<class SWindow> ViewportClientSetupWindow;
	bool bSuppressViewportClientPromptForSession = false;
#endif

	FDelegateHandle PostEngineInitHandle;
};
