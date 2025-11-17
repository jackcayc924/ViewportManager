#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AssetTypeActions_Base.h"
#include "VMSplitLayoutAsset.h"
#include "VMSplitLayoutFactory.generated.h"

UCLASS()
class VIEWPORTMANAGEREDITOR_API UVMSplitLayoutFactory : public UFactory
{
	GENERATED_BODY()

public:
	UVMSplitLayoutFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};

class VIEWPORTMANAGEREDITOR_API FAssetTypeActions_VMSplitLayout : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor) override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
};

