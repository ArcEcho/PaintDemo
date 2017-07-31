// Fill out your copyright notice in the Description page of Project Settings.

#include "SPaintDemoView.h"
#include "SlateOptMacros.h"
#include "SOverlay.h"
#include "PaintDemoStyle.h"
#include "SGridPanel.h"
#include "SBorder.h"
#include "CoreStyle.h"
#include "SPaintDemoRuler.h"


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
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

   bIsOriginGeometryCached = false;
}

void SPaintDemoView::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    if (!bIsOriginGeometryCached)
    {
        CachedOriginGeometry = AllottedGeometry;
        bIsOriginGeometryCached = true;
    }

    SPaintDemoSurface::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

    FGeometry RootGeometry = CachedOriginGeometry;
    // Compute the origin in absolute space.
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
