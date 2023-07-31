// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlasterOverlay.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class MPTESTING_API UBlasterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetHealth(float Health, float MaxHealth);
	void SetStamina(float Stamina, float MaxStamina);

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;
};
