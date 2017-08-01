// Fill out your copyright notice in the Description page of Project Settings.

#include "SPaintDemoView.h"
#include "SlateOptMacros.h"
#include "SOverlay.h"
#include "PaintDemoStyle.h"
#include "SGridPanel.h"
#include "SBorder.h"
#include "CoreStyle.h"
#include "SPaintDemoRuler.h"
#include "SPaintDemoZoomPan.h"

void SPaintDemoView::Construct(const FArguments& InArgs)
{
    SPaintDemoSurface::Construct(
        SPaintDemoSurface::FArguments()
        .AllowContinousZoomInterpolation(false)
        .Content()
        [
            SNew(SOverlay)
            + SOverlay::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SAssignNew(PreviewHitTestRoot, SPaintDemoZoomPan)
                .Visibility(EVisibility::HitTestInvisible)
                .ZoomAmount(this, &SPaintDemoView::GetZoomAmount)
                .ViewOffset(this, &SPaintDemoView::GetViewOffset)
                [
                    SNew(SOverlay)
                    + SOverlay::Slot()
                    [
                        SAssignNew(PreviewAreaConstraint, SBox)
                        .WidthOverride(this, &SPaintDemoView::GetPreviewAreaWidth)
                        .HeightOverride(this, &SPaintDemoView::GetPreviewAreaHeight)
                    ]
                ]
            ]
            + SOverlay::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SNew(SGridPanel)
                .FillColumn(1, 1.0f)
                .FillRow(1, 1.0f)

                // Corner
                + SGridPanel::Slot(0, 0)
                [
                    SNew(SBorder)
                    .BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
                .BorderBackgroundColor(FLinearColor(FColor(48, 48, 48)))
                ]

                // Top Ruler
                + SGridPanel::Slot(1, 0)
                [
                    SAssignNew(TopRuler, SPaintDemoRuler)
                    .Orientation(Orient_Horizontal)
                ]

                // Side Ruler
                + SGridPanel::Slot(0, 1)
                [
                    SAssignNew(SideRuler, SPaintDemoRuler)
                    .Orientation(Orient_Vertical)
                ]
            ]
        ]
   );

}

void SPaintDemoView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

    SPaintDemoSurface::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    // Perform an arrange children pass to cache the geometry of all widgets so that we can query it later.
    {
        CachedWidgetGeometry.Reset();
        FArrangedWidget WindowWidgetGeometry(PreviewHitTestRoot.ToSharedRef(), AllottedGeometry);
        PopulateWidgetGeometryCache(WindowWidgetGeometry);
    }


    // Compute the origin in absolute space.
    FGeometry RootGeometry = CachedWidgetGeometry.FindChecked(PreviewAreaConstraint.ToSharedRef()).Geometry;
    FVector2D AbsoluteOrigin = MakeGeometryWindowLocal(RootGeometry).LocalToAbsolute(FVector2D::ZeroVector);

    TopRuler->SetRuling(AbsoluteOrigin, 1.0f / GetZoomAmount());
    SideRuler->SetRuling(AbsoluteOrigin, 1.0f / GetZoomAmount());

    if (IsHovered())
    {
        // Get cursor in absolute window space.
        FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
        CursorPos = MakeGeometryWindowLocal(RootGeometry).LocalToAbsolute(RootGeometry.AbsoluteToLocal(CursorPos));

        TopRuler->SetCursor(CursorPos);
        SideRuler->SetCursor(CursorPos);
    }
    else
    {
        TopRuler->SetCursor(TOptional<FVector2D>());
        SideRuler->SetCursor(TOptional<FVector2D>());
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SPaintDemoView::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{

    SPaintDemoSurface::OnMouseButtonDown(MyGeometry, MouseEvent);

    return FReply::Handled().PreventThrottling().SetUserFocus(AsShared(), EFocusCause::Mouse).CaptureMouse(AsShared());
}

FReply SPaintDemoView::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    SPaintDemoSurface::OnMouseButtonUp(MyGeometry, MouseEvent);

    return FReply::Handled().ReleaseMouseCapture();
}

FReply SPaintDemoView::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetCursorDelta().IsZero())
    {
        return FReply::Unhandled();
    }

    FReply SurfaceHandled = SPaintDemoSurface::OnMouseMove(MyGeometry, MouseEvent);
    if (SurfaceHandled.IsEventHandled())
    {
        return SurfaceHandled;
    }

    return FReply::Unhandled();
}

EVisibility SPaintDemoView::GetRulerVisibility()
{
    return EVisibility::Visible;
}

FGeometry SPaintDemoView::MakeGeometryWindowLocal(const FGeometry& WidgetGeometry) const 
{
    FGeometry NewGeometry = WidgetGeometry;

    TSharedPtr<SWindow> WidgetWindow = FSlateApplication::Get().FindWidgetWindow(SharedThis(this));
    if (WidgetWindow.IsValid())
    {
        TSharedRef<SWindow> CurrentWindowRef = WidgetWindow.ToSharedRef();

        NewGeometry.AppendTransform(FSlateLayoutTransform(Inverse(CurrentWindowRef->GetPositionInScreen())));
        //NewGeometry.AppendTransform(Inverse(CurrentWindowRef->GetLocalToScreenTransform()));
    }

    return NewGeometry;
}

void SPaintDemoView::PopulateWidgetGeometryCache(FArrangedWidget& Root)
{
    FArrangedChildren ArrangedChildren(EVisibility::All);
    Root.Widget->ArrangeChildren(Root.Geometry, ArrangedChildren);

    CachedWidgetGeometry.Add(Root.Widget, Root);

    // A widget's children are implicitly Z-ordered from first to last
    for (int32 ChildIndex = ArrangedChildren.Num() - 1; ChildIndex >= 0; --ChildIndex)
    {
        FArrangedWidget& SomeChild = ArrangedChildren[ChildIndex];
        PopulateWidgetGeometryCache(SomeChild);
    }
}

FOptionalSize SPaintDemoView::GetPreviewAreaWidth() const
{
    return 100;
}

FOptionalSize SPaintDemoView::GetPreviewAreaHeight() const
{
    return 100;
}
