// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStates/BlasterPlayerState.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

bool ABlasterGameState::IsTopScoringPlayer(ABlasterPlayerState* PlayerState)
{
	return TopScoringPlayers.Contains(PlayerState);
}

void ABlasterGameState::RemoveTopScoringPlayer(ABlasterPlayerState* PlayerToRemove)
{
	if (TopScoringPlayers.Contains(PlayerToRemove)) TopScoringPlayers.Remove(PlayerToRemove);
}
