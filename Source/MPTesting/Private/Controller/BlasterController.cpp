// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/BlasterController.h"
#include "HUD/BlasterHUD.h"
#include "HUD/BlasterOverlay.h"

void ABlasterController::SetHUDHealth(float Health, float MaxHealth)
{
	if (HUD == nullptr)
	{
		HUD = Cast<ABlasterHUD>(GetHUD());
	}
	//HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	
	if (!HUD || !HUD->Overlay) return; // || HUD->Overlay->Health) return;

	HUD->Overlay->SetHealth(Health, MaxHealth);
}

void ABlasterController::BeginPlay()
{
	Super::BeginPlay();

	HUD = Cast<ABlasterHUD>(GetHUD());
}
