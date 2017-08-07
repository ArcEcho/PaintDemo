// Fill out your copyright notice in the Description page of Project Settings.

#include "SPaintDemoSurface.h"
#include "SlateOptMacros.h"
#include "PaintDemoStyle.h"
#include "UnrealMathUtility.h"

#define LOCTEXT_NAMESPACE "PaintDemo"

/////////////////////////////////////////////////////////////////////



struct FWallElement : public FFloorPlanElement
{

    int32 Id;
    FVector2D FirstAnchorPoint;
    FVector2D SecondAnchorPoint;

    //Topology
    TArray<int32>  FirstAnchorPointAdjacencyWallKeys;
    TArray<int32>  SecondAnchorPointAdjacencyWallKeys;

    //Shape
    //float Length;  //Duplicated?
    //float Height;  // Necessary?
    float Thickness;

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
    {
        //TODO now just use four lines to express a wall.

        FVector2D NominalDirection = SecondAnchorPoint - FirstAnchorPoint;
        FVector HomogeneousNominalDirection(NominalDirection, 0.0f);
        FVector HomogeneousZDirection(0.0f, 0.0f, 1.0f);
        FVector HomogeneousNormal = FVector::CrossProduct(HomogeneousNominalDirection, HomogeneousZDirection);
        FVector2D Normal;
        Normal.X = HomogeneousNormal.X;
        Normal.Y = HomogeneousNormal.Y;
        Normal.Normalize();

        FVector2D VertexA = FirstAnchorPoint + Normal * Thickness * 0.5f;
        FVector2D VertexB = FirstAnchorPoint - Normal * Thickness * 0.5f;
        FVector2D VertexC = SecondAnchorPoint - Normal * Thickness * 0.5f;
        FVector2D VertexD = SecondAnchorPoint + Normal * Thickness * 0.5f;

        TArray<FVector2D> LinePoints;
        LinePoints.Add(VertexA);
        LinePoints.Add(VertexB);
        LinePoints.Add(VertexC);
        LinePoints.Add(VertexD);
        LinePoints.Add(VertexA);

        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            LinePoints,
            MyClippingRect,
            ESlateDrawEffect::None,
            FLinearColor::Red,
            false,
            1.0f
        );


        return LayerId;
    }

};

void SPaintDemoSurface::ProduceTestWalls()
{
    {
        FWallElement WallElement;
        WallElement.FirstAnchorPoint = FVector2D(500.0f, 800.0f);
        WallElement.SecondAnchorPoint = FVector2D(800.0f, 800.0f);
        WallElement.Thickness = 50.0f;

        FloorPlanElementList.Add(MakeUnique<FWallElement>(WallElement));
    }

    {
        FWallElement WallElement;
        WallElement.FirstAnchorPoint = FVector2D(400.0f, 800.0f);
        WallElement.SecondAnchorPoint = FVector2D(800.0f, 400.0f);
        WallElement.Thickness = 10.0f;

        FloorPlanElementList.Add(MakeUnique<FWallElement>(WallElement));
    }
}
struct FWindowElement
{

};

struct FDoorElement
{

};

/////////////////////////////////////////////////////////////////////
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

    TotalMouseDeltaY = 0.0f;
    ZoomStartOffset = FVector2D::ZeroVector;

    ChildSlot
        [
            InArgs._Content.Widget
        ];

    TestLinePointA = FVector2D::ZeroVector;
    TestLinePointB = FVector2D::ZeroVector;


    bIsDrawingLine = false;

    ConstructSequence();

    ProduceTestWalls();
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

    for (auto & Line : LineList)
    {
        TArray<FVector2D> LinePoints;
        LinePoints.AddUninitialized(2);
        LinePoints[0] = GraphCoordToPanelCoord(Line.Key);
        LinePoints[1] = GraphCoordToPanelCoord(Line.Value);

        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            LinePoints,
            MyClippingRect,
            ESlateDrawEffect::None,
            FLinearColor::Red,
            true,
            5.0f
        );
    }

    {
        FVector2D LocalSize{ 100.0f, 200.0f };
        FVector2D PositionInGraphCoord{ 10.0f, 10.0f };
        LocalSize *= GetZoomAmount();
        FVector2D PositionInPanelCoord = GraphCoordToPanelCoord(PositionInGraphCoord);
        FSlateLayoutTransform LayoutTransform{ PositionInPanelCoord };
        FGeometry BoxGeo = AllottedGeometry.MakeChild(LocalSize, LayoutTransform);

        const FSlateBrush* BoxBrush = FPaintDemoStyle::Get().GetBrush(TEXT("Box"));
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            BoxGeo.ToPaintGeometry(),
            BoxBrush,
            MyClippingRect
        );
    }

    {
        FVector2D LocalSize{ 300.0f, 300.0f };
        FVector2D PositionInGraphCoord{ 500.0f, 500.0f };
        LocalSize *= GetZoomAmount();
        FVector2D PositionInPanelCoord = GraphCoordToPanelCoord(PositionInGraphCoord);
        FSlateLayoutTransform LayoutTransform{ PositionInPanelCoord };
        FGeometry BoxGeo = AllottedGeometry.MakeChild(LocalSize, LayoutTransform);

        const FSlateBrush* DashedBorderBrush = FPaintDemoStyle::Get().GetBrush(TEXT("DashedBorder"));

        FSlateDrawElement::MakeRotatedBox(
            OutDrawElements,
            LayerId,
            BoxGeo.ToPaintGeometry(),
            DashedBorderBrush,
            MyClippingRect,
            ESlateDrawEffect::None,
            45.0f
        );
    }

    {
        const float Radius = 50.0f;
        const FVector2D Center = FVector2D(100, 100);

        const FSlateBrush* MyBrush = FPaintDemoStyle::Get().GetBrush("TestCircleColor");

        FSlateShaderResourceProxy *ResourceProxy = FSlateDataPayload::ResourceManager->GetShaderResource(*MyBrush);
        FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*MyBrush);

        FVector2D UVCenter = FVector2D::ZeroVector;
        FVector2D UVRadius = FVector2D(1, 1);
        if (ResourceProxy != nullptr)
        {
            UVRadius = 0.5f*ResourceProxy->SizeUV;
            UVCenter = ResourceProxy->StartUV + UVRadius;
        }

        // Make a triangle fan in the area allotted
        const int NumTris = 12;
        TArray<FSlateVertex> Verts;
        Verts.Reserve(NumTris * 3);

        // Center Vertex
        Verts.AddZeroed();
        {
            FSlateVertex& NewVert = Verts.Last();
            NewVert.Position[0] = Center.X;
            NewVert.Position[1] = Center.Y;
            NewVert.TexCoords[0] = UVCenter.X;
            NewVert.TexCoords[1] = UVCenter.Y;
            NewVert.TexCoords[2] = NewVert.TexCoords[3] = 1.0f;
            NewVert.Color = FColor::White;
            NewVert.ClipRect = FSlateRotatedRect(MyClippingRect);
        }

        for (int i = 0; i < NumTris; ++i)
        {
            Verts.AddZeroed();
            {
                const float Angle = (2 * PI*i) / NumTris;
                const FVector2D EdgeDirection(FMath::Cos(Angle), FMath::Sin(Angle));
                const FVector2D Edge(Radius*EdgeDirection);
                FSlateVertex& NewVert = Verts.Last();
                NewVert.Position[0] = Center.X + Edge.X;
                NewVert.Position[1] = Center.Y + Edge.Y;
                NewVert.TexCoords[0] = UVCenter.X + UVRadius.X*EdgeDirection.X;
                NewVert.TexCoords[1] = UVCenter.Y + UVRadius.Y*EdgeDirection.Y;
                NewVert.TexCoords[2] = NewVert.TexCoords[3] = 1.0f;
                NewVert.Color = FColor::White;
                NewVert.ClipRect = FSlateRotatedRect(MyClippingRect);
            }
        }

        TArray<SlateIndex> Indexes;
        for (int i = 1; i <= NumTris; ++i)
        {
            Indexes.Add(0);
            Indexes.Add(i);
            Indexes.Add((i + 1 > 12) ? (1) : (i + 1));
        }

        FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, Verts, Indexes, nullptr, 0, 0);
    }

    {
        const FText Text = LOCTEXT("TestText", "The quick brown fox jumps over the lazy dog 0123456789");
        uint32 FontSize = 14;
        FSlateFontInfo FontInfo = FCoreStyle::Get().GetFontStyle("ToolTip.Font");
        FontInfo.OutlineSettings.OutlineColor = FLinearColor::Blue;
        FontInfo.OutlineSettings.OutlineSize = 2;

        FSlateDrawElement::MakeText(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(FVector2D(200.0f, 200.0f), AllottedGeometry.Size, 1.0f),
            Text.ToString(),
            FontInfo,
            MyClippingRect,
            ESlateDrawEffect::DisabledEffect,
            FColor(0, 255, 255)
        );
    }

    {
        TArray<FSlateGradientStop> GradientStops;

        GradientStops.Add(FSlateGradientStop(FVector2D(AllottedGeometry.Size.X*.1f, 0), FColor::Yellow));
        GradientStops.Add(FSlateGradientStop(FVector2D(AllottedGeometry.Size.X*.25f, 0), FColor::Magenta));

        FSlateDrawElement::MakeGradient(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(FVector2D(700.0f, 50.0f), FVector2D(200.0f, 200.0f)),
            GradientStops,
            Orient_Vertical,
            MyClippingRect,
            ESlateDrawEffect::None
        );
    }

    {
        const FVector2D Start(10, 10);
        const FVector2D StartDir(AllottedGeometry.Size.X * 1000 / 600, 0);
        const FVector2D End(AllottedGeometry.Size.X / 4, AllottedGeometry.Size.Y - 10);
        const FVector2D EndDir(AllottedGeometry.Size.X * 1000 / 600, 0);

        FSlateDrawElement::MakeSpline(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            Start, StartDir,
            End, EndDir,
            MyClippingRect,
            50.0f,
            ESlateDrawEffect::None,
            FColor::White
        );
    }

    {
        TArray<FVector2D> LinePoints;
        LinePoints.Add(FVector2D(300.0f, 330.0f));
        LinePoints.Add(FVector2D(400.0f, 330.0f));
        LinePoints.Add(FVector2D(500.0f, 550.0f));
        LinePoints.Add(FVector2D(400.0f, 670.0f));

        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            LinePoints,
            MyClippingRect,
            ESlateDrawEffect::None,
            FLinearColor::Green,
            true,
            50.0f
        );
    }

    {
        TArray<FVector2D> LinePoints;
        LinePoints.Add(FVector2D(310.0f, 330.0f));
        LinePoints.Add(FVector2D(410.0f, 330.0f));
        LinePoints.Add(FVector2D(510.0f, 550.0f));
        LinePoints.Add(FVector2D(410.0f, 670.0f));


        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            LinePoints,
            MyClippingRect,
            ESlateDrawEffect::None,
            FLinearColor::Blue,
            true,
            1.0f
        );
    }


    {
        const float Radius = 50.0f;
        const FVector2D Center = FVector2D(100, 100);

        const FSlateBrush* MyBrush = FPaintDemoStyle::Get().GetBrush("TestCircleColor");

        FSlateShaderResourceProxy *ResourceProxy = FSlateDataPayload::ResourceManager->GetShaderResource(*MyBrush);
        FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*MyBrush);

        FVector2D UVCenter = FVector2D::ZeroVector;
        FVector2D UVRadius = FVector2D(1, 1);
        if (ResourceProxy != nullptr)
        {
            UVRadius = 0.5f*ResourceProxy->SizeUV;
            UVCenter = ResourceProxy->StartUV + UVRadius;
        }

        // Make a triangle fan in the area allotted
        const int NumTris = 12;


        TArray<FSlateVertex> Verts;
        Verts.AddUninitialized(4);

        {
            FSlateVertex& NewVert = Verts[0];
            NewVert.Position = FVector2D(860.0f, 440.0f);
            NewVert.TexCoords[0] = 0.0f;
            NewVert.TexCoords[1] = 0.0f;
            NewVert.TexCoords[2] = NewVert.TexCoords[3] = 1.0f;
            NewVert.Color = FColor::White;
            NewVert.ClipRect = FSlateRotatedRect(MyClippingRect);
        }

        {
            FSlateVertex& NewVert = Verts[1];
            NewVert.Position = FVector2D(1060.0f, 440.0f);
            NewVert.TexCoords[0] = 1.0f;
            NewVert.TexCoords[1] = 0.0f;
            NewVert.TexCoords[2] = NewVert.TexCoords[3] = 1.0f;
            NewVert.Color = FColor::White;
            NewVert.ClipRect = FSlateRotatedRect(MyClippingRect);
        }

        {
            FSlateVertex& NewVert = Verts[2];
            NewVert.Position = FVector2D(860.0f, 640.0f);

            NewVert.TexCoords[0] = 0.0f;
            NewVert.TexCoords[1] = 1.0f;
            NewVert.TexCoords[2] = NewVert.TexCoords[3] = 1.0f;
            NewVert.Color = FColor::White;
            NewVert.ClipRect = FSlateRotatedRect(MyClippingRect);
        }

        {
            FSlateVertex& NewVert = Verts[3];
            NewVert.Position = FVector2D(1060.0f, 640.0f);

            NewVert.TexCoords[0] = 1.0f;
            NewVert.TexCoords[1] = 1.0f;
            NewVert.TexCoords[2] = NewVert.TexCoords[3] = 1.0f;
            NewVert.Color = FColor::White;
            NewVert.ClipRect = FSlateRotatedRect(MyClippingRect);
        }

        TArray<SlateIndex> Indexes;
        Indexes.Add(0);
        Indexes.Add(1);
        Indexes.Add(2);

        Indexes.Add(1);
        Indexes.Add(3);
        Indexes.Add(2);

        FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, Verts, Indexes, nullptr, 0, 0);
    }

    {
        if (bIsDrawingLine)
        {
            TArray<FVector2D> LinePoints;
            auto V = GraphCoordToPanelCoord(TestLinePointA);
            LinePoints.Add(V);
            LinePoints.Add(CachedMousePositionInPanelCoord);

            FSlateDrawElement::MakeLines(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(),
                LinePoints,
                MyClippingRect,
                ESlateDrawEffect::None,
                FLinearColor::Yellow,
                true,
                1.0f
            );

            auto MM = (V + CachedMousePositionInPanelCoord) * 0.5f;
            float L = FVector2D::Distance(V, CachedMousePositionInPanelCoord);
            

            const FText Text = FText::FromString(FString::Printf(TEXT("Length: %fm"), L));
            uint32 FontSize = 14;
            FSlateFontInfo FontInfo = FCoreStyle::Get().GetFontStyle("ToolTip.Font");
            FontInfo.OutlineSettings.OutlineColor = FLinearColor::Green;

            FSlateDrawElement::MakeText(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(MM, AllottedGeometry.Size, 1.0f),
                Text.ToString(),
                FontInfo,
                MyClippingRect,
                ESlateDrawEffect::None,
                FColor(0, 255, 255)
            );


        }

    }



    {
        for (auto &Element : FloorPlanElementList)
        {
            Element->OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
        }
    }


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

    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        auto MouseDownPositionInScreenCoord = MouseEvent.GetLastScreenSpacePosition();
        auto MouseDownPositionInPanelCoord = MyGeometry.AbsoluteToLocal(MouseDownPositionInScreenCoord);
        auto PositionInGraphCoord = PanelCoordToGraphCoord(MouseDownPositionInPanelCoord);

        if (bIsDrawingLine)
        {
            TestLinePointB = PositionInGraphCoord;

            LineList.Push(TPair<FVector2D, FVector2D>(TestLinePointA, TestLinePointB));
            bIsDrawingLine = false;
        }
        else
        {
            TestLinePointA = PositionInGraphCoord;
            bIsDrawingLine = true;
        }

        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red,
            FString::Printf(TEXT("%s - %s"), *MouseDownPositionInScreenCoord.ToString(), *PositionInGraphCoord.ToString())
        );
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

    {
        auto MousePositionInScreenCoord = MouseEvent.GetLastScreenSpacePosition();
        CachedMousePositionInPanelCoord = MyGeometry.AbsoluteToLocal(MousePositionInScreenCoord);
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

        this->ViewOffset = PointToMaintainGraphSpace - WidgetSpaceZoomOrigin / GetZoomAmount();
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
    return 10;
}

void SPaintDemoSurface::ConstructSequence()
{
    Sequence = FCurveSequence();
    Curve = Sequence.AddCurve(0.f, 1.0f);
    Sequence.Play(this->AsShared(), true);
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
    const float GraphSmallestGridSize = 10.0f;
    const float RawZoomFactor = GetZoomAmount();
    const float NominalGridSize = GetSnapGridSize() * GetGridScaleAmount();

    float ZoomFactor = RawZoomFactor;
    float Inflation = 1.0f;
    while (ZoomFactor * Inflation*NominalGridSize <= GraphSmallestGridSize)
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