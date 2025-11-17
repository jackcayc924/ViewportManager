#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VMSplitLayoutAsset.h"
#include "VMSplitSubsystem.generated.h"

UCLASS(BlueprintType)
class VIEWPORTMANAGER_API UVMSplitSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	void ApplyLayout(UVMSplitLayoutAsset* Layout);

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	void SetPaneRect(int32 LocalPlayerIndex, float OriginX, float OriginY, float SizeX, float SizeY);

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	bool GetPaneRect(int32 LocalPlayerIndex, FVector2f& OutOrigin01, FVector2f& OutSize01);

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	void SetPanePawnClass(int32 LocalPlayerIndex, TSubclassOf<APawn> PawnClass);

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	void SetActiveKeyboardMousePlayer(int32 LocalPlayerIndex);

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	int32 GetActiveKeyboardMousePlayer() const;

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	APlayerController* GetPlayerController(int32 LocalPlayerIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	int32 GetLocalPlayerCount() const;

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	bool IsVMGameViewportClientActive() const;

protected:
	UPROPERTY()
	TObjectPtr<UVMSplitLayoutAsset> CurrentLayout;

	class UVMGameViewportClient* GetViewportClient() const;

	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS);

	void LoadDefaultLayout();
};


