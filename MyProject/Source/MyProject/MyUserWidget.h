// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget.h"
#include "MyUserWidget.generated.h"

class SPaintDemoWidget;

/**
 * 
 */
UCLASS()
class  UMyUserWidget : public UWidget
{
	GENERATED_BODY()
	
public:
    UMyUserWidget();


protected:
    TSharedPtr<SPaintDemoWidget> PaintDemoWidget;
    virtual TSharedRef<SWidget> RebuildWidget() override;
	
};
