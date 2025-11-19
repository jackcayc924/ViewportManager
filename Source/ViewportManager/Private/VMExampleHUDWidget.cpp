// Copyright jackcayc924 2025. All Rights Reserved.

#include "VMExampleHUDWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Engine/Engine.h"
#include "VMLog.h"

UVMExampleHUDWidget::UVMExampleHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UVMExampleHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CreateUIElements();

	if (TestButton)
	{
		TestButton->OnClicked.AddDynamic(this, &UVMExampleHUDWidget::OnTestButtonClicked);
	}

	UpdateHUDDisplay();

	UE_LOG(LogViewportManager, Log, TEXT("UVMExampleHUDWidget::NativeConstruct - Example HUD constructed for LocalPlayer %d"), GetLocalPlayerIndex());
}

void UVMExampleHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateHUDDisplay();
}

void UVMExampleHUDWidget::OnTestButtonClicked()
{
	UE_LOG(LogViewportManager, Log, TEXT("UVMExampleHUDWidget::OnTestButtonClicked - Button clicked for LocalPlayer %d"), GetLocalPlayerIndex());

	if (HealthBar)
	{
		float CurrentPercent = HealthBar->GetPercent();
		float NewPercent = FMath::Fmod(CurrentPercent + 0.25f, 1.0f);
		HealthBar->SetPercent(NewPercent);
	}
}

void UVMExampleHUDWidget::UpdateHUDDisplay()
{
	if (PlayerIndexText)
	{
		FText PlayerText = FText::FromString(FString::Printf(TEXT("Player %d"), GetLocalPlayerIndex()));
		PlayerIndexText->SetText(PlayerText);
	}

	if (ViewportInfoText)
	{
		FVMSplitRect CurrentViewportRect = GetViewportRect();
		FText InfoText = FText::FromString(FString::Printf(TEXT("Viewport: (%.2f, %.2f) [%.2fx%.2f]"), 
			CurrentViewportRect.Origin01.X, CurrentViewportRect.Origin01.Y, 
			CurrentViewportRect.Size01.X, CurrentViewportRect.Size01.Y));
		ViewportInfoText->SetText(InfoText);
	}

	if (HealthBar && GetLocalPlayerIndex() >= 0)
	{
		float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		float HealthPercent = (FMath::Sin(Time + GetLocalPlayerIndex()) + 1.0f) * 0.5f;
		HealthBar->SetPercent(HealthPercent);
	}
}

void UVMExampleHUDWidget::CreateUIElements()
{
	UPanelWidget* RootWidget = Cast<UPanelWidget>(GetRootWidget());
	if (!RootWidget)
	{
		UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
		RootWidget = RootCanvas;
	}

	if (!PlayerIndexText || !ViewportInfoText || !HealthBar || !TestButton)
	{
		UVerticalBox* MainVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainVerticalBox"));

		RootWidget->AddChild(MainVerticalBox);

		if (UCanvasPanelSlot* VBoxSlot = Cast<UCanvasPanelSlot>(MainVerticalBox->Slot))
		{
			VBoxSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			VBoxSlot->SetOffsets(FMargin(10, 10, -10, -10));
			VBoxSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		}

		if (!PlayerIndexText)
		{
			PlayerIndexText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PlayerIndexText"));
			PlayerIndexText->SetText(FText::FromString(TEXT("Player 0")));
			PlayerIndexText->SetColorAndOpacity(FLinearColor::White);
			MainVerticalBox->AddChild(PlayerIndexText);
		}

		if (!ViewportInfoText)
		{
			ViewportInfoText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ViewportInfoText"));
			ViewportInfoText->SetText(FText::FromString(TEXT("Viewport Info")));
			ViewportInfoText->SetColorAndOpacity(FLinearColor::Yellow);
			MainVerticalBox->AddChild(ViewportInfoText);
		}

		if (!HealthBar)
		{
			HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
			HealthBar->SetPercent(0.75f);
			HealthBar->SetFillColorAndOpacity(FLinearColor::Green);
			MainVerticalBox->AddChild(HealthBar);
		}

		if (!TestButton)
		{
			TestButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("TestButton"));
			MainVerticalBox->AddChild(TestButton);

			UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ButtonText"));
			ButtonText->SetText(FText::FromString(TEXT("Test Button")));
			ButtonText->SetColorAndOpacity(FLinearColor::Black);
			TestButton->AddChild(ButtonText);
		}

		UE_LOG(LogViewportManager, Log, TEXT("UVMExampleHUDWidget::CreateUIElements - Created UI elements programmatically for LocalPlayer %d"), GetLocalPlayerIndex());
	}
}



