// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "SPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	ASPlayerCameraManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Camera")
	float LOSProbe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TEnumAsByte<ECollisionChannel> LOSProbeChannel;

protected:
	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

};
