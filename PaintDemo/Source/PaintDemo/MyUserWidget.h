// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget.h"
#include "MyUserWidget.generated.h"

class SPaintDemoView;

/**
 * 
 */
UCLASS()
class PAINTDEMO_API UMyUserWidget : public UWidget
{
	GENERATED_BODY()
	
public:
    UMyUserWidget();


protected:
    TSharedPtr<SPaintDemoView> PaintDemoView;
    virtual TSharedRef<SWidget> RebuildWidget() override;
	
};
