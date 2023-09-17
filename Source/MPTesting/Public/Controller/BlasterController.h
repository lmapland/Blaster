// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

class ABlasterHUD;
class UBlasterOverlay;
class ABlasterGameMode;
class ABlasterGameState;
class ABlasterPlayerState;
class UInputAction;
class UUserWidget;
class UReturnToMainMenu;

/**
 * 
 */
UCLASS()
class MPTESTING_API ABlasterController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDScore(float Score);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);
	void SetTeamScoresVisible(bool IsVisible);
	void SetHUDRedTeamScore(int32 Score);
	void SetHUDBlueTeamScore(int32 Score);

	float SingleTripTime = 0.f;

	UPROPERTY()
	FHighPingDelegate HighPingDelegate;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HandleCooldown();

	// Sync time between client & server
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float ClientRequestTime);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float ClientRequestTime, float TimeServerReceivedClientRequest);

	void CheckTimeSync(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown, bool InShouldShowTeamScores);

	void ShowHighPingWarning(bool bShouldShow);
	void CheckPing(float DeltaTime);
	void SetupInputComponent() override;
	void ShowRetunToMainMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<ABlasterPlayerState*>& InPlayers);
	FString GetTeamsInfoText(ABlasterGameState* BlasterGameState);

	// Difference between client and server time
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = "Controller Properties | Server Sync")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Controller Properties | Input")
	UInputAction* QuitAction;
	
	/* Whether or not the team scores section of the Overlay should be shown. This is a stand-in for whether or not the game is in Teams Mode */
	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

private:
	UFUNCTION()
	void OnRep_MatchState();

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY()
	ABlasterHUD* HUD;

	UPROPERTY()
	UBlasterOverlay* Overlay;

	UPROPERTY()
	ABlasterGameMode* GameMode;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth = false;
	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false;
	float HUDScore;
	bool bInitializeScore = false;
	int32 HUDDefeats;
	bool bInitializeDefeats = false;
	int32 HUDGrenades;
	bool bInitializeGrenades = false;
	int32 HUDAmmo;
	bool bInitializeAmmo = false;
	int32 HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	
	UPROPERTY(EditAnywhere, Category = "Controller Properties | Ping")
	float HighPingThreshold;

	/* The current amount of time since the last High Ping check */
	float HighPingRunningTime = 0.f;

	/* How often to check for High Ping */
	UPROPERTY(EditAnywhere, Category = "Controller Properties | Ping")
	float CheckPingFrequency = 20.f;

	/* The current amount of time the High Ping Warning has been running */
	float HighPingWarningRunningTime = 0.f;

	/* How long to play the High Ping Warning */
	UPROPERTY(EditAnywhere, Category = "Controller Properties | Ping")
	float HighPingWarningDuration = 5.f;

	/* Indicates whether or not we need to track the High Ping Warning running time */
	bool bHighPingWarningIsRunning = false;

	UPROPERTY(EditAnywhere, Category = "Controller Properties | Widgets")
	TSubclassOf<UUserWidget> ReturnToMainMenuClass;

	UPROPERTY()
	UReturnToMainMenu* ReturnToMainMenuWidget;

	bool bReturnToMainMenuOpen = false;

};
