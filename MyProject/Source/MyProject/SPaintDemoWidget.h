// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Animation/CurveSequence.h"


class FClockCalibrationLine
{
public:
    FClockCalibrationLine()
        : Color(FLinearColor::Red)
    {
        LinePoints.AddUninitialized(2);
    }
public:
    const FVector2D &GetStartPoint() const
    {
        return LinePoints[0];
    }
    void SetStartPoint(const FVector2D &InPoint)
    {
        LinePoints[0] = InPoint;
    }

    const FVector2D GetEndPoint() const
    {
        return LinePoints[1];
    }

    void SetEndPoint(const FVector2D &InPoint)
    {
        LinePoints[1] = InPoint;
    }

    const FLinearColor &GetColor() const
    {
        return Color;
    }

    void SetColor(const FLinearColor &InColor)
    {
        Color = InColor;
    }

    const  TArray<FVector2D> &GetLinePoints() const
    {
        return LinePoints;
    }

private:
    TArray<FVector2D> LinePoints;
    FLinearColor Color;
};

/**
*
*/
class  SPaintDemoWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SPaintDemoWidget)
    {}
    SLATE_END_ARGS()

public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);

    // SWidget interface
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
 
protected:
    void PaintBackgroundAsLines(const FSlateBrush* BackgroundImage, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32& DrawLayerId) const;

    void BuildClockOutlineCirclePath();
    void BuildClockCalibrationLines();

private:

    TArray<FClockCalibrationLine> ClockCalibrationLines;
    float ClockRadius;
    FVector2D ClockCenter;

    TArray<FVector2D> ClockOutlineCirclePath;
    TArray<FVector2D> SecondHandPath;
    TArray<FVector2D> HourHandPath;
    TArray<FVector2D> MinuteHandPath;
    FString TimeString;
    float AccumulatedTime;
};
