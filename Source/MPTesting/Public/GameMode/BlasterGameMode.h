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

	UPROPERTY(EditDefaultsOnly, Category = "Game Start")
	float WarmupTime = 10.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Start")
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Cooldown")
	float CooldownTime = 120.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
