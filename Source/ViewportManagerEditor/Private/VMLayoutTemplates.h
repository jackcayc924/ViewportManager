#pragma once

#include "CoreMinimal.h"
#include "VMSplitLayoutAsset.h"

struct FVMLayoutTemplate
{
	FText Name;
	FText Description;
	TArray<FVMSplitPane> Panes;

	FVMLayoutTemplate(FText InName, FText InDescription)
		: Name(InName), Description(InDescription)
	{}
};

class FVMLayoutTemplateLibrary
{
public:
	static TArray<FVMLayoutTemplate> GetAllTemplates();

private:
	static FVMLayoutTemplate CreateSingleFullscreen();
	static FVMLayoutTemplate CreateTwoPlayerHorizontal();
	static FVMLayoutTemplate CreateTwoPlayerVertical();
	static FVMLayoutTemplate CreateFourPlayerGrid();
	static FVMLayoutTemplate CreatePictureInPicture();
	static FVMLayoutTemplate CreateThreePlayerAsymmetric();
	static FVMLayoutTemplate CreateSixPlayerGrid();
};
