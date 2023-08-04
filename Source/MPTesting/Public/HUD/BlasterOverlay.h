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
	void SetScore(float Score);
	void SetDefeats(int32 Defeats);

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

};
