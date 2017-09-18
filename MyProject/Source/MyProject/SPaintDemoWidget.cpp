// Fill out your copyright notice in the Description page of Project Settings.

#include "SPaintDemoWidget.h"
#include "SlateOptMacros.h"
#include "UnrealMathUtility.h"
#include "SBox.h"
#include "DrawElements.h"
#include "CoreStyle.h"

#define LOCTEXT_NAMESPACE "PaintDemo"


void SPaintDemoWidget::Construct(const FArguments& InArgs)
{
    ClockCenter = FVector2D(400.0f, 400.0f);
    ClockRadius = 200.0f;
    AccumulatedTime = 0.0f;

    BuildClockCalibrationLines();

    BuildClockOutlineCirclePath();

    SecondHandPath.Add(ClockCenter);
    SecondHandPath.Add(ClockCenter);

    MinuteHandPath.Add(ClockCenter);
    MinuteHandPath.Add(ClockCenter);

    HourHandPath.Add(ClockCenter);
    HourHandPath.Add(ClockCenter);

    ChildSlot
        [
            SNew(SBox)
            .WidthOverride(800.0f)
        .HeightOverride(800.0f)
        ];

}

void SPaintDemoWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    AccumulatedTime += InDeltaTime;
    if (AccumulatedTime > 60.0f)
    {
        AccumulatedTime -= 60.0f;
    }

    FDateTime NowTime = FDateTime::Now();
    {
        float Hour = static_cast<float>(NowTime.GetHour12());
        FVector2D StartDirection(0.0f, -1.0f);
        float CurrentAngle = Hour / 12.0f * 360.0f;
        FVector2D CurreDirection = StartDirection.GetRotated(CurrentAngle);
        CurreDirection.Normalize();
        FVector2D NewPoint = ClockRadius * 0.3f * CurreDirection + ClockCenter;
        HourHandPath[1] = NewPoint;
    }

    {
        float Minute = static_cast<float>(NowTime.GetMinute());
        FVector2D StartDirection(0.0f, -1.0f);
        float CurrentAngle = Minute / 60.0f * 360.0f;
        FVector2D CurreDirection = StartDirection.GetRotated(CurrentAngle);
        CurreDirection.Normalize();
        FVector2D NewPoint = ClockRadius * 0.5f * CurreDirection + ClockCenter;
        MinuteHandPath[1] = NewPoint;
    }

    {
        float Minute = static_cast<float>(NowTime.GetSecond());
        FVector2D StartDirection(0.0f, -1.0f);
        float CurrentAngle = Minute / 60.0f * 360.0f;
        FVector2D CurreDirection = StartDirection.GetRotated(CurrentAngle);
        CurreDirection.Normalize();
        FVector2D NewPoint = ClockRadius * 0.8f * CurreDirection + ClockCenter;
        SecondHandPath[1] = NewPoint;
    }

    TimeString = FString::Printf(TEXT("%02d:%02d:%02d"), NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());
}

int32 SPaintDemoWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
        // Draw Hour Hand
        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            HourHandPath,
            ESlateDrawEffect::None,
            FLinearColor::Green,
            true,
            5.0f
        );

        // Draw Minute Hand
        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            MinuteHandPath,
            ESlateDrawEffect::None,
            FLinearColor::Blue,
            true,
            3.0f
        );

        // Draw Second Hand
        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            SecondHandPath,
            ESlateDrawEffect::None,
            FLinearColor::Red,
            true,
            1.0f
        );

    // Draw Circle
    FSlateDrawElement::MakeLines(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(),
        ClockOutlineCirclePath,
        ESlateDrawEffect::None,
        FLinearColor::Yellow,
        true,
        1.0f
    );

    // Draw ClockCalibrationLines
    for (auto &Line : ClockCalibrationLines)
    {
        FSlateDrawElement::MakeLines(
            OutDrawElements,
            LayerId,
            AllottedGeometry.ToPaintGeometry(),
            Line.GetLinePoints(),
            ESlateDrawEffect::None,
            Line.GetColor(),
            true,
            1.0f
        );
    }

    FSlateFontInfo FontInfo = FCoreStyle::Get().GetFontStyle("NormalFont");
    FontInfo.Size = 30.0f;
    FSlateDrawElement::MakeText(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry(FVector2D(350.0f, 650.0f), AllottedGeometry.Size),
        TimeString,
        FontInfo,
        ESlateDrawEffect::None ,
        FLinearColor::White
    );

    return LayerId;
}


void SPaintDemoWidget::BuildClockOutlineCirclePath()
{
    FVector2D StartDirection(0.0f, -1.0f);
    int32 SliceCount = 360;
    float CurrentStepAngleInDegree = 0.0f;
    ClockOutlineCirclePath.AddUninitialized(SliceCount + 1);

    for (int32 i = 0; i <= SliceCount; i++)
    {
        CurrentStepAngleInDegree += 360.0f / static_cast<float>(SliceCount);
        FVector2D CurreDirection = StartDirection.GetRotated(CurrentStepAngleInDegree);
        CurreDirection.Normalize();
        FVector2D NewPoint = ClockRadius * CurreDirection + ClockCenter;
        ClockOutlineCirclePath[i] = NewPoint;
    }

}


void SPaintDemoWidget::BuildClockCalibrationLines()
{
    FVector2D StartDirection(0.0f, -1.0f);
    int32 SliceCount = 60;
    float CurrentStepAngleInDegree = 0.0f;
    for (int32 i = 0; i < SliceCount; i++)
    {
        FVector2D CurreDirection = StartDirection.GetRotated(CurrentStepAngleInDegree);
        CurreDirection.Normalize();

        FClockCalibrationLine NewClockCalibration;
        NewClockCalibration.SetStartPoint(ClockRadius * CurreDirection + ClockCenter);

        float ClockCalibrationLength = ClockRadius * 0.03f;
        FLinearColor ClockCalibrationColor = FLinearColor::Green;

        // Hour
        if (i % 5 == 0)
        {
            // Main hour
            if (i % 15 == 0)
            {
                ClockCalibrationLength = ClockRadius * 0.1f;
                ClockCalibrationColor = FLinearColor::Blue;
            }
            else
            {
                ClockCalibrationLength = ClockRadius * 0.06f;
                ClockCalibrationColor = FLinearColor::Red;
            }
        }

        NewClockCalibration.SetEndPoint((ClockRadius - ClockCalibrationLength) * CurreDirection + ClockCenter);
        NewClockCalibration.SetColor(ClockCalibrationColor);
        ClockCalibrationLines.Add(NewClockCalibration);

        CurrentStepAngleInDegree += 360.0f / static_cast<float>(SliceCount);
    }
}

#undef LOCTEXT_NAMESPACE

