// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/SPlayerCameraManager.h"

ASPlayerCameraManager::ASPlayerCameraManager(const FObjectInitializer& ObjectInitializer)
{
	LOSProbe = 12.0f;
	LOSProbeChannel = ECC_Camera;
}

void ASPlayerCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);
}
