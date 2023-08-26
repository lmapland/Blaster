// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/BlasterController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/BlasterGameMode.h"
#include "HUD/BlasterHUD.h"
#include "HUD/BlasterOverlay.h"
#include "HUD/AnnouncementWidget.h"
#include "Characters/Blaster.h"
#include "BlasterComponents/CombatComponent.h"
#include "GameStates/BlasterGameState.h"
#include "PlayerStates/BlasterPlayerState.h"

void ABlasterController::BeginPlay()
{
	Super::BeginPlay();

	HUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlasterController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterController, MatchState);
}

void ABlasterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
	CheckPing(DeltaTime);
}

void ABlasterController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			// GetCompressedPing() returns a compressed value; must multiply by 4 to get the real value
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold) ShowHighPingWarning(true);
		}
		HighPingRunningTime = 0.f;
	}

	if (bHighPingWarningIsRunning) HighPingWarningRunningTime += DeltaTime;
	if (HighPingWarningRunningTime > HighPingWarningDuration)
	{
		ShowHighPingWarning(false);
		HighPingWarningRunningTime = 0.f;
		bHighPingWarningIsRunning = false;
	}
}

void ABlasterController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterController::ShowHighPingWarning(bool bShouldShow)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;

	if (HUD && HUD->Overlay)
	{
		HUD->Overlay->SetHighPingWarningVisible(bShouldShow);
		if (bShouldShow) bHighPingWarningIsRunning = true;
	}
}

void ABlasterController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (BlasterGameMode)
	{
		WarmupTime = BlasterGameMode->WarmupTime;
		MatchTime = BlasterGameMode->MatchTime;
		LevelStartingTime = BlasterGameMode->LevelStartingTime;
		CooldownTime = BlasterGameMode->CooldownTime;
		MatchState = BlasterGameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);

		if (HUD && MatchState == MatchState::WaitingToStart)
		{
			HUD->AddAnnouncement();
		}
	}
}

void ABlasterController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);

	if (HUD && MatchState == MatchState::WaitingToStart)
	{
		HUD->AddAnnouncement();
	}
}

float ABlasterController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
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

void ABlasterController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterController::HandleMatchHasStarted()
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	if (!HUD) return;

	HUD->AddCharacterOverlay();
	if (HUD->Announcement)
	{
		HUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABlasterController::HandleCooldown()
{
	ABlaster* Blaster = Cast<ABlaster>(GetPawn());
	if (Blaster)
	{
		Blaster->bDisableGameplay = true;
		if (Blaster->GetCombatComponent())
		{
			Blaster->GetCombatComponent()->FireButtonPressed(false);
		}
	}

	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	if (!HUD) return;

	HUD->Overlay->RemoveFromParent();
	if (HUD->Announcement)
	{
		HUD->Announcement->SetVisibility(ESlateVisibility::Visible);
		HUD->Announcement->SetAnnouncementText(FString("New Match Starts In:"));

		ABlasterGameState* GameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
		ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();

		if (GameState && BlasterPlayerState)
		{
			TArray<ABlasterPlayerState*> TopPlayers = GameState->TopScoringPlayers;
			FString InfoTextString;
			if (TopPlayers.Num() == 0)
			{
				InfoTextString = FString("There is no winner.");
			}
			else if (TopPlayers.Num() == 1)
			{
				if (TopPlayers[0] == BlasterPlayerState)
				{
					InfoTextString = FString("You won!");
				}
				else
				{
					InfoTextString = FString::Printf(TEXT("%s won!"), *TopPlayers[0]->GetPlayerName());
				}
			}
			else
			{
				InfoTextString = FString("Players tied for the win:\n");
				for (auto TiedPlayer : TopPlayers)
				{
					InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
				}
			}
			HUD->Announcement->SetInfoText(InfoTextString);
		}
	}
}

void ABlasterController::SetHUDHealth(float Health, float MaxHealth)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	
	if (HUD && HUD->Overlay)
	{
		HUD->Overlay->SetHealth(Health, MaxHealth);
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterController::SetHUDShield(float Shield, float MaxShield)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;

	if (HUD && HUD->Overlay)
	{
		HUD->Overlay->SetShield(Shield, MaxShield);
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABlasterController::SetHUDDefeats(int32 Defeats)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	
	if (HUD && HUD->Overlay)
	{
		HUD->Overlay->SetDefeats(Defeats);
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterController::SetHUDScore(float Score)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	
	if (HUD && HUD->Overlay)
	{
		HUD->Overlay->SetScore(Score);
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ABlasterController::SetHUDWeaponAmmo(int32 Ammo)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;

	if (HUD && HUD->Overlay)
	{
		HUD->Overlay->SetWeaponAmmo(Ammo);
	}
	else
	{
		bInitializeAmmo = true;
		HUDAmmo = Ammo;
	}
}

void ABlasterController::SetHUDCarriedAmmo(int32 Ammo)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;

	if (HUD && HUD->Overlay)
	{
		HUD->Overlay->SetCarriedAmmo(Ammo);
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterController::SetHUDMatchCountdown(float CountdownTime)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	if (!HUD || !HUD->Overlay) return;

	if (CountdownTime < 0.f)
	{
		HUD->Overlay->SetMatchCountdownText(0, 0);
		return;
	}

	int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);

	HUD->Overlay->SetMatchCountdownText(Minutes, FMath::FloorToInt(CountdownTime - (Minutes * 60)));
}

void ABlasterController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	if (!HUD || !HUD->Announcement) return;

	if (CountdownTime < 0.f)
	{
		HUD->Announcement->SetWarmupTime(0, 0);
		return;
	}

	int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);

	HUD->Announcement->SetWarmupTime(Minutes, FMath::FloorToInt(CountdownTime - (Minutes * 60)));
}

void ABlasterController::SetHUDGrenades(int32 Grenades)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;

	if (HUD && HUD->Overlay)
	{
		HUD->Overlay->SetGrenadesText(Grenades);
	}
	else
	{
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

void ABlasterController::SetHUDTime()
{
	float TimeLeft = 0.f;

	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		GameMode = GameMode ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : GameMode;
		if (GameMode)
		{
			SecondsLeft = FMath::CeilToInt(GameMode->GetCountdownTime()) + LevelStartingTime;
		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ABlasterController::PollInit()
{
	if (Overlay == nullptr)
	{
		if (HUD && HUD->Overlay)
		{
			Overlay = HUD->Overlay;
			if (Overlay)
			{
				if (bInitializeHealth) Overlay->SetHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeDefeats) Overlay->SetDefeats(HUDDefeats);
				if (bInitializeScore) Overlay->SetScore(HUDScore);
				if (bInitializeGrenades) Overlay->SetGrenadesText(HUDGrenades);
				if (bInitializeShield) Overlay->SetShield(HUDShield, HUDMaxShield);
				if (bInitializeAmmo) Overlay->SetWeaponAmmo(HUDAmmo);
				if (bInitializeCarriedAmmo) Overlay->SetCarriedAmmo(HUDCarriedAmmo);
			}
		}
	}
}

void ABlasterController::ServerRequestServerTime_Implementation(float ClientRequestTime)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(ClientRequestTime, ServerTimeOfReceipt);
}

void ABlasterController::ClientReportServerTime_Implementation(float ClientRequestTime, float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - ClientRequestTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + RoundTripTime * 0.5f;
	SingleTripTime = RoundTripTime * 0.5f;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
