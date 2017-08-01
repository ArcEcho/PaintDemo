// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPaintDemoSurface.h"

class SPaintDemoRuler;
class SPaintDemoZoomPan;
class SBox;

/**
 *
 */

class PAINTDEMO_API SPaintDemoView : public SPaintDemoSurface
{
public:
    SLATE_BEGIN_ARGS(SPaintDemoView)
    {}
    SLATE_END_ARGS()

public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
    
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

    EVisibility GetRulerVisibility();

    FGeometry MakeGeometryWindowLocal(const FGeometry& WidgetGeometry) const;

    void PopulateWidgetGeometryCache(FArrangedWidget& Root);

    /** The width of the preview screen for the UI */
    FOptionalSize GetPreviewAreaWidth() const;

    /** The height of the preview screen for the UI */
    FOptionalSize GetPreviewAreaHeight() const;

private:
    /** The ruler bar at the top of the designer. */
    TSharedPtr<SPaintDemoRuler> TopRuler;

    /** The ruler bar on the left side of the designer. */
    TSharedPtr<SPaintDemoRuler> SideRuler;

    TSharedPtr<SPaintDemoZoomPan> PreviewHitTestRoot;

    TSharedPtr<SBox> PreviewAreaConstraint;

    TMap<TSharedRef<SWidget>, FArrangedWidget> CachedWidgetGeometry;
};
