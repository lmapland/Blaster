// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/BlasterController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameMode/BlasterGameMode.h"
#include "GameStates/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Characters/Blaster.h"
#include "BlasterComponents/CombatComponent.h"
#include "PlayerStates/BlasterPlayerState.h"
#include "HUD/BlasterHUD.h"
#include "HUD/BlasterOverlay.h"
#include "HUD/AnnouncementWidget.h"
#include "HUD/ReturnToMainMenu.h"
#include "Enums/Announcement.h"

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
	DOREPLIFETIME(ABlasterController, bShowTeamScores);
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
	if (HasAuthority()) return;

	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			// GetCompressedPing() returns a compressed value; must multiply by 4 to get the real value
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold)
			{
				//UE_LOG(LogTemp, Warning, TEXT("CheckPing(): Ping: %i"), PlayerState->GetCompressedPing() * 4);
				ShowHighPingWarning(true);
				ServerReportPingStatus(true);
			}
			else ServerReportPingStatus(false);
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

void ABlasterController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent == nullptr) return;
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(QuitAction, ETriggerEvent::Completed, this, &ABlasterController::ShowRetunToMainMenu);
	}
}

void ABlasterController::ShowRetunToMainMenu()
{
	if (!ReturnToMainMenuClass) return;
	if (!ReturnToMainMenuWidget)
	{
		ReturnToMainMenuWidget = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuClass);
	}
	if (!ReturnToMainMenuWidget) return;

	// bReturnToMainMenuOpen starts out false
	if (bReturnToMainMenuOpen) // menu is currently open; close it
	{
		ReturnToMainMenuWidget->MenuTeardown();
	}
	else // menu is currently closed; open it
	{
		ReturnToMainMenuWidget->MenuSetup();
	}

	bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
}

void ABlasterController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
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
		bShowTeamScores = BlasterGameMode->GetIsTeamsMatch();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime, bShowTeamScores);

		if (HUD && MatchState == MatchState::WaitingToStart)
		{
			HUD->AddAnnouncement();
		}
	}
}

void ABlasterController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown, bool InShouldShowTeamScores)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	bShowTeamScores = InShouldShowTeamScores;
	OnRep_ShowTeamScores();
	OnMatchStateSet(MatchState, bShowTeamScores);

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

void ABlasterController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}

void ABlasterController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;

	if (Attacker && Victim && Self && HUD)
	{
		if (Attacker == Self && Victim != Self)
		{
			HUD->AddElimAnnouncement("You", Victim->GetPlayerName());
		}
		else if (Attacker == Self && Victim == Self)
		{
			HUD->AddElimAnnouncement("You", "yourself");
		}
		else if (Attacker != Self && Victim == Self)
		{
			HUD->AddElimAnnouncement(Attacker->GetPlayerName(), "you");
		}
		else if (Attacker == Victim)
		{
			HUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
		}
		else
		{
			HUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
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

void ABlasterController::HandleMatchHasStarted(bool bTeamsMatch)
{
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;

	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;
	if (!HUD) return;

	HUD->AddCharacterOverlay();
	HUD->SetAnnouncementVisibility(false);

	if (HasAuthority()) OnRep_ShowTeamScores();
}

void ABlasterController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		SetHUDBlueTeamScore(0);
		SetHUDRedTeamScore(0);
		SetTeamScoresVisible(true);
	}
	else
	{
		SetTeamScoresVisible(false);
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
	if (!HUD->Announcement) HUD->AddAnnouncement(); // Sometimes the client's Announcement Widget is not created at this point

	if (HUD->Announcement)
	{
		HUD->SetAnnouncementVisibility(true);
		HUD->Announcement->SetAnnouncementText(Announcement::NewMatchStartsIn);

		ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
		if (BlasterGameState)
		{
			TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->GetTopScoringPlayers();
			FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(BlasterGameState) : GetInfoText(TopPlayers);
			HUD->Announcement->SetInfoText(InfoTextString);
		}
	}
}

FString ABlasterController::GetInfoText(const TArray<ABlasterPlayerState*>& InPlayers)
{
	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (!BlasterPlayerState) return FString();

	FString InfoTextString;
	if (InPlayers.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (InPlayers.Num() == 1)
	{
		if (InPlayers[0] == BlasterPlayerState)
		{
			InfoTextString = Announcement::YouWon;
		}
		else
		{
			InfoTextString = FString::Printf(TEXT("%s won!"), *InPlayers[0]->GetPlayerName());
		}
	}
	else
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		for (auto TiedPlayer : InPlayers)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

FString ABlasterController::GetTeamsInfoText(ABlasterGameState* BlasterGameState)
{
	if (!BlasterGameState) return FString();

	FString InfoTextString;
	int32 RedTeamScore = BlasterGameState->GetRedTeamScore();
	int32 BlueTeamScore = BlasterGameState->GetBlueTeamScore();

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = Announcement::TeamsTiedForTheWin;
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
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

void ABlasterController::SetTeamScoresVisible(bool IsVisible)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;

	if (HUD && HUD->Overlay) HUD->Overlay->SetTeamScoreVisibility(IsVisible);
}

void ABlasterController::SetHUDRedTeamScore(int32 Score)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;

	if (HUD && HUD->Overlay) HUD->Overlay->SetRedTeamScore(Score);
}

void ABlasterController::SetHUDBlueTeamScore(int32 Score)
{
	HUD = HUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : HUD;

	if (HUD && HUD->Overlay) HUD->Overlay->SetBlueTeamScore(Score);
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
				OnRep_ShowTeamScores();
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
