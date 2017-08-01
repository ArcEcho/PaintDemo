// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SPaintDemoZoomPan.h"
#include "Layout/LayoutUtils.h"

/////////////////////////////////////////////////////
// SPaintDemoZoomPan

void SPaintDemoZoomPan::Construct(const FArguments& InArgs)
{
    ViewOffset = InArgs._ViewOffset;
    ZoomAmount = InArgs._ZoomAmount;

    ChildSlot
        [
            InArgs._Content.Widget
        ];
}

void SPaintDemoZoomPan::SetContent(const TSharedRef< SWidget >& InContent)
{
    ChildSlot
        [
            InContent
        ];
}

void SPaintDemoZoomPan::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
    const EVisibility ChildVisibility = ChildSlot.GetWidget()->GetVisibility();
    if (ArrangedChildren.Accepts(ChildVisibility))
    {
        const FMargin SlotPadding(ChildSlot.SlotPadding.Get());
        AlignmentArrangeResult XResult = AlignChild<Orient_Horizontal>(AllottedGeometry.Size.X, ChildSlot, SlotPadding, 1);
        AlignmentArrangeResult YResult = AlignChild<Orient_Vertical>(AllottedGeometry.Size.Y, ChildSlot, SlotPadding, 1);

        ArrangedChildren.AddWidget(ChildVisibility, AllottedGeometry.MakeChild(
            ChildSlot.GetWidget(),
            FVector2D(XResult.Offset, YResult.Offset) - ViewOffset.Get(),
            ChildSlot.GetWidget()->GetDesiredSize(),
            ZoomAmount.Get()
        ));
    }
}

float SPaintDemoZoomPan::GetRelativeLayoutScale(const FSlotBase& Child, float LayoutScaleMultiplier) const
{
    return ZoomAmount.Get();
}
