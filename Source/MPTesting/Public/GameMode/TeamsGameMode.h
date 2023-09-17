// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameMode/BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

class ABlaster;
class ABlasterController;

/**
 * 
 */
UCLASS()
class MPTESTING_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void PlayerEliminated(ABlaster* ElimmedCharacter, ABlasterController* VictimController, ABlasterController* AttackerController) override;
	bool PlayerShouldDamage(AController* PlayerOne, AController* PlayerTwo) override;

protected:
	virtual void HandleMatchHasStarted() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode Properties|Options")
	bool bFriendlyFireEnabled = false;

};
