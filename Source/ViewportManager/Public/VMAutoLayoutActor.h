// Copyright jackcayc924 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VMSplitLayoutAsset.h"
#include "VMAutoLayoutActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class VIEWPORTMANAGER_API AVMAutoLayoutActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AVMAutoLayoutActor();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewport Manager")
	TObjectPtr<UVMSplitLayoutAsset> LayoutAsset;

	// Delay before applying layout (to ensure everything is initialized)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Viewport Manager")
	float ApplyDelay = 0.5f;

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	void ApplyLayout();

	UFUNCTION(BlueprintCallable, Category = "Viewport Manager")
	void TestSetup();

protected:
	FTimerHandle ApplyLayoutTimerHandle;
};


