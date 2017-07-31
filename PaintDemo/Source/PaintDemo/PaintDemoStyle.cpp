// Fill out your copyright notice in the Description page of Project Settings.

#include "PaintDemoStyle.h"
#include "SlateGameResources.h"
#include "Paths.h"


TSharedPtr<FSlateStyleSet> FPaintDemoStyle::Instance = nullptr;

void FPaintDemoStyle::Initialize()
{
    if (!Instance.IsValid())
    {
        Instance = Create();
        FSlateStyleRegistry::RegisterSlateStyle(*Instance);
    }
}

void FPaintDemoStyle::Shutdown()
{
    FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);

    ensure(Instance.IsUnique());

    Instance.Reset();
}

void FPaintDemoStyle::ReloadTextures()
{
    FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
}

const ISlateStyle & FPaintDemoStyle::Get()
{
    if (!Instance.IsValid())
    {
        FPaintDemoStyle::Initialize();
    }

    return *Instance;
}

FName FPaintDemoStyle::GetStyleSetName()
{
    static FName StyleSetName(TEXT("PaintDemoStyle"));
    return StyleSetName;
}

#define IMAGE_BRUSH(RelativePath, ... ) FSlateImageBrush(FPaths::GameContentDir() / "Slate" / RelativePath +TEXT(".png"), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ... ) FSlateBoxBrush(FPaths::GameContentDir() / "Slate" / RelativePath +TEXT(".png"), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ... ) FSlateBorderBrush(FPaths::GameContentDir() / "Slate" / RelativePath +TEXT(".png"), __VA_ARGS__)
#define TTF_FONT(RelativePath, ... ) FSlateFontInfo(FPaths::GameContentDir() / "Slate" / RelativePath +TEXT(".ttf"), __VA_ARGS__)
#define OTF_FONT(RelativePath, ... ) FSlateFontInfo( FPaths::GameContentDir() / "Slate" / RelativePath +TEXT(".otf"), __VA_ARGS__)

TSharedRef<FSlateStyleSet> FPaintDemoStyle::Create()
{
    TSharedRef<FSlateStyleSet> StyleRef = FSlateGameResources::New(FPaintDemoStyle::GetStyleSetName(), "/Game/Slate", "/Game/Slate");
    FSlateStyleSet &Style = StyleRef.Get();

    Style.Set(
        "NormalButtonBrush",
        FButtonStyle().SetNormal(BOX_BRUSH("Button", FVector2D(54.0f, 54.0f), FMargin(14.0f / 54.0f)))
    );

    Style.Set("NormalButtonText",
        FTextBlockStyle(FTextBlockStyle::GetDefault())
        .SetColorAndOpacity(FSlateColor(FLinearColor(1, 1, 1, 1))));


    Style.Set("SolidBackground", new IMAGE_BRUSH("GraphPanel_SolidBackground", FVector2D(16, 16), FLinearColor::White, ESlateBrushTileType::Both));
    Style.Set("GridLineColor", FLinearColor(0.035f, 0.035f, 0.035f));
    Style.Set("GridRuleColor", FLinearColor(0.008f, 0.008f, 0.008f));
    Style.Set("GridCenterColor", FLinearColor(0.000f, 0.000f, 0.000f));

    Style.Set("GridRulePeriod", 8.0f); // should be a strictly positive integral value


    return StyleRef;


}
#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
