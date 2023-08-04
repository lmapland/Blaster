// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/BlasterController.h"
#include "HUD/BlasterHUD.h"
#include "HUD/BlasterOverlay.h"
#include "Characters/Blaster.h"

void ABlasterController::SetHUDHealth(float Health, float MaxHealth)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	
	if (!HUD || !HUD->Overlay) return;

	HUD->Overlay->SetHealth(Health, MaxHealth);
}

void ABlasterController::SetHUDDefeats(int32 Defeats)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	if (!HUD || !HUD->Overlay) return;

	HUD->Overlay->SetDefeats(Defeats);
}

void ABlasterController::SetHUDScore(float Score)
{
	//UE_LOG(LogTemp, Warning, TEXT("SetHUDScore(): In Score: %f"), Score);

	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	if (!HUD || !HUD->Overlay) return;

	HUD->Overlay->SetScore(Score);
}

void ABlasterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlaster* Blaster = Cast<ABlaster>(InPawn);
	if (Blaster)
	{
		Blaster->SetHUDHealth();
	}
}

void ABlasterController::BeginPlay()
{
	Super::BeginPlay();

	HUD = Cast<ABlasterHUD>(GetHUD());
}
