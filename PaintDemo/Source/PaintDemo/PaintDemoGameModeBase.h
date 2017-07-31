// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PaintDemoGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class PAINTDEMO_API APaintDemoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
	
public:
    virtual void BeginPlay() override;
	
private:
    TSharedPtr<SWidget> Widget;
};
