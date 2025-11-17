#include "VMHUDRootWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "VMLog.h"

void UVMHUDRootWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureRootCanvas();
}

UCanvasPanel* UVMHUDRootWidget::GetRootCanvas()
{
	EnsureRootCanvas();
	return RootCanvas;
}

void UVMHUDRootWidget::EnsureRootCanvas()
{
	if (RootCanvas)
		return;

	if (WidgetTree && WidgetTree->RootWidget)
	{
		RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
		if (RootCanvas)
		{
			UE_LOG(LogViewportManager, Log, TEXT("UVMHUDRootWidget::EnsureRootCanvas - Found existing Canvas Panel from Blueprint"));
			return;
		}
	}

	if (WidgetTree)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
		
		UE_LOG(LogViewportManager, Log, TEXT("UVMHUDRootWidget::EnsureRootCanvas - Created root canvas programmatically"));
	}
	else
	{
		UE_LOG(LogViewportManager, Error, TEXT("UVMHUDRootWidget::EnsureRootCanvas - WidgetTree is null!"));
	}
}



