// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UBlasterOverlay::SetHealth(float Health, float MaxHealth)
{
	if (HealthBar && HealthText)
	{
		HealthBar->SetPercent(Health / MaxHealth);
		const FString HealthString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		HealthText->SetText(FText::FromString(HealthString));
	}
}

void UBlasterOverlay::SetStamina(float Stamina, float MaxStamina)
{
	/*if (StaminaBar && StaminaText)
	{
		HealthBar->SetPercent(Stamina / MaxStamina);
		const FString StaminaString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Stamina), FMath::CeilToInt(MaxStamina));
		HealthText->SetText(FText::FromString(StaminaString));
	}*/
}
