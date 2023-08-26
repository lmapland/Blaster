// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterController.generated.h"

class ABlasterHUD;
class UBlasterOverlay;
class ABlasterGameMode;

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
	void OnMatchStateSet(FName State);

	float SingleTripTime = 0.f;

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	void HandleMatchHasStarted();
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
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown);

	void ShowHighPingWarning(bool bShouldShow);
	void CheckPing(float DeltaTime);

	// Difference between client and server time
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = "Controller Properties | Server Sync")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

private:
	UFUNCTION()
	void OnRep_MatchState();

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

	bool bInitializeCharacterOverlay = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDShield;
	float HUDMaxShield;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;
	
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

};
