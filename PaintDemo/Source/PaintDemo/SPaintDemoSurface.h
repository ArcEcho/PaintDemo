// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Animation/CurveSequence.h"

/**
* Interface for ZoomLevel values
* Provides mapping for a range of virtual ZoomLevel values to actual node scaling values
*/
struct FZoomLevelsContainer
{
    /**
    * @param InZoomLevel virtual zoom level value
    *
    * @return associated scaling value
    */
    virtual float						GetZoomAmount(int32 InZoomLevel) const = 0;

    /**
    * @param InZoomAmount scaling value
    *
    * @return nearest ZoomLevel mapping for provided scale value
    */
    virtual int32						GetNearestZoomLevel(float InZoomAmount) const = 0;

    /**
    * @param InZoomLevel virtual zoom level value
    *
    * @return associated friendly name
    */
    virtual FText						GetZoomText(int32 InZoomLevel) const = 0;

    /**
    * @return count of supported zoom levels
    */
    virtual int32						GetNumZoomLevels() const = 0;

    /**
    * @return the optimal(1:1) zoom level value, default zoom level for the graph
    */
    virtual int32						GetDefaultZoomLevel() const = 0;

    // Necessary for Mac OS X to compile 'delete <pointer_to_this_object>;'
    virtual ~FZoomLevelsContainer(void) {};
};

/**
 *
 */
class  SPaintDemoSurface : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SPaintDemoSurface)
        : _AllowContinousZoomInterpolation(false)
    {}
    /** Slot for this designers content (optional) */
    SLATE_DEFAULT_SLOT(FArguments, Content)
        SLATE_ATTRIBUTE(bool, AllowContinousZoomInterpolation)
        SLATE_END_ARGS()

public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);

    // SWidget interface
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
    virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual bool SupportsKeyboardFocus() const override;
    // End of Swidget interface

protected:
    void PaintBackgroundAsLines(const FSlateBrush* BackgroundImage, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32& DrawLayerId) const;

    /** Gets the current zoom factor. */
    float GetZoomAmount() const;

    void ChangeZoomLevel(int32 ZoomLevelDelta, const FVector2D& WidgetSpaceZoomOrigin, bool bOverrideZoomLimiting);

    void PostChangedZoom();

    bool ScrollToLocation(const FGeometry& MyGeometry, FVector2D DesiredCenterPosition, const float InDeltaTime);

    bool ZoomToLocation(const FVector2D& CurrentSizeWithoutZoom, const FVector2D& InDesiredSize, bool bDoneScrolling);

    void ZoomToFit(bool bInstantZoom);

    FText GetZoomText() const;
    FSlateColor GetZoomTextColorAndOpacity() const;

    FVector2D GetViewOffset() const;

    FSlateRect ComputeSensibleBounds() const;

    FVector2D GraphCoordToPanelCoord(const FVector2D& GraphSpaceCoordinate) const;
    FVector2D PanelCoordToGraphCoord(const FVector2D& PanelSpaceCoordinate) const;

protected:
    virtual FSlateRect ComputeAreaBounds() const;
    virtual float GetGridScaleAmount() const;
    virtual int32 GetGraphRulePeriod() const;
    virtual int32 GetSnapGridSize() const;

protected:
    /** The position within the graph at which the user is looking */
    FVector2D ViewOffset;

    /** Previous Zoom Level */
    int32 PreviousZoomLevel;

    /** How zoomed in/out we are. e.g. 0.25f results in quarter-sized nodes. */
    int32 ZoomLevel;

    /** Are we panning the view at the moment? */
    bool bIsPanning;

    /** Allow continuous zoom interpolation? */
    TAttribute<bool> AllowContinousZoomInterpolation;

    /** Fade on zoom for graph */
    FCurveSequence ZoomLevelGraphFade;

    /** Curve that handles fading the 'Zoom +X' text */
    FCurveSequence ZoomLevelFade;

    // The interface for mapping ZoomLevel values to actual node scaling values
    TUniquePtr<FZoomLevelsContainer> ZoomLevels;

    bool bAllowContinousZoomInterpolation;

    bool bTeleportInsteadOfScrollingWhenZoomingToFit;

    FVector2D ZoomTargetTopLeft;
    FVector2D ZoomTargetBottomRight;
    FVector2D ZoomToFitPadding;

    /** The Y component of mouse drag (used when zooming) */
    float TotalMouseDeltaY;

    /** Offset in the panel the user started the LMB+RMB zoom from */
    FVector2D ZoomStartOffset;

    /**  */
    FVector2D ViewOffsetStart;

    /**  */
    FVector2D MouseDownPositionAbsolute;

    /** Cumulative magnify delta from trackpad gesture */
    float TotalGestureMagnify;

    /** Does the user need to press Control in order to over-zoom. */
    bool bRequireControlToOverZoom;

    /** Cached geometry for use within the active timer */
    FGeometry CachedGeometry;

private:
    /** Active timer that handles deferred zooming until the target zoom is reached */
    EActiveTimerReturnType HandleZoomToFit(double InCurrentTime, float InDeltaTime);

    /** The handle to the active timer */
    TWeakPtr<FActiveTimerHandle> ActiveTimerHandle;

    // A flag noting if we have a pending zoom to extents operation to perform next tick.
    bool bDeferredZoomToExtents;
};
