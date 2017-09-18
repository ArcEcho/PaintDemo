// Fill out your copyright notice in the Description page of Project Settings.

#include "MyUserWidget.h"
#include "SPaintDemoWidget.h"


UMyUserWidget::UMyUserWidget()
    : PaintDemoWidget{nullptr}
{

}

TSharedRef<SWidget> UMyUserWidget::RebuildWidget()
{
    PaintDemoWidget = SNew(SPaintDemoWidget);

    return PaintDemoWidget.ToSharedRef();

}
