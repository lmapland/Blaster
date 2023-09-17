// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Enums/Team.h"
#include "BlasterGameState.generated.h"

class ABlasterPlayerState;

/**
 * 
 */
UCLASS()
class MPTESTING_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);
	bool IsTopScoringPlayer(ABlasterPlayerState* PlayerState);
	void RemoveTopScoringPlayer(ABlasterPlayerState* PlayerToRemove);
	void RemovePlayerFromTeam(ABlasterPlayerState* PlayerToRemove);
	void RedTeamScores();
	void BlueTeamScores();

	UFUNCTION()
	void OnRep_RedTeamScore();
	
	UFUNCTION()
	void OnRep_BlueTeamScore();

private:
	float TopScore = 0.f;
	
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;
	
	UPROPERTY(Replicated)
	TArray<ETeam> TopScoringTeams;

	TArray<ABlasterPlayerState*> RedTeam;
	TArray<ABlasterPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	

public:
	FORCEINLINE TArray<ABlasterPlayerState*> GetTopScoringPlayers() const { return TopScoringPlayers; }
	FORCEINLINE TArray<ABlasterPlayerState*> GetRedTeam() const { return RedTeam; }
	FORCEINLINE TArray<ABlasterPlayerState*> GetBlueTeam() const { return BlueTeam; }
	FORCEINLINE void AddToRedTeam(ABlasterPlayerState* NewPlayer) { RedTeam.Add(NewPlayer); }
	FORCEINLINE void AddToBlueTeam(ABlasterPlayerState* NewPlayer) { BlueTeam.Add(NewPlayer); }
	FORCEINLINE float GetRedTeamScore() const { return RedTeamScore; }
	FORCEINLINE float GetBlueTeamScore() const { return BlueTeamScore; }
};
