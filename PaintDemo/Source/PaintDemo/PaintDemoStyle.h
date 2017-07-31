// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SlateBasics.h"
#include "SlateExtras.h"


class PAINTDEMO_API FPaintDemoStyle 
{

public:
    static void Initialize();
    static void Shutdown();

    static void ReloadTextures();
    static const ISlateStyle &Get();

    static FName GetStyleSetName();

private:
    static TSharedRef<FSlateStyleSet> Create();

    static TSharedPtr<FSlateStyleSet> Instance;
};
