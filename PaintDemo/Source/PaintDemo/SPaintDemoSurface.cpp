// Fill out your copyright notice in the Description page of Project Settings.

#include "SPaintDemoSurface.h"
#include "SlateOptMacros.h"
#include "PaintDemoStyle.h"

#define LOCTEXT_NAMESPACE "PaintDemo"

struct FZoomLevelEntry
{
public:
    FZoomLevelEntry(float InZoomAmount, const FText& InDisplayText)
        : DisplayText(FText::Format(LOCTEXT("Zoom", "Zoom {0}"), InDisplayText))
        , ZoomAmount(InZoomAmount)
    {
    }

public:
    FText DisplayText;
    float ZoomAmount;
};

struct FFixedZoomLevelsContainer : public FZoomLevelsContainer
{
    FFixedZoomLevelsContainer()
    {
        ZoomLevels.Reserve(26);
        ZoomLevels.Add(FZoomLevelEntry(0.150f, FText::FromString("-10")));
        ZoomLevels.Add(FZoomLevelEntry(0.175f, FText::FromString("-9")));
        ZoomLevels.Add(FZoomLevelEntry(0.200f, FText::FromString("-8")));
        ZoomLevels.Add(FZoomLevelEntry(0.225f, FText::FromString("-7")));
        ZoomLevels.Add(FZoomLevelEntry(0.250f, FText::FromString("-6")));
        ZoomLevels.Add(FZoomLevelEntry(0.375f, FText::FromString("-5")));
        ZoomLevels.Add(FZoomLevelEntry(0.500f, FText::FromString("-4")));
        ZoomLevels.Add(FZoomLevelEntry(0.675f, FText::FromString("-3")));
        ZoomLevels.Add(FZoomLevelEntry(0.750f, FText::FromString("-2")));
        ZoomLevels.Add(FZoomLevelEntry(0.875f, FText::FromString("-1")));
        ZoomLevels.Add(FZoomLevelEntry(1.000f, FText::FromString("1:1")));
        ZoomLevels.Add(FZoomLevelEntry(1.250f, FText::FromString("+1")));
        ZoomLevels.Add(FZoomLevelEntry(1.500f, FText::FromString("+2")));
        ZoomLevels.Add(FZoomLevelEntry(1.750f, FText::FromString("+3")));
        ZoomLevels.Add(FZoomLevelEntry(2.000f, FText::FromString("+4")));
        ZoomLevels.Add(FZoomLevelEntry(2.250f, FText::FromString("+5")));
        ZoomLevels.Add(FZoomLevelEntry(2.500f, FText::FromString("+6")));
        ZoomLevels.Add(FZoomLevelEntry(2.750f, FText::FromString("+7")));
        ZoomLevels.Add(FZoomLevelEntry(3.000f, FText::FromString("+8")));
        ZoomLevels.Add(FZoomLevelEntry(3.250f, FText::FromString("+9")));
        ZoomLevels.Add(FZoomLevelEntry(3.500f, FText::FromString("+10")));
        ZoomLevels.Add(FZoomLevelEntry(4.000f, FText::FromString("+11")));
        ZoomLevels.Add(FZoomLevelEntry(5.000f, FText::FromString("+12")));
        ZoomLevels.Add(FZoomLevelEntry(6.000f, FText::FromString("+13")));
        ZoomLevels.Add(FZoomLevelEntry(7.000f, FText::FromString("+14")));
        ZoomLevels.Add(FZoomLevelEntry(8.000f, FText::FromString("+15")));
    }

    float GetZoomAmount(int32 InZoomLevel) const
    {
        checkSlow(ZoomLevels.IsValidIndex(InZoomLevel));
        return ZoomLevels[InZoomLevel].ZoomAmount;
    }

    int32 GetNearestZoomLevel(float InZoomAmount) const
    {
        for (int32 ZoomLevelIndex = 0; ZoomLevelIndex < GetNumZoomLevels(); ++ZoomLevelIndex)
        {
            if (InZoomAmount <= GetZoomAmount(ZoomLevelIndex))
            {
                return ZoomLevelIndex;
            }
        }

        return GetDefaultZoomLevel();
    }

    FText GetZoomText(int32 InZoomLevel) const
    {
        checkSlow(ZoomLevels.IsValidIndex(InZoomLevel));
        return ZoomLevels[InZoomLevel].DisplayText;
    }

    int32 GetNumZoomLevels() const
    {
        return ZoomLevels.Num();
    }

    int32 GetDefaultZoomLevel() const
    {
        return 10;
    }

    TArray<FZoomLevelEntry> ZoomLevels;
};


/////////////////////////////////////////////////////
// SPaintDemoSurface

void SPaintDemoSurface::Construct(const FArguments& InArgs)
{
    if (!ZoomLevels)
    {
        ZoomLevels = MakeUnique<FFixedZoomLevelsContainer>();
    }
    ZoomLevel = ZoomLevels->GetDefaultZoomLevel();
    PreviousZoomLevel = ZoomLevels->GetDefaultZoomLevel();
    PostChangedZoom();
    AllowContinousZoomInterpolation = InArgs._AllowContinousZoomInterpolation;
    bIsPanning = false;

    ViewOffset = FVector2D::ZeroVector;

    ZoomLevelFade = FCurveSequence(0.0f, 1.0f);
    ZoomLevelFade.Play(this->AsShared());

    ZoomLevelGraphFade = FCurveSequence(0.0f, 0.5f);
    ZoomLevelGraphFade.Play(this->AsShared());

    bDeferredZoomToExtents = false;

    bAllowContinousZoomInterpolation = false;
    bTeleportInsteadOfScrollingWhenZoomingToFit = false;

    bRequireControlToOverZoom = false;

    ZoomTargetTopLeft = FVector2D::ZeroVector;
    ZoomTargetBottomRight = FVector2D::ZeroVector;

    ZoomToFitPadding = FVector2D(100, 100);
    TotalGestureMagnify = 0.0f;

    TotalMouseDeltaY = 0.0f;
    ZoomStartOffset = FVector2D::ZeroVector;

    ChildSlot
        [
            InArgs._Content.Widget
        ];
}

EActiveTimerReturnType SPaintDemoSurface::HandleZoomToFit(double InCurrentTime, float InDeltaTime)
{
    const FVector2D DesiredViewCenter = (ZoomTargetTopLeft + ZoomTargetBottomRight) * 0.5f;
    const bool bDoneScrolling = ScrollToLocation(CachedGeometry, DesiredViewCenter, bTeleportInsteadOfScrollingWhenZoomingToFit ? 1000.0f : InDeltaTime);
    const bool bDoneZooming = ZoomToLocation(CachedGeometry.Size, ZoomTargetBottomRight - ZoomTargetTopLeft, bDoneScrolling);

    if (bDoneZooming && bDoneScrolling)
    {
        // One final push to make sure we're centered in the end
        ViewOffset = DesiredViewCenter - (0.5f * CachedGeometry.Scale * CachedGeometry.Size / GetZoomAmount());

        ZoomTargetTopLeft = FVector2D::ZeroVector;
        ZoomTargetBottomRight = FVector2D::ZeroVector;

        return EActiveTimerReturnType::Stop;
    }

    return EActiveTimerReturnType::Continue;
}

void SPaintDemoSurface::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    CachedGeometry = AllottedGeometry;

    // Zoom to extents
    FSlateRect Bounds = ComputeAreaBounds();
    if (bDeferredZoomToExtents)
    {
        bDeferredZoomToExtents = false;
        ZoomTargetTopLeft = FVector2D(Bounds.Left, Bounds.Top);
        ZoomTargetBottomRight = FVector2D(Bounds.Right, Bounds.Bottom);

        if (!ActiveTimerHandle.IsValid())
        {
            RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SPaintDemoSurface::HandleZoomToFit));
        }
    }
}

FCursorReply SPaintDemoSurface::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
    if (bIsPanning)
    {
        return FCursorReply::Cursor(EMouseCursor::GrabHand);
    }

    return SCompoundWidget::OnCursorQuery(MyGeometry, CursorEvent);
}

int32 SPaintDemoSurface::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const FSlateBrush* BackgroundImage = FPaintDemoStyle::Get().GetBrush(TEXT("SolidBackground"));
    PaintBackgroundAsLines(BackgroundImage, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId);

    SCompoundWidget::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

    return LayerId;
}

bool SPaintDemoSurface::SupportsKeyboardFocus() const
{
    return true;
}

FReply SPaintDemoSurface::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);

    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        bIsPanning = false;

        ViewOffsetStart = ViewOffset;
        MouseDownPositionAbsolute = MouseEvent.GetLastScreenSpacePosition();
    }

    if (FSlateApplication::Get().IsUsingTrackpad())
    {
        TotalMouseDeltaY = 0.0f;
        ZoomStartOffset = MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition());
    }

    return FReply::Unhandled();
}

FReply SPaintDemoSurface::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    SCompoundWidget::OnMouseButtonUp(MyGeometry, MouseEvent);

    if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        bIsPanning = false;
    }

    return FReply::Unhandled();
}

FReply SPaintDemoSurface::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    const bool bIsRightMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);
    const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);
    const FModifierKeysState ModifierKeysState = FSlateApplication::Get().GetModifierKeys();

    if (HasMouseCapture() && bIsRightMouseButtonDown)
    {
        FReply ReplyState = FReply::Handled();

        bIsPanning = true;
        ViewOffset = ViewOffsetStart + ((MouseDownPositionAbsolute - MouseEvent.GetScreenSpacePosition()) / MyGeometry.Scale) / GetZoomAmount();

        return ReplyState;
    }

    return FReply::Unhandled();
}

FReply SPaintDemoSurface::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    // We want to zoom into this point; i.e. keep it the same fraction offset into the panel
    const FVector2D WidgetSpaceCursorPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
    const int32 ZoomLevelDelta = FMath::FloorToInt(MouseEvent.GetWheelDelta());
    ChangeZoomLevel(ZoomLevelDelta, WidgetSpaceCursorPos, !bRequireControlToOverZoom || MouseEvent.IsControlDown());

    return FReply::Handled();
}

inline float FancyMod(float Value, float Size)
{
    return ((Value >= 0) ? 0.0f : Size) + FMath::Fmod(Value, Size);
}

float SPaintDemoSurface::GetZoomAmount() const
{
    if (AllowContinousZoomInterpolation.Get())
    {
        return FMath::Lerp(ZoomLevels->GetZoomAmount(PreviousZoomLevel), ZoomLevels->GetZoomAmount(ZoomLevel), ZoomLevelGraphFade.GetLerp());
    }
    else
    {
        return ZoomLevels->GetZoomAmount(ZoomLevel);
    }
}

void SPaintDemoSurface::ChangeZoomLevel(int32 ZoomLevelDelta, const FVector2D& WidgetSpaceZoomOrigin, bool bOverrideZoomLimiting)
{
    // We want to zoom into this point; i.e. keep it the same fraction offset into the panel
    const FVector2D PointToMaintainGraphSpace = PanelCoordToGraphCoord(WidgetSpaceZoomOrigin);

    const int32 DefaultZoomLevel = ZoomLevels->GetDefaultZoomLevel();
    const int32 NumZoomLevels = ZoomLevels->GetNumZoomLevels();

    const bool bAllowFullZoomRange =
        // To zoom in past 1:1 the user must press control
        (ZoomLevel == DefaultZoomLevel && ZoomLevelDelta > 0 && bOverrideZoomLimiting) ||
        // If they are already zoomed in past 1:1, user may zoom freely
        (ZoomLevel > DefaultZoomLevel);

    const float OldZoomLevel = ZoomLevel;

    if (bAllowFullZoomRange)
    {
        ZoomLevel = FMath::Clamp(ZoomLevel + ZoomLevelDelta, 0, NumZoomLevels - 1);
    }
    else
    {
        // Without control, we do not allow zooming in past 1:1.
        ZoomLevel = FMath::Clamp(ZoomLevel + ZoomLevelDelta, 0, DefaultZoomLevel);
    }

    if (OldZoomLevel != ZoomLevel)
    {
        PostChangedZoom();

        // Note: This happens even when maxed out at a stop; so the user sees the animation and knows that they're at max zoom in/out
        ZoomLevelFade.Play(this->AsShared());

        // Re-center the screen so that it feels like zooming around the cursor.
        {
            FSlateRect GraphBounds = ComputeSensibleBounds();

            // Make sure we are not zooming into/out into emptiness; otherwise the user will get lost..
            const FVector2D ClampedPointToMaintainGraphSpace(
                FMath::Clamp(PointToMaintainGraphSpace.X, GraphBounds.Left, GraphBounds.Right),
                FMath::Clamp(PointToMaintainGraphSpace.Y, GraphBounds.Top, GraphBounds.Bottom)
            );

            this->ViewOffset = ClampedPointToMaintainGraphSpace - WidgetSpaceZoomOrigin / GetZoomAmount();
        }
    }
}

FSlateRect SPaintDemoSurface::ComputeSensibleBounds() const
{
    // Pad it out in every direction, to roughly account for nodes being of non-zero extent
    const float Padding = 100.0f;

    FSlateRect Bounds = ComputeAreaBounds();
    Bounds.Left -= Padding;
    Bounds.Top -= Padding;
    Bounds.Right -= Padding;
    Bounds.Bottom -= Padding;

    return Bounds;
}

void SPaintDemoSurface::PostChangedZoom()
{
}

bool SPaintDemoSurface::ScrollToLocation(const FGeometry& MyGeometry, FVector2D DesiredCenterPosition, const float InDeltaTime)
{
    const FVector2D HalfOFScreenInGraphSpace = 0.5f * MyGeometry.Size / GetZoomAmount();
    FVector2D CurrentPosition = ViewOffset + HalfOFScreenInGraphSpace;

    FVector2D NewPosition = FMath::Vector2DInterpTo(CurrentPosition, DesiredCenterPosition, InDeltaTime, 10.f);
    ViewOffset = NewPosition - HalfOFScreenInGraphSpace;

    // If within 1 pixel of target, stop interpolating
    return ((NewPosition - DesiredCenterPosition).SizeSquared() < 1.f);
}

bool SPaintDemoSurface::ZoomToLocation(const FVector2D& CurrentSizeWithoutZoom, const FVector2D& InDesiredSize, bool bDoneScrolling)
{
    if (bAllowContinousZoomInterpolation && ZoomLevelGraphFade.IsPlaying())
    {
        return false;
    }

    const int32 DefaultZoomLevel = ZoomLevels->GetDefaultZoomLevel();
    const int32 NumZoomLevels = ZoomLevels->GetNumZoomLevels();
    int32 DesiredZoom = DefaultZoomLevel;

    // Find lowest zoom level that will display all nodes
    for (int32 Zoom = 0; Zoom < DefaultZoomLevel; ++Zoom)
    {
        const FVector2D SizeWithZoom = (CurrentSizeWithoutZoom - ZoomToFitPadding) / ZoomLevels->GetZoomAmount(Zoom);
        const FVector2D LeftOverSize = SizeWithZoom - InDesiredSize;

        if ((InDesiredSize.X > SizeWithZoom.X) || (InDesiredSize.Y > SizeWithZoom.Y))
        {
            // Use the previous zoom level, this one is too tight
            DesiredZoom = FMath::Max<int32>(0, Zoom - 1);
            break;
        }
    }

    if (DesiredZoom != ZoomLevel)
    {
        if (bAllowContinousZoomInterpolation)
        {
            // Animate to it
            PreviousZoomLevel = ZoomLevel;
            ZoomLevel = FMath::Clamp(DesiredZoom, 0, NumZoomLevels - 1);
            ZoomLevelGraphFade.Play(this->AsShared());
            return false;
        }
        else
        {
            // Do it instantly, either first or last
            if (DesiredZoom < ZoomLevel)
            {
                // Zooming out; do it instantly
                ZoomLevel = PreviousZoomLevel = DesiredZoom;
                ZoomLevelFade.Play(this->AsShared());
            }
            else
            {
                // Zooming in; do it last
                if (bDoneScrolling)
                {
                    ZoomLevel = PreviousZoomLevel = DesiredZoom;
                    ZoomLevelFade.Play(this->AsShared());
                }
            }
        }

        PostChangedZoom();
    }

    return true;
}

void SPaintDemoSurface::ZoomToFit(bool bInstantZoom)
{
    bTeleportInsteadOfScrollingWhenZoomingToFit = bInstantZoom;
    bDeferredZoomToExtents = true;
}

FText SPaintDemoSurface::GetZoomText() const
{
    return ZoomLevels->GetZoomText(ZoomLevel);
}

FSlateColor SPaintDemoSurface::GetZoomTextColorAndOpacity() const
{
    return FLinearColor(1, 1, 1, 1.25f - ZoomLevelFade.GetLerp());
}

FSlateRect SPaintDemoSurface::ComputeAreaBounds() const
{
    return FSlateRect(0, 0, 0, 0);
}

FVector2D SPaintDemoSurface::GetViewOffset() const
{
    return ViewOffset;
}

FVector2D SPaintDemoSurface::GraphCoordToPanelCoord(const FVector2D& GraphSpaceCoordinate) const
{
    return (GraphSpaceCoordinate - GetViewOffset()) * GetZoomAmount();
}

FVector2D SPaintDemoSurface::PanelCoordToGraphCoord(const FVector2D& PanelSpaceCoordinate) const
{
    return PanelSpaceCoordinate / GetZoomAmount() + GetViewOffset();
}

int32 SPaintDemoSurface::GetGraphRulePeriod() const
{
    return (int32)FPaintDemoStyle::Get().GetFloat("GridRulePeriod");
}

int32 SPaintDemoSurface::GetSnapGridSize() const
{
    return 16;
}

float SPaintDemoSurface::GetGridScaleAmount() const
{
    return 1;
}

void SPaintDemoSurface::PaintBackgroundAsLines(const FSlateBrush* BackgroundImage, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32& DrawLayerId) const
{
    const bool bAntialias = false;

    const int32 RulePeriod = GetGraphRulePeriod();
    check(RulePeriod > 0);

    const FLinearColor RegularColor(FPaintDemoStyle::Get().GetColor("GridLineColor"));
    const FLinearColor RuleColor(FPaintDemoStyle::Get().GetColor("GridRuleColor"));
    const FLinearColor CenterColor(FPaintDemoStyle::Get().GetColor("GridCenterColor"));
    const float GraphSmallestGridSize = 8.0f;
    const float RawZoomFactor = GetZoomAmount();
    const float NominalGridSize = GetSnapGridSize() * GetGridScaleAmount();

    float ZoomFactor = RawZoomFactor;
    float Inflation = 1.0f;
    while (ZoomFactor*Inflation*NominalGridSize <= GraphSmallestGridSize)
    {
        Inflation *= 2.0f;
    }

    const float GridCellSize = NominalGridSize * ZoomFactor * Inflation;

    const float GraphSpaceGridX0 = FancyMod(ViewOffset.X, Inflation * NominalGridSize * RulePeriod);
    const float GraphSpaceGridY0 = FancyMod(ViewOffset.Y, Inflation * NominalGridSize * RulePeriod);

    float ImageOffsetX = GraphSpaceGridX0 * -ZoomFactor;
    float ImageOffsetY = GraphSpaceGridY0 * -ZoomFactor;

    const FVector2D ZeroSpace = GraphCoordToPanelCoord(FVector2D::ZeroVector);

    // Fill the background
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        DrawLayerId,
        AllottedGeometry.ToPaintGeometry(),
        BackgroundImage,
        MyClippingRect
    );

    TArray<FVector2D> LinePoints;
    new (LinePoints)FVector2D(0.0f, 0.0f);
    new (LinePoints)FVector2D(0.0f, 0.0f);

    // Horizontal bars
    for (int32 GridIndex = 0; ImageOffsetY < AllottedGeometry.Size.Y; ImageOffsetY += GridCellSize, ++GridIndex)
    {
        if (ImageOffsetY >= 0.0f)
        {
            const bool bIsRuleLine = (GridIndex % RulePeriod) == 0;
            const int32 Layer = bIsRuleLine ? (DrawLayerId + 1) : DrawLayerId;

            const FLinearColor* Color = bIsRuleLine ? &RuleColor : &RegularColor;
            if (FMath::IsNearlyEqual(ZeroSpace.Y, ImageOffsetY, 1.0f))
            {
                Color = &CenterColor;
            }

            LinePoints[0] = FVector2D(0.0f, ImageOffsetY);
            LinePoints[1] = FVector2D(AllottedGeometry.Size.X, ImageOffsetY);

            FSlateDrawElement::MakeLines(
                OutDrawElements,
                Layer,
                AllottedGeometry.ToPaintGeometry(),
                LinePoints,
                MyClippingRect,
                ESlateDrawEffect::None,
                *Color,
                bAntialias);
        }
    }

    // Vertical bars
    for (int32 GridIndex = 0; ImageOffsetX < AllottedGeometry.Size.X; ImageOffsetX += GridCellSize, ++GridIndex)
    {
        if (ImageOffsetX >= 0.0f)
        {
            const bool bIsRuleLine = (GridIndex % RulePeriod) == 0;
            const int32 Layer = bIsRuleLine ? (DrawLayerId + 1) : DrawLayerId;

            const FLinearColor* Color = bIsRuleLine ? &RuleColor : &RegularColor;
            if (FMath::IsNearlyEqual(ZeroSpace.X, ImageOffsetX, 1.0f))
            {
                Color = &CenterColor;
            }

            LinePoints[0] = FVector2D(ImageOffsetX, 0.0f);
            LinePoints[1] = FVector2D(ImageOffsetX, AllottedGeometry.Size.Y);

            FSlateDrawElement::MakeLines(
                OutDrawElements,
                Layer,
                AllottedGeometry.ToPaintGeometry(),
                LinePoints,
                MyClippingRect,
                ESlateDrawEffect::None,
                *Color,
                bAntialias);
        }
    }

    DrawLayerId += 2;
}

#undef LOCTEXT_NAMESPACE