// Fill out your copyright notice in the Description page of Project Settings.

#include "MyUserWidget.h"
#include "SPaintDemoView.h"



UMyUserWidget::UMyUserWidget()
    : PaintDemoView{nullptr}
{

}

TSharedRef<SWidget> UMyUserWidget::RebuildWidget()
{
    PaintDemoView = SNew(SPaintDemoView);

    return PaintDemoView.ToSharedRef();

}
