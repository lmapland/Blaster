// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	extern MPTESTING_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

class ABlaster;
class ABlasterController;
class ABlasterPlayerState;

/**
 * 
 */
UCLASS()
class MPTESTING_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABlasterGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(ABlaster* ElimmedCharacter, ABlasterController* VictimController, ABlasterController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(ABlasterPlayerState* PlayerLeaving);

	/* Should PlayerOne be able to Damage PlayerTwo? Will depend on Friendly Fire and whether the two players are on the same team */
	virtual bool PlayerShouldDamage(AController* PlayerOne, AController* PlayerTwo);

	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Properties|Game Start")
	float WarmupTime = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Properties|Game Start")
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Properties|Game Cooldown")
	float CooldownTime = 120.f;

	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	FORCEINLINE bool GetIsTeamsMatch() const { return bTeamsMatch; }
};
