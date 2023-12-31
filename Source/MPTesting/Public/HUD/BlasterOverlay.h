// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlasterOverlay.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;

/**
 * 
 */
UCLASS()
class MPTESTING_API UBlasterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetHealth(float Health, float MaxHealth);
	void SetShield(float Shield, float MaxShield);
	void SetStamina(float Stamina, float MaxStamina);
	void SetScore(float Score);
	void SetDefeats(int32 Defeats);
	void SetWeaponAmmo(int32 Ammo);
	void SetCarriedAmmo(int32 Ammo);
	void SetMatchCountdownText(int32 Minutes, int32 Seconds);
	void SetGrenadesText(int32 Grenades);
	void SetBlueTeamScore(int32 Score);
	void SetRedTeamScore(int32 Score);
	void SetTeamScoreVisibility(bool bIsVisible);
	void SetHighPingWarningVisible(bool bVisible);

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthText;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScore;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamScore;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreSpacer;

	// Opacity is set to 0 by default
	UPROPERTY(meta = (BindWidget))
	UImage* HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;
};
