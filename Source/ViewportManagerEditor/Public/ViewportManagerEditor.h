#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FViewportManagerEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterAssetTypeActions();
	void UnregisterAssetTypeActions();
	void RegisterMenus();
	void UnregisterMenus();
	void ExtendMenus();
	void OpenLayoutDesigner();
	void OpenQuickStartWizard();
	void SetupLegacyInput();

	TArray<TSharedPtr<class IAssetTypeActions>> CreatedAssetTypeActions;
	FDelegateHandle MenuRegistrationHandle;
	TWeakPtr<class SWindow> LayoutDesignerWindow;
	TWeakPtr<class SWindow> QuickStartWindow;

	static const FName LayoutDesignerMenuEntryName;
	static const FName QuickStartMenuEntryName;
	static const FName SetupInputMenuEntryName;
};



