// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UBlasterOverlay::SetHealth(float Health, float MaxHealth)
{
	if (HealthBar && HealthText)
	{
		HealthBar->SetPercent(Health / MaxHealth);
		const FString HealthString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		HealthText->SetText(FText::FromString(HealthString));
	}
}

void UBlasterOverlay::SetShield(float Shield, float MaxShield)
{
	if (ShieldBar && ShieldText)
	{
		ShieldBar->SetPercent(Shield / MaxShield);
		const FString ShieldString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		ShieldText->SetText(FText::FromString(ShieldString));
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

void UBlasterOverlay::SetScore(float Score)
{
	if (ScoreAmount)
	{
		const FString ScoreString = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		ScoreAmount->SetText(FText::FromString(ScoreString));
	}
}

void UBlasterOverlay::SetDefeats(int32 Defeats)
{
	if (DefeatsAmount)
	{
		const FString DefeatsString = FString::Printf(TEXT("%d"), Defeats);
		DefeatsAmount->SetText(FText::FromString(DefeatsString));
	}
}

void UBlasterOverlay::SetWeaponAmmo(int32 Ammo)
{
	if (WeaponAmmoAmount)
	{
		const FString AmmoString = FString::Printf(TEXT("%d"), Ammo);
		//UE_LOG(LogTemp, Warning, TEXT("UBlasterOverlay::SetWeaponAmmo(): Defeats: [%s]"), *AmmoString);
		WeaponAmmoAmount->SetText(FText::FromString(AmmoString));
	}
}

void UBlasterOverlay::SetCarriedAmmo(int32 Ammo)
{
	if (CarriedAmmoAmount)
	{
		const FString AmmoString = FString::Printf(TEXT("%d"), Ammo);
		CarriedAmmoAmount->SetText(FText::FromString(AmmoString));
	}
}

void UBlasterOverlay::SetMatchCountdownText(int32 Minutes, int32 Seconds)
{
	if (MatchCountdownText)
	{
		const FString CountdownString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		MatchCountdownText->SetText(FText::FromString(CountdownString));
	}
}

void UBlasterOverlay::SetGrenadesText(int32 Grenades)
{
	if (GrenadesText)
	{
		const FString GrenadesString = FString::Printf(TEXT("%d"), Grenades);
		GrenadesText->SetText(FText::FromString(GrenadesString));
	}
}

void UBlasterOverlay::SetBlueTeamScore(int32 Score)
{
	if (BlueTeamScore)
	{
		const FString ScoreString = FString::Printf(TEXT("%d"), Score);
		BlueTeamScore->SetText(FText::FromString(ScoreString));
	}
}

void UBlasterOverlay::SetRedTeamScore(int32 Score)
{
	if (RedTeamScore)
	{
		const FString ScoreString = FString::Printf(TEXT("%d"), Score);
		RedTeamScore->SetText(FText::FromString(ScoreString));
	}
}

void UBlasterOverlay::SetTeamScoreVisibility(bool bIsVisible)
{
	if (RedTeamScore && BlueTeamScore && ScoreSpacer)
	{
		if (bIsVisible)
		{
			RedTeamScore->SetVisibility(ESlateVisibility::Visible);
			BlueTeamScore->SetVisibility(ESlateVisibility::Visible);
			ScoreSpacer->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			RedTeamScore->SetVisibility(ESlateVisibility::Hidden);
			BlueTeamScore->SetVisibility(ESlateVisibility::Hidden);
			ScoreSpacer->SetVisibility(ESlateVisibility::Hidden);

		}
	}
}

void UBlasterOverlay::SetHighPingWarningVisible(bool bVisible)
{
	if (HighPingImage && HighPingAnimation)
	{
		if (bVisible)
		{
			HighPingImage->SetOpacity(1.f);
			PlayAnimation(HighPingAnimation, 0.f, 5);
		}
		else
		{
			HighPingImage->SetOpacity(0.f);
			if (IsAnimationPlaying(HighPingAnimation)) StopAnimation(HighPingAnimation);
		}
	}
}
