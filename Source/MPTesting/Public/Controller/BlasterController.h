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

	// Difference between client and server time
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = "Server Sync")
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
};
