// Fill out your copyright notice in the Description page of Project Settings.

#include "PaintDemoGameModeBase.h"

#include "PaintDemoStyle.h"
#include "SPaintDemoView.h"


void APaintDemoGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    Widget =
        SNew(SOverlay)
        + SOverlay::Slot()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        [
            SNew(SPaintDemoView)
        ];
    GEngine->GameViewport->AddViewportWidgetForPlayer(GetWorld()->GetFirstLocalPlayerFromController(), Widget.ToSharedRef(), 1);

    auto FirstPlayerController = GetWorld()->GetFirstPlayerController();
    FirstPlayerController->SetInputMode(FInputModeUIOnly());
    FirstPlayerController->bShowMouseCursor = true;
}
